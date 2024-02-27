/**
 ******************************************************************************
 *
 * aml_cmds.c
 *
 * Handles queueing (push to IPC, ack/cfm from IPC) of commands issued to
 * LMAC FW
 *
 * Copyright (C) Amlogic 2014-2021
 *
 ******************************************************************************
 */

#include <linux/list.h>
#include "aml_cmds.h"
#include "aml_defs.h"
#include "aml_strs.h"
#define CREATE_TRACE_POINTS
#include "aml_events.h"
#include "aml_interface.h"


extern unsigned int aml_bus_type;
extern char *bus_type;

/**
 *
 */
static void cmd_dump(const struct aml_cmd *cmd)
{
#ifndef CONFIG_AML_FHOST
    pr_err("tkn[%d]  flags:%04x  result:%3d  cmd:%4d-%-24s - reqcfm(%4d-%-s)\n",
           cmd->tkn, cmd->flags, cmd->result, cmd->id, AML_ID2STR(cmd->id),
           cmd->reqid, ((cmd->flags & AML_CMD_FLAG_REQ_CFM) &&
               (cmd->reqid != (lmac_msg_id_t)-1)) ? AML_ID2STR(cmd->reqid) : "none");
#endif
}

#define CMD_PRINT(cmd) do { \
    if (cmd->id != ME_TRAFFIC_IND_REQ) { \
        AML_PRINT(AML_DBG_MODULES_CMD, "[%-20.20s %4d] cmd tkn[%d]  flags:%04x  result:%3d  cmd:%4d-%-24s - reqcfm(%4d-%-s)\n", \
               __func__, __LINE__,  \
               cmd->tkn, cmd->flags, cmd->result, cmd->id, cmd->id == MM_OTHER_REQ ? AML_MM_OTHER_CMD2STR(cmd) : AML_ID2STR(cmd->id), \
               cmd->reqid, (((cmd->flags & AML_CMD_FLAG_REQ_CFM) && \
               (cmd->reqid != (lmac_msg_id_t)-1)) ? AML_ID2STR(cmd->reqid) : "none")); \
    } \
} while (0);

/**
 *
 */
static void cmd_complete(struct aml_cmd_mgr *cmd_mgr, struct aml_cmd *cmd)
{
    lockdep_assert_held(&cmd_mgr->lock);

    list_del(&cmd->list);
    cmd_mgr->queue_sz--;

    CMD_PRINT(cmd);

    /*recovery happen when aml open, but AML_DEV_STARTED is not set*/
    if ((cmd->id == MM_START_REQ) && (cmd->reqid == MM_START_CFM)) {
        struct aml_hw *aml_hw = container_of(cmd_mgr, struct aml_hw, cmd_mgr);
        set_bit(AML_DEV_STARTED, &aml_hw->flags);
    }

    cmd->flags |= AML_CMD_FLAG_DONE;
    if (cmd->flags & AML_CMD_FLAG_NONBLOCK) {
        kfree(cmd);
    } else {
        if (AML_CMD_WAIT_COMPLETE(cmd->flags)) {
            cmd->result = 0;
            complete(&cmd->complete);
        }
    }
}

