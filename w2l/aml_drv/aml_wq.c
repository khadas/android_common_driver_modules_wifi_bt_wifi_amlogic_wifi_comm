/**
****************************************************************************************
*
* @file aml_wq.c
*
* Copyright (C) Amlogic, Inc. All rights reserved (2022-2023).
*
* @brief workqueue API implementation.
*
****************************************************************************************
*/
#include "aml_wq.h"
#include "aml_recy.h"
#include "aml_msg_tx.h"
#include "aml_scc.h"


struct aml_wq *aml_wq_alloc(int len)
{
    struct aml_wq *aml_wq = NULL;
    int size = sizeof(struct aml_wq) + len;

    aml_wq = kzalloc(size, GFP_ATOMIC);
    if (aml_wq) {
        INIT_LIST_HEAD(&aml_wq->list);
        aml_wq->len = len;
    }
    return aml_wq;
}

static struct aml_wq *aml_wq_get(struct aml_hw *aml_hw)
{
    struct aml_wq *aml_wq = NULL;

    spin_lock_bh(&aml_hw->wq_lock);
    if (!list_empty(&aml_hw->work_list)) {
        aml_wq = list_first_entry(&aml_hw->work_list,
                struct aml_wq, list);
        list_del(&aml_wq->list);
    }
    spin_unlock_bh(&aml_hw->wq_lock);

    return aml_wq;
}

void aml_wq_add(struct aml_hw *aml_hw, struct aml_wq *aml_wq)
{
    spin_lock_bh(&aml_hw->wq_lock);
    list_add_tail(&aml_wq->list, &aml_hw->work_list);
    spin_unlock_bh(&aml_hw->wq_lock);

    if (!work_pending(&aml_hw->work))
        queue_work(aml_hw->wq, &aml_hw->work);
}

void aml_wq_del(struct aml_hw *aml_hw)
{
    struct aml_wq *aml_wq, *tmp;

    spin_lock_bh(&aml_hw->wq_lock);
    list_for_each_entry_safe(aml_wq, tmp, &aml_hw->work_list, list) {
        list_del(&aml_wq->list);
        kfree(aml_wq);
    }
    spin_unlock_bh(&aml_hw->wq_lock);

    flush_work(&aml_hw->work);
}

extern void aml_show_tx_msg(struct aml_hw *aml_hw,struct aml_wq *aml_wq);
#ifndef CONFIG_AML_DEBUGFS
extern void aml_alloc_global_rx_rate(struct aml_hw *aml_hw, struct aml_wq *aml_wq);
#endif
static void aml_wq_doit(struct work_struct *work)
{
    struct aml_wq *aml_wq = NULL;
    struct aml_hw *aml_hw = container_of(work, struct aml_hw, work);

    while (1) {
        aml_wq = aml_wq_get(aml_hw);
        if (!aml_wq)
            return;

        switch (aml_wq->id) {
#ifdef CONFIG_AML_RECOVERY
            case AML_WQ_RECY:
                aml_recy_doit(aml_hw);
                break;
            case AML_WQ_CHECK_SCC:
                aml_scc_check_chan_conflict(aml_hw);
                break;
            case AML_WQ_RECY_CONNECT_RETRY:
                aml_recy_sta_connect(aml_hw, NULL);
                break;
#endif
            case AML_WQ_SYNC_TRACE:
                aml_send_sync_trace(aml_hw);
                break;
            case AML_WQ_SYNC_BEACON:
                aml_scc_sync_bcn(aml_hw, aml_wq);
                break;
            case AML_WQ_SHOW_TX_MSG:
                aml_show_tx_msg(aml_hw, aml_wq);
                break;
#ifndef CONFIG_AML_DEBUGFS
            case AML_WQ_ALLOC_RX_RATE:
                aml_alloc_global_rx_rate(aml_hw, aml_wq);
                break;
#endif
            default:
                AML_INFO("wq type(%d) unknown", aml_wq->id);
                break;
        }
        kfree(aml_wq);
    }
}

int aml_wq_init(struct aml_hw *aml_hw)
{
    AML_INFO("workqueue init");
    spin_lock_init(&aml_hw->wq_lock);
    INIT_LIST_HEAD(&aml_hw->work_list);
    INIT_WORK(&aml_hw->work, aml_wq_doit);

    aml_hw->wq = alloc_ordered_workqueue("w2_wq",
            WQ_HIGHPRI | WQ_CPU_INTENSIVE |
            WQ_MEM_RECLAIM);
    if (!aml_hw->wq) {
        AML_INFO("wq create failed");
        return -ENOMEM;
    }
    return 0;
}

void aml_wq_deinit(struct aml_hw *aml_hw)
{
    struct aml_wq *aml_wq, *tmp;

    AML_INFO("workqueue deinit");
    cancel_work_sync(&aml_hw->work);

    spin_lock_bh(&aml_hw->wq_lock);
    list_for_each_entry_safe(aml_wq, tmp, &aml_hw->work_list, list) {
        list_del(&aml_wq->list);
        kfree(aml_wq);
    }
    spin_unlock_bh(&aml_hw->wq_lock);

    flush_workqueue(aml_hw->wq);
    destroy_workqueue(aml_hw->wq);
}