int aml_msg_task(void *data)
{
    struct aml_hw *aml_hw = (struct aml_hw *)data;
    struct aml_cmd_mgr *cmd_mgr = &aml_hw->cmd_mgr;
    struct aml_cmd *cmd = NULL;
    struct sched_param sch_param;

    sch_param.sched_priority = 90;
#ifndef CONFIG_PT_MODE
    sched_setscheduler(current, SCHED_RR, &sch_param);
#endif
    while (!aml_hw->aml_msg_task_quit) {
        if (down_interruptible(&aml_hw->aml_msg_sem) != 0) {
            AML_PRINT(AML_DBG_MODULES_CMD, "%s:%d wait aml_msg_sem fail!\n", __func__, __LINE__);
            break;
        }

        AML_DBG(AML_FN_ENTRY_STR);
        if (aml_hw->aml_msg_task_quit) {
            break;
        }

        spin_lock_bh(&cmd_mgr->lock);
        cmd = NULL;
        list_for_each_entry(cmd, &cmd_mgr->cmds, list) {
            if (cmd->flags & AML_CMD_FLAG_WAIT_PUSH) {
                break;
            }
        }
        spin_unlock_bh(&cmd_mgr->lock);

        if (cmd != NULL) {
            CMD_PRINT(cmd);
            cmd->flags &= ~AML_CMD_FLAG_WAIT_PUSH;

            trace_msg_send(cmd->id);
            aml_ipc_msg_push(aml_hw, cmd, AML_CMD_A2EMSG_LEN(cmd->a2e_msg));
            kfree(cmd->a2e_msg);
        }
    }
    if (aml_hw->aml_msg_completion_init) {
        aml_hw->aml_msg_completion_init = 0;
    complete_and_exit(&aml_hw->aml_msg_completion, 0);
    }

    return 0;
}

/**
 *
 */
extern struct aml_bus_state_detect bus_state_detect;
unsigned char g_fw_recovery_flag = 0;
static int cmd_mgr_queue(struct aml_cmd_mgr *cmd_mgr, struct aml_cmd *cmd)
{
    struct aml_hw *aml_hw = container_of(cmd_mgr, struct aml_hw, cmd_mgr);
    bool defer_push = false;
    u16 cmd_flags;
#ifndef CONFIG_AML_FHOST
    unsigned long tout = msecs_to_jiffies(AML_80211_CMD_TIMEOUT_MS * (cmd_mgr->queue_sz + 1));
#endif
    long ret;

    trace_msg_send(cmd->id);

    spin_lock_bh(&cmd_mgr->lock);

    if (cmd_mgr->state == AML_CMD_MGR_STATE_CRASHED) {
        u32 i;
        AML_PRINT(AML_DBG_MODULES_CMD, KERN_CRIT"cmd queue crashed\n");
        cmd->result = -EPIPE;
        spin_unlock_bh(&cmd_mgr->lock);
        if (aml_bus_type == PCIE_MODE) {
            for (i = 0; i < NX_VIRT_DEV_MAX; i++) {
                if (aml_hw->vif_table[i] != NULL) {
                    struct aml_vif *vif = aml_hw->vif_table[i];
                    for (i = 0; i < CMD_CRASH_FW_PC_NUM; i++) {
                        AML_INFO("fw_pc:%08x", aml_read_reg(vif->ndev, AML_FW_PC_POINTER));
                        mdelay(100);
                    }
                    break;
                }
            }
        }
        return -EPIPE;
    }

    #ifndef CONFIG_AML_FHOST
    if (!list_empty(&cmd_mgr->cmds)) {
        struct aml_cmd *last;

        if (cmd_mgr->queue_sz == cmd_mgr->max_queue_sz) {
            AML_PRINT(AML_DBG_MODULES_CMD, KERN_CRIT"Too many cmds (%d) already queued\n",
                   cmd_mgr->max_queue_sz);
            cmd->result = -ENOMEM;
            spin_unlock_bh(&cmd_mgr->lock);
            return -ENOMEM;
        }
        last = list_entry(cmd_mgr->cmds.prev, struct aml_cmd, list);
        if ((last->flags & (AML_CMD_FLAG_WAIT_ACK | AML_CMD_FLAG_WAIT_PUSH))
            || ((aml_bus_type == PCIE_MODE) && (last->flags & AML_CMD_FLAG_WAIT_CFM))) {
#if 0 // queue even NONBLOCK command.
            if (cmd->flags & AML_CMD_FLAG_NONBLOCK) {
                AML_PRINT(AML_DBG_MODULES_CMD, KERN_CRIT"cmd queue busy\n");
                cmd->result = -EBUSY;
                spin_unlock_bh(&cmd_mgr->lock);
                return -EBUSY;
            }
#endif
            cmd->flags |= AML_CMD_FLAG_WAIT_PUSH;
            defer_push = true;
        }
    }
    #endif

    cmd->flags |= AML_CMD_FLAG_WAIT_ACK;
    if (cmd->flags & AML_CMD_FLAG_REQ_CFM)
        cmd->flags |= AML_CMD_FLAG_WAIT_CFM;

    cmd->tkn    = cmd_mgr->next_tkn++;
    cmd->result = -EINTR;

    if (!(cmd->flags & AML_CMD_FLAG_NONBLOCK))
        init_completion(&cmd->complete);

    list_add_tail(&cmd->list, &cmd_mgr->cmds);
    cmd_mgr->queue_sz++;
    /* Prevent critical resources kfree by msg ack hw irq,
       Using local variables */
    cmd_flags = cmd->flags;
    spin_unlock_bh(&cmd_mgr->lock);

    if (!defer_push) {
        if ((aml_bus_type != PCIE_MODE) && (cmd->flags & AML_CMD_FLAG_CALL_THREAD)) {
            cmd->flags |= AML_CMD_FLAG_WAIT_PUSH;
            up(&aml_hw->aml_msg_sem);

        } else {
            aml_ipc_msg_push(aml_hw, cmd, AML_CMD_A2EMSG_LEN(cmd->a2e_msg));
            kfree(cmd->a2e_msg);
        }
    }


    if (!(cmd_flags & AML_CMD_FLAG_NONBLOCK)) {
        #ifdef CONFIG_AML_FHOST
        if (wait_for_completion_killable(&cmd->complete)) {
            if (cmd->flags & AML_CMD_FLAG_WAIT_ACK)
                up(&aml_hw->term.fw_cmd);
            cmd->result = -EINTR;
            spin_lock_bh(&cmd_mgr->lock);
            cmd_complete(cmd_mgr, cmd);
            spin_unlock_bh(&cmd_mgr->lock);
            /* TODO: kill the cmd at fw level */
        } else {
            // possible when commands are aborted with cmd_mgr_drain
            if (cmd->flags & AML_CMD_FLAG_WAIT_ACK)
                up(&aml_hw->term.fw_cmd);
        }
        #else
        ret = wait_for_completion_killable_timeout(&cmd->complete, tout);
        if (ret == -ERESTARTSYS) {
           // the completion have break by signal kill, need wait cmd complete
            while (1) {
                if (cmd->flags & AML_CMD_FLAG_DONE || tout/5 == 0)
                    break;
                msleep(5);
                tout = tout -5;
            }
        }
        if (!ret || tout/5 == 0) {
            u32 i;
#ifdef CONFIG_PT_MODE
            static u8 g_fw_recovery_ongoing = 0;
#endif
            struct aml_vif *vif = NULL;
            for (i = 0; i < NX_VIRT_DEV_MAX; i++) {
                if (aml_hw->vif_table[i] != NULL) {
                    vif = aml_hw->vif_table[i];
                    break;
                }
            }
            AML_PRINT(AML_DBG_MODULES_CMD, KERN_CRIT"cmd timed-out\n");
            cmd_dump(cmd);
            spin_lock_bh(&cmd_mgr->lock);
            cmd_mgr->state = AML_CMD_MGR_STATE_CRASHED;
            if (!(cmd->flags & AML_CMD_FLAG_DONE)) {
                cmd->result = -ETIMEDOUT;
                cmd_complete(cmd_mgr, cmd);
            }
            spin_unlock_bh(&cmd_mgr->lock);
#ifdef CONFIG_PT_MODE
            if (aml_bus_type == SDIO_MODE) {
                if (bus_state_detect.is_drv_load_finished) {
                    if (!g_fw_recovery_ongoing) {
                        g_fw_recovery_ongoing = 1;
                        g_fw_recovery_flag = 1;
                        aml_recy_fw_reload_for_usb_sdio(aml_hw);
                        g_fw_recovery_ongoing = 0;
                    }
                }
            }
#endif
            for (i = 0; i < CMD_CRASH_FW_PC_NUM; i++) {
                AML_INFO("fw_pc:%08x\n", AML_REG_READ(aml_hw->plat, AML_ADDR_MAC_PHY, AML_FW_PC_POINTER));
                mdelay(100);
            }
        }
        #endif
    }

    return 0;
}

/**
 *
 */
static int cmd_mgr_llind(struct aml_cmd_mgr *cmd_mgr, struct aml_cmd *cmd)
{
    struct aml_cmd *cur, *acked = NULL, *next = NULL;
    struct aml_hw *aml_hw = container_of(cmd_mgr, struct aml_hw, cmd_mgr);
    bool defer_push = true;

    if (aml_bus_type != USB_MODE)
        CMD_PRINT(cmd);
    aml_spin_lock(&cmd_mgr->lock);
    list_for_each_entry(cur, &cmd_mgr->cmds, list) {
        if (!acked) {
            if (cur->tkn == cmd->tkn) {
                if (WARN_ON_ONCE(cur != cmd)) {
                    cmd_dump(cmd);
                }
                acked = cur;
                continue;
            }
        }
        if (cur->flags & AML_CMD_FLAG_WAIT_PUSH) {
            next = cur;
            break;
        }
    }
    if (!acked) {
        AML_PRINT(AML_DBG_MODULES_CMD, KERN_CRIT "Error: acked cmd not found\n");
    } else {
        cmd->flags &= ~AML_CMD_FLAG_WAIT_ACK;
        if (AML_CMD_WAIT_COMPLETE(cmd->flags)) {
            defer_push = false;
            cmd_complete(cmd_mgr, cmd);
        }
    }

    if (next) {
        if (aml_bus_type != PCIE_MODE) {
            up(&aml_hw->aml_msg_sem);
        } else {
            if (!defer_push) {
                next->flags &= ~AML_CMD_FLAG_WAIT_PUSH;
                aml_ipc_msg_push(aml_hw, next, AML_CMD_A2EMSG_LEN(next->a2e_msg));
                kfree(next->a2e_msg);
            }
        }
    }
    aml_spin_unlock(&cmd_mgr->lock);

    return 0;
}



static int cmd_mgr_run_callback(struct aml_hw *aml_hw, struct aml_cmd *cmd,
                                struct aml_cmd_e2amsg *msg, msg_cb_fct cb)
{
    int res;

    if (! cb)
        return 0;
    aml_spin_lock(&aml_hw->cb_lock);
    res = cb(aml_hw, cmd, msg);
    aml_spin_unlock(&aml_hw->cb_lock);

    return res;
}

/**
 *

 */
static int cmd_mgr_msgind(struct aml_cmd_mgr *cmd_mgr, struct aml_cmd_e2amsg *msg,
                          msg_cb_fct cb)
{
    struct aml_hw *aml_hw = container_of(cmd_mgr, struct aml_hw, cmd_mgr);
    struct aml_cmd *cmd, *next = NULL;
    bool found = false;

    //AML_DBG(AML_FN_ENTRY_STR);
    trace_msg_recv(msg->id);
    aml_spin_lock(&cmd_mgr->lock);
    list_for_each_entry(cmd, &cmd_mgr->cmds, list) {
        if (cmd->reqid == msg->id &&
            (cmd->flags & AML_CMD_FLAG_WAIT_CFM)) {
            if (aml_bus_type != USB_MODE)
                CMD_PRINT(cmd);
            if (!cmd_mgr_run_callback(aml_hw, cmd, msg, cb)) {
                found = true;
                cmd->flags &= ~AML_CMD_FLAG_WAIT_CFM;

                if (WARN((msg->param_len > AML_CMD_E2AMSG_LEN_MAX),
                         "Unexpect E2A msg len %d > %d\n", msg->param_len,
                         AML_CMD_E2AMSG_LEN_MAX)) {
                    msg->param_len = AML_CMD_E2AMSG_LEN_MAX;
                }

                if (cmd->e2a_msg && msg->param_len)
                    memcpy(cmd->e2a_msg, &msg->param, msg->param_len);

                if (AML_CMD_WAIT_COMPLETE(cmd->flags)) {
                    if (cmd->list.next != &cmd_mgr->cmds) {
                        next = (struct aml_cmd *)cmd->list.next;
                        AML_INFO("next 0x%x \n", next);
                    }
                    cmd_complete(cmd_mgr, cmd);
                }

                break;
            }
        }
    }
    if (aml_bus_type == PCIE_MODE) {
        if (found && (next != NULL) && (next->flags & AML_CMD_FLAG_WAIT_PUSH)) {
            CMD_PRINT(next);
            AML_INFO("next len 0x%x, queue sz %d \n", AML_CMD_A2EMSG_LEN(next->a2e_msg), cmd_mgr->queue_sz);
            next->flags &= ~AML_CMD_FLAG_WAIT_PUSH;
            aml_ipc_msg_push(aml_hw, next, AML_CMD_A2EMSG_LEN(next->a2e_msg));
            kfree(next->a2e_msg);
        }
    }
    aml_spin_unlock(&cmd_mgr->lock);

    if (!found)
        cmd_mgr_run_callback(aml_hw, NULL, msg, cb);

    if (msg->id == MSG_I(MM_CSA_FINISH_IND))
        aml_sta_notify_csa_ch_switch(aml_hw, msg);

    return 0;
}

/**
 *
 */
static void cmd_mgr_print(struct aml_cmd_mgr *cmd_mgr)
{
    struct aml_cmd *cur;

    spin_lock_bh(&cmd_mgr->lock);
    AML_DBG("q_sz/max: %2d / %2d - next tkn: %d\n",
             cmd_mgr->queue_sz, cmd_mgr->max_queue_sz,
             cmd_mgr->next_tkn);
    list_for_each_entry(cur, &cmd_mgr->cmds, list) {
        cmd_dump(cur);
    }
    spin_unlock_bh(&cmd_mgr->lock);
}

/**
 *
 */
static void cmd_mgr_drain(struct aml_cmd_mgr *cmd_mgr)
{
    struct aml_cmd *cur, *nxt;

    AML_DBG(AML_FN_ENTRY_STR);

    spin_lock_bh(&cmd_mgr->lock);
    list_for_each_entry_safe(cur, nxt, &cmd_mgr->cmds, list) {
        list_del(&cur->list);

        cmd_mgr->queue_sz--;
        if (!(cur->flags & AML_CMD_FLAG_NONBLOCK))
            complete(&cur->complete);

        if (cur->flags & AML_CMD_FLAG_WAIT_PUSH) {
            kfree(cur->a2e_msg);
            kfree(cur);
        }
    }
    spin_unlock_bh(&cmd_mgr->lock);
}

/**
 *
 */
void aml_cmd_mgr_init(struct aml_cmd_mgr *cmd_mgr)
{
    AML_DBG(AML_FN_ENTRY_STR);

    INIT_LIST_HEAD(&cmd_mgr->cmds);

    spin_lock_init(&cmd_mgr->lock);
    cmd_mgr->state = AML_CMD_MGR_STATE_INITED;
    cmd_mgr->max_queue_sz = AML_CMD_MAX_QUEUED;
    cmd_mgr->queue  = &cmd_mgr_queue;
    cmd_mgr->print  = &cmd_mgr_print;
    cmd_mgr->drain  = &cmd_mgr_drain;
    cmd_mgr->llind  = &cmd_mgr_llind;
    cmd_mgr->msgind = &cmd_mgr_msgind;
}

/**
 *
 */
void aml_cmd_mgr_deinit(struct aml_cmd_mgr *cmd_mgr)
{
    cmd_mgr->state = AML_CMD_MGR_STATE_DEINIT;
    cmd_mgr->print(cmd_mgr);
    cmd_mgr->drain(cmd_mgr);
    cmd_mgr->print(cmd_mgr);
    memset(cmd_mgr, 0, sizeof(*cmd_mgr));
}

