#include <linux/sort.h>
#include <linux/math64.h>
#include "aml_iwpriv_cmds.h"
#include "aml_mod_params.h"
#include "aml_debugfs.h"
#include "aml_main.h"
#include "aml_msg_tx.h"
#include "aml_platform.h"
#include "reg_access.h"
#include "wifi_debug.h"
#include "aml_fw_trace.h"
#include "aml_utils.h"
#include "aml_compat.h"
#include "aml_recy.h"
#ifdef SDIO_SPEED_DEBUG
#include "sg_common.h"
#endif
#include "sdio_common.h"
#include "aml_fw_trace.h"

extern unsigned long long g_dbg_modules;

#define RC_AUTO_RATE_INDEX -1
#define MAX_CHAR_SIZE 60
#define PRINT_BUF_SIZE 512
#define LA_BUF_SIZE 2048
#define LA_MEMORY_BASE_ADDRESS 0x60830000
static char *mactrace_path = "/vendor/lib/firmware/la_dump/mactrace";
static char *reg_path = "/data/dumpinfo";
#define REG_DUMP_SIZE 2048

bool pt_mode = 0;
static unsigned char offset_times = 0;

//hx add
typedef unsigned char   U8;
#define MACBYP_TXV_ADDR 0x60C06200
#define MACBYP_TXV_04 0x04
#define MACBYP_TXV_08 0x08
#define MACBYP_TXV_0c 0x0c
#define MACBYP_TXV_10 0x10
#define MACBYP_TXV_14 0x14
#define MACBYP_TXV_18 0x18
#define MACBYP_TXV_1c 0x1c
#define MACBYP_TXV_20 0x20
#define MACBYP_TXV_24 0x24
#define MACBYP_TXV_28 0x28
#define MACBYP_TXV_2c 0x2c
#define MACBYP_TXV_30 0x30
#define MACBYP_TXV_34 0x34
#define MACBYP_TXV_38 0x38
#define MACBYP_TXV_3c 0x3c
#define MACBYP_TXV_40 0x40
#define MACBYP_TXV_44 0x44
#define MACBYP_CTRL_ADDR 0x60C06000
#define MACBYP_CTRL_80 0x80
#define MACBYP_CTRL_84 0x84
#define MACBYP_CTRL_88 0x88
#define MACBYP_CTRL_8C 0x8C
#define RF_CTRL_ADDR 0X80000000
#define RF_CTRL_08 0x08
#define RF_CTRL_1008 0x1008
#define RF_ANTTA_ACTIVE 0X60C0B500
#define MACBYP_RIU_EN 0X60C0B004
#define MACBYP_AP_BW 0X60C00800
#define MACBYP_DIG_GAIN 0X60C0B100
#define MACBYP_PKT_ADDR 0X60C06010
#define MACBYP_PAYLOAD_ADDR 0x60C06004
#define MACBYP_TRIGGER_ADDR 0x60C06008
#define MACBYP_CLKEN_ADDR 0x60C0600C
#define MACBYP_INTERFRAME_DELAY_ADDR 0x60C06048
#define CPU_CLK_REG_ADDR 0x00a0d090
#define MPF_CLK_REG_ADDR 0x00a0d084
#define CRM_CLKRST_CNTL_ADDR 0x60805008
#define CRM_CLKGATEPHYFCTRL0_ADDR 0x60805010
#define CRM_CLKGATEPHYFCTRL1_ADDR 0x60805014
#define PCIE_BAR4_TABLE5_EP_BASE_MIMO 0x60c0088C
#define AGC_ADDR_HT 0x60c0b104
#define XOSC_CTUNE_BASE 0x00f01024
#define POWER_OFFSET_BASE_WF0 0x00a0e658
#define POWER_OFFSET_BASE_WF1 0x00a0f658
#define EFUSE_BASE_1A 0X1A
#define EFUSE_BASE_1B 0X1B
#define EFUSE_BASE_1C 0X1C
#define EFUSE_BASE_1E 0X1E
#define EFUSE_BASE_1F 0X1F
#define EFUSE_BASE_04 0X04
#define EFUSE_BASE_05 0X05
#define EFUSE_BASE_01 0x01
#define EFUSE_BASE_02 0x02
#define EFUSE_BASE_03 0x03
#define EFUSE_BASE_09 0x09
#define EFUSE_BASE_0F 0x0F
#define EFUSE_BASE_11 0x11
#define EFUSE_BASE_12 0x12
#define EFUSE_BASE_13 0x13
#define EFUSE_BASE_18 0x18
#define EFUSE_BASE_14 0x14
#define EFUSE_BASE_15 0x15
#define EFUSE_BASE_16 0x16
#define EFUSE_BASE_17 0x17
#define EFUSE_BASE_07 0x07
#define EFUSE_BASE_10 0x10
#define EFUSE_BASE_0B 0x0B
#define EFUSE_BASE_0F 0X0F
#define EFUSE_BASE_00 0x00
#define EFUSE_BASE_06 0x06

#define BIT4 0x00000010
#define BIT31 0x80000000
#define BIT16 0x00010000
#define BIT17 0x00020000
#define BIT20 0x0100000

//Returns a char * arr [] and size is the length of the returned array
char **aml_cmd_char_phrase(char sep, const char *str, int *size)
{
    int count = 0;
    int i;
    char **ret;
    int lastindex = -1;
    int j = 0;

    for (i = 0; i < strlen(str); i++) {
        if (str[i] == sep) {
            count++;
        }
    }

    ret = (char **)kzalloc((++count) * sizeof(char *), GFP_KERNEL);

    for (i = 0; i < strlen(str); i++) {
        if (str[i] == sep) {
            // kzalloc the memory space of substring length + 1
            ret[j] = (char *)kzalloc((i - lastindex) * sizeof(char), GFP_KERNEL);
            memcpy(ret[j], str + lastindex + 1, i - lastindex - 1);
            j++;
            lastindex = i;
        }
    }
    //Processing the last substring
    if (lastindex <= strlen(str) - 1) {
        ret[j] = (char *)kzalloc((strlen(str) - lastindex) * sizeof(char), GFP_KERNEL);
        memcpy(ret[j], str + lastindex + 1, strlen(str) - 1 - lastindex);
        j++;
    }

    *size = j;

    return ret;
}

/** To calculate the index:
# NON-HT CCK : idx = (RATE_INDEX*2) + pre_type
# NON-HT OFDM: idx = 8 + (RATE_INDEX-4)
# HT: 16 + NSS*32 + MCS*4 + BW*2 + GI
# VHT: 144 + NSS*80 + MCS*8 + BW*2 + GI
# HE: 784 + NSS*144 + MCS*12 + BW*3 + GI
#
# Where:
#
# RATE_INDEX=[0-11] (1,2,5.5,11,6,9,12,18,24,36,48,54)
# pre_type=1: only long preamble, pre_type=0: short and long preamble
# NSS=0: 1 spatial stream, NSS=1: 2 spatial streams, ...
# MCS=0: MCS0, MCS=1: MCS1, ... MCS11
# BW=0: 20 MHz, BW=1: 40 MHz, BW=2: 80 MHz, BW=3: 160 MHz
# GI=0: long guard interval, GI=1: short guard interval (for HT/VHT)
# GI=0: 0.8us guard interval, GI=1: 1.6us guard interval, GI=2: 3.2us guard interval (for HE)
**/
static int aml_get_mcs_rate_index(enum aml_iwpriv_subcmd type,  unsigned int nss,
    unsigned int mcs, unsigned int bw, unsigned int gi)
{
    int rate_index = RC_AUTO_RATE_INDEX;

    switch (type) {
        case AML_IWP_SET_RATE_HT:
            rate_index = 16 + (nss * 32) + (mcs * 4) + (bw * 2) + gi;
            break;

        case AML_IWP_SET_RATE_VHT:
            rate_index = 144 + (nss * 80) + (mcs * 8) + (bw * 2) + gi;
            break;

        case AML_IWP_SET_RATE_HE:
            rate_index = 784 + (nss * 144) + (mcs * 12) + (bw * 3) + gi;
            break;

        case AML_IWP_SET_RATE_AUTO:
        default:
            rate_index = RC_AUTO_RATE_INDEX;
            break;
    }

    return rate_index;
}

static int aml_legacy_rate_to_index(int legacy)
{
    int i = 0;
    int legacy_rate_map[12]={1, 2, 5, 11, 6, 9, 12, 18, 24, 36, 48, 54};

    for (i = 0; i < 12; i++) {
        if (legacy_rate_map[i] == legacy) {
            return i;
        }
    }

    return -1;
}

static int aml_set_fixed_rate(struct net_device *dev,  int fixed_rate_idx)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    struct aml_sta *sta = NULL;
    union aml_rate_ctrl_info rate_config;
    int i = 0, error = 0;

     /* Convert rate index into rate configuration */
    if ((fixed_rate_idx < 0) || (fixed_rate_idx >= (N_CCK + N_OFDM + N_HT + N_VHT + N_HE_SU + N_HE_MU + N_HE_ER))) {
        // disable fixed rate
        rate_config.value = (u32)-1;
    } else {
        idx_to_rate_cfg(fixed_rate_idx, (union aml_rate_ctrl_info *)&rate_config, NULL);
    }

    // Forward the request to the LMAC
    for (i = 0; i < NX_REMOTE_STA_MAX; i++) {
        sta = aml_hw->sta_table + i;

        if (sta && sta->valid && (aml_vif->vif_index == sta->vif_idx)) {
             if ((error = aml_send_me_rc_set_rate(aml_hw, sta->sta_idx,
                (u16)rate_config.value)) != 0) {
                return error;
            }

            aml_hw->debugfs.rc_config[sta->sta_idx] = (int)rate_config.value;
        }
    }

    return error;
}

static int aml_set_p2p_noa(struct net_device *dev, int count, int interval, int duration)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    struct mm_set_p2p_noa_cfm cfm;
    if (AML_VIF_TYPE(aml_vif) == NL80211_IFTYPE_P2P_GO) {
        /* Forward request to the embedded and wait for confirmation */
        aml_send_p2p_noa_req(aml_hw, aml_vif, count, interval, duration, 0,  &cfm);
    }
    return 0;
}

static int aml_set_mcs_fixed_rate(struct net_device *dev, enum aml_iwpriv_subcmd type,
    unsigned int nss_mcs, unsigned int bw, unsigned int gi)
{
    int fix_rate_idx = 0;
    unsigned int nss = 0;
    unsigned int mcs = 0;

    nss = (nss_mcs >> 16) & 0xff;
    mcs = nss_mcs & 0xff;

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set fix_rate[nss:%d mcs:%d bw:%d gi:%d]\n", nss, mcs, bw, gi);

    fix_rate_idx = aml_get_mcs_rate_index(type, nss, mcs, bw, gi);

    return aml_set_fixed_rate(dev,fix_rate_idx);
}

static int aml_set_legacy_rate(struct net_device *dev, int legacy, int pre_type)
{
    int fix_rate_idx = 0;
    int legacy_rate_idx = 0;

    legacy_rate_idx = aml_legacy_rate_to_index(legacy);

    if (legacy_rate_idx < 0)
    {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "Operation failed! Please enter the correct format\n");
        return 0;
    }

    if (legacy_rate_idx < N_CCK/2)
        fix_rate_idx = (legacy_rate_idx * 2) + pre_type;
    else
        fix_rate_idx = 8 + (legacy_rate_idx - 4);

    return aml_set_fixed_rate(dev,fix_rate_idx);
}

static int aml_iwpriv_set_scan_hang(struct net_device *dev, int scan_hang)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
/*
    if (aml_vif->sta.scan_hang == scan_hang) {
       return 0;
    }

*/
    aml_vif->sta.scan_hang = scan_hang;

    aml_scan_hang(aml_vif, scan_hang);

    return 0;
}

static int aml_set_limit_power_status(struct net_device *dev, int limit_power_switch)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    if (limit_power_switch > 0x2)
    {
        AML_INFO("param error \n")
    }

    return aml_set_limit_power(aml_hw, limit_power_switch);
}

static int aml_set_scan_time(struct net_device *dev, int scan_duration)
{
    struct aml_vif *aml_vif = netdev_priv(dev);

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set scan duration to %d us \n", scan_duration);
    aml_vif->sta.scan_duration = scan_duration;

    return 0;
}

int aml_get_reg_2(struct net_device *dev, unsigned int addr,union iwreq_data *wrqu, char *extra)
{
    unsigned int reg_val = 0;
    if (aml_bus_type == PCIE_MODE) {
         u8 *map_address = NULL;
         if (addr & 3) {
             reg_val = 0xdead5555;
         }
         else {
             map_address = aml_pci_get_map_address(dev, addr);
             if (map_address) {
                 reg_val = aml_pci_readl(map_address);
             }
         }
     } else {
         struct aml_vif *aml_vif = netdev_priv(dev);
         struct aml_hw *aml_hw = aml_vif->aml_hw;
         struct aml_plat *aml_plat = aml_hw->plat;
         reg_val = AML_REG_READ(aml_plat, 0, addr);
     }
    wrqu->data.length = scnprintf(extra, IW_PRIV_SIZE_MASK, "&0x%08x", reg_val);
    wrqu->data.length++;

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "reg_val: 0x%08x", reg_val);
    //AML_PRINT(AML_DBG_MODULES_IWPRIV, "Get reg addr: 0x%08x value: 0x%08x\n", addr, reg_val);
    return reg_val;
}

int aml_get_reg(struct net_device *dev, char *str_addr, union iwreq_data *wrqu, char *extra)
{
    unsigned int addr = 0;
    unsigned int reg_val = 0;

    addr = simple_strtol(str_addr, NULL, 0);

    if (aml_bus_type == PCIE_MODE) {
        u8 *map_address = NULL;
        if (addr & 3) {
            reg_val = 0xdead5555;
        }
        else {
            map_address = aml_pci_get_map_address(dev, addr);
            if (map_address) {
                reg_val = aml_pci_readl(map_address);
            }
        }
    } else {
        struct aml_vif *aml_vif = netdev_priv(dev);
        struct aml_hw *aml_hw = aml_vif->aml_hw;
        struct aml_plat *aml_plat = aml_hw->plat;
        reg_val = AML_REG_READ(aml_plat, 0, addr);
    }

    wrqu->data.length = scnprintf(extra, IW_PRIV_SIZE_MASK, "&0x%08x", reg_val);
    wrqu->data.length++;

//    AML_PRINT(AML_DBG_MODULES_IWPRIV, "Get reg addr: 0x%08x value: 0x%08x\n", addr, reg_val);
    return 0;
}

int aml_set_reg(struct net_device *dev, int addr, int val)
{

    if (aml_bus_type != PCIE_MODE) {
        struct aml_vif *aml_vif = netdev_priv(dev);
        struct aml_hw *aml_hw = aml_vif->aml_hw;
        struct aml_plat *aml_plat = aml_hw->plat;
        AML_REG_WRITE(val, aml_plat, 0, addr);
    } else {
        u8* map_address = NULL;
        if (addr & 3) {
            AML_PRINT(AML_DBG_MODULES_IWPRIV, "Set Fail addr error: 0x%08x\n", addr);
            return -1;
        }
        map_address = aml_pci_get_map_address(dev, addr);
        if (map_address) {
            aml_pci_writel(val, map_address);
        }
    }

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "Set reg addr: 0x%08x value:0x%08x\n", addr, val);
    return 0;
}

int aml_sdio_start_test(struct net_device *dev)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    unsigned int temperature = 0;
    unsigned int i = 0;

    unsigned char set_buf[100] = {0};
    unsigned char get_buf[100] = {0};
    unsigned char rst_buf[100] = {0};
    memset(set_buf, 0x66, 100);

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "sdio stress testing start\n");

    while (1) {
        if (aml_bus_type == USB_MODE) {
            aml_hw->plat->hif_ops->hi_write_sram((unsigned char *)set_buf, (unsigned char *)0x6000f4f4, 100, USB_EP4);
            aml_hw->plat->hif_ops->hi_read_sram((unsigned char *)get_buf, (unsigned char *)0x6000f4f4, 100, USB_EP4); //RXU_STAT_DESC_POOL
        } else if (aml_bus_type == SDIO_MODE) {
            aml_hw->plat->hif_sdio_ops->hi_random_ram_write((unsigned char *)set_buf, (unsigned char *)0x6000f4f4, 100); //RXU_STAT_DESC_POOL
            aml_hw->plat->hif_sdio_ops->hi_random_ram_read((unsigned char *)get_buf, (unsigned char *)0x6000f4f4, 100); //RXU_STAT_DESC_POOL
        }

        if (memcmp(set_buf, get_buf, 100)) {
            if (aml_bus_type == USB_MODE) {
                temperature = aml_hw->plat->hif_ops->hi_read_word(0x00a04940, USB_EP4);
            } else if (aml_bus_type == SDIO_MODE) {
                temperature = aml_hw->plat->hif_sdio_ops->hi_random_word_read(0x00a04940);
            }
            AML_PRINT(AML_DBG_MODULES_IWPRIV, " test NG, temperature is 0x%08x\n", temperature & 0x0000ffff);
        } else {
            if (aml_bus_type == USB_MODE) {
                temperature = aml_hw->plat->hif_ops->hi_read_word(0x00a04940,USB_EP4);
            } else if (aml_bus_type == SDIO_MODE) {
                temperature = aml_hw->plat->hif_sdio_ops->hi_random_word_read(0x00a04940);
            }

            i++;
            if (i == 1000) {
                AML_PRINT(AML_DBG_MODULES_IWPRIV, " test OK, temperature is 0x%08x\n", temperature & 0x0000ffff);
                i = 0;
            }
        }
        memset(get_buf, 0x77, 100);
        if (aml_bus_type == USB_MODE) {
            aml_hw->plat->hif_ops->hi_write_sram((unsigned char *)rst_buf, (unsigned char *)0x6000f4f4, 100, USB_EP4); //RXU_STAT_DESC_POOL
        } else if (aml_bus_type == SDIO_MODE) {
            aml_hw->plat->hif_sdio_ops->hi_random_ram_write((unsigned char *)rst_buf, (unsigned char *)0x6000f4f4, 100); //RXU_STAT_DESC_POOL
        }
    }

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "sdio stress testing end\n");
    return 0;
}


int aml_enable_wf(struct net_device *dev, int wfflag)
{
    struct aml_vif *aml_vif = netdev_priv(dev);

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "aml_enable wf: 0x%08x\n", wfflag);

    _aml_enable_wf(aml_vif, wfflag);

    return 0;
}

int aml_get_efuse(struct net_device *dev, char *str_addr, union iwreq_data *wrqu, char *extra)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    unsigned int reg_val = 0;
    unsigned int addr = 0;

    addr = simple_strtol(str_addr, NULL, 0);

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "Get efuse addr: 0x%08x\n", addr);

    reg_val = _aml_get_efuse(aml_vif, addr);

    wrqu->data.length = scnprintf(extra, IW_PRIV_SIZE_MASK, "&0x%08x", reg_val);
    wrqu->data.length++;

    return 0;
}

int aml_set_efuse(struct net_device *dev, int addr, int val)
{
    struct aml_vif *aml_vif = netdev_priv(dev);

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "Set reg addr: 0x%08x value:0x%08x\n", addr, val);

    _aml_set_efuse(aml_vif, addr, val);

    return 0;
}

int reg_cca_cond_get(struct aml_hw *aml_hw)
{
    struct aml_plat *aml_plat = aml_hw->plat;

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "CCA Check [%d], CCA BUSY: Prim20: %08d Second20: %08d Second40: %08d\n",
        AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, AGCCCCACAL0_ADDR_CT) & 0xfffff,
        AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, AGCCCCACAL1_ADDR_CT) & 0xffff,
        AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, AGCCCCACAL1_ADDR_CT) >> 16 & 0xffff,
        AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, AGCCCCACAL2_ADDR_CT) & 0xffff);
    return 0;
}

int aml_get_rf_reg(struct net_device *dev, char *str_addr, union iwreq_data *wrqu, char *extra)
{
    unsigned int addr = 0;
    unsigned int reg_val = 0;

    addr = simple_strtol(str_addr, NULL, 0);

    reg_val = aml_rf_reg_read(dev, addr);

    wrqu->data.length = scnprintf(extra, IW_PRIV_SIZE_MASK, "&0x%08x", reg_val);
    wrqu->data.length++;

//    AML_PRINT(AML_DBG_MODULES_IWPRIV, "Get reg addr: 0x%08x value: 0x%08x\n", addr, reg_val);
    return 0;
}

int aml_get_csi_status_com(struct net_device *dev, union iwreq_data *wrqu)
{
    unsigned int ret_copy = 0;
    struct csi_com_status_get_ind ind;

    memset(&ind, 0, sizeof(struct csi_com_status_get_ind));
    aml_csi_status_com_read(dev, &ind);

    AML_PRINT(AML_DBG_MODULES_CSI, "time_stamp: %llu \n", ind.time_stamp);
    AML_PRINT(AML_DBG_MODULES_CSI, "mac_ra:     %02X%02X-%02X%02X-%02X%02X \n", ind.mac_ra[0], ind.mac_ra[1], ind.mac_ra[2], ind.mac_ra[3], ind.mac_ra[4], ind.mac_ra[5]);
    AML_PRINT(AML_DBG_MODULES_CSI, "mac_ta:     %02X%02X-%02X%02X-%02X%02X \n", ind.mac_ta[0], ind.mac_ta[1], ind.mac_ta[2], ind.mac_ta[3], ind.mac_ta[4], ind.mac_ta[5]);
    AML_PRINT(AML_DBG_MODULES_CSI, "freq_band:  %u \n", ind.frequency_band);
    AML_PRINT(AML_DBG_MODULES_CSI, "bw:         %u \n", ind.bw);
    AML_PRINT(AML_DBG_MODULES_CSI, "rssi:       %d %d \n", ind.rssi[0], ind.rssi[1]);
    AML_PRINT(AML_DBG_MODULES_CSI, "snr:        %u \n", ind.snr);
    AML_PRINT(AML_DBG_MODULES_CSI, "noise:      %u \n", ind.noise);
    AML_PRINT(AML_DBG_MODULES_CSI, "phase_incr: %d \n", ind.phase_incr);
    AML_PRINT(AML_DBG_MODULES_CSI, "pro_mode:   0x%x \n", ind.protocol_mode);
    AML_PRINT(AML_DBG_MODULES_CSI, "frame_type: 0x%x \n", ind.frame_type);
    AML_PRINT(AML_DBG_MODULES_CSI, "chain_num:  %u \n", ind.chain_num);
    AML_PRINT(AML_DBG_MODULES_CSI, "tones_num:  %u \n", ind.tones_num);
    AML_PRINT(AML_DBG_MODULES_CSI, "chan_index: %u \n", ind.primary_channel_index);
    AML_PRINT(AML_DBG_MODULES_CSI, "phyerr:------- \n");
    AML_PRINT(AML_DBG_MODULES_CSI, "rate:       %u \n", ind.rate);
    AML_PRINT(AML_DBG_MODULES_CSI, "agc_code:   %u %u \n", ind.agc_code[0], ind.agc_code[1]);
    AML_PRINT(AML_DBG_MODULES_CSI, "channel:    %u \n", ind.channel);
    AML_PRINT(AML_DBG_MODULES_CSI, "packet_idx:--- \n");
    AML_PRINT(AML_DBG_MODULES_CSI, "nrx:        %u \n", ind.nrx);
    AML_PRINT(AML_DBG_MODULES_CSI, "ntx:        %u \n", ind.ntx);
    AML_PRINT(AML_DBG_MODULES_CSI, "perm:--------- \n");
    AML_PRINT(AML_DBG_MODULES_CSI, "csi_len:    %u \n", ind.csi_len);
    AML_PRINT(AML_DBG_MODULES_CSI, "payload_len:---\n");
    AML_PRINT(AML_DBG_MODULES_CSI, "extra_info:--- \n");
    AML_PRINT(AML_DBG_MODULES_CSI, "sequence_no:%u \n", ind.sequence_no);
    AML_PRINT(AML_DBG_MODULES_CSI, "csi_ready  :%u \n", ind.csi_ready);

    wrqu->data.length = sizeof(ind);
    ret_copy = copy_to_user(wrqu->data.pointer, (void*)&ind, wrqu->data.length);
    if (ret_copy != 0)
        AML_INFO("copy csi com to user failed, failed num:%d%%%d", ret_copy, wrqu->data.length);

    return 0;
}

int aml_get_csi_status_sp(struct net_device *dev, union iwreq_data *wrqu, char *extra)
{
    unsigned int ret_copy = 0;
    struct csi_sp_status_get_ind ind;
    int *param = (int *)extra;
    int csi_mode = param[0];
    int i;

    memset(&ind, 0, sizeof(struct csi_sp_status_get_ind));
    aml_csi_status_sp_read(dev, csi_mode, &ind);

    AML_PRINT(AML_DBG_MODULES_CSI, "csi_mode:0x%02x csi data_len:0x%02x\n", csi_mode, ind.data_len);

    for (i = 0; i < ind.data_len; i = i + 1)
    {
        AML_PRINT(AML_DBG_MODULES_CSI, "index 0x%02x: 0x%08x\n", i, ind.csi[i]);
    }

    wrqu->data.length = sizeof(ind.data_len) + sizeof(struct csi_complex) * ind.data_len;
    ret_copy = copy_to_user(wrqu->data.pointer, (void*)&ind, wrqu->data.length);
    if (ret_copy != 0)
        AML_INFO("copy csi sp to user failed, failed num:%d%%%d", ret_copy, wrqu->data.length);

    return 0;
}

int aml_set_csi(struct net_device *dev, char* arg_iw)
{
    struct csi_set_req req;
    int cmd_arg;
    char **arg;
    char sep = '-';
    int arg_index = 0;
    char **mac_addr;
    int i;
    int count = 0;

    for (i = 0; i < strlen(arg_iw); i++) {
        if (arg_iw[i] == sep) {
            count++;
        }
    }

    /**cmd format："mask-nss-protocol mode-bw-frame type-addr1-addr2"
    #arg0-mask filter meaning：
    #arg1-BIT0：nss
    #arg2-BIT1：protocol mode
    #arg3-BIT2：bw
    #arg4-BIT3：frame type
    #arg5-BIT4：addr1
    #arg6-BIT5：addr2
    **/
    if (count != 6)
        return 0;

    arg = aml_cmd_char_phrase(sep, arg_iw, &cmd_arg);

    if (!arg)
    {
        kfree(arg);
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "cmd format failed!\n");
        return 0;
    }

    req.mask = (U8)simple_strtol(arg[0], NULL, 0);
    arg_index++;

    if ((req.mask & BIT(0)) == 0) {
       req.rxv2_nss = simple_strtol(arg[arg_index], NULL, 0);
    }
    arg_index++;

    if ((req.mask & BIT(1)) == 0) {
       req.protocol_mode = simple_strtol(arg[arg_index], NULL, 0);
    }
    arg_index++;

    if ((req.mask & BIT(2)) == 0) {
        req.bw = simple_strtol(arg[arg_index], NULL, 0);
    }
    arg_index++;

    if ((req.mask & BIT(3)) == 0) {
        req.frame_type = simple_strtol(arg[arg_index], NULL, 0);
    }
    arg_index++;

    if ((req.mask & BIT(4)) == 0) {
        if (strlen(arg[arg_index]) != strlen("00:00:00:00:00:00")) {
            AML_PRINT(AML_DBG_MODULES_IWPRIV, "mac size error!\n");
            kfree(arg);
            return 0;
        }

        sep = ':';
        mac_addr = aml_cmd_char_phrase(sep, arg[arg_index], &cmd_arg);
        if (mac_addr) {
            for (i = 0; i < 6; i++) {
                req.mac_ra[i] = simple_strtoul(mac_addr[i], NULL, 16);
            }
        }
        kfree(mac_addr);
    }
    arg_index++;

    if ((req.mask & BIT(5)) == 0) {
        if (strlen(arg[arg_index]) != strlen("00:00:00:00:00:00")) {
            AML_PRINT(AML_DBG_MODULES_IWPRIV, "mac size error!\n");
            kfree(arg);
            return 0;
        }

        sep = ':';
        mac_addr = aml_cmd_char_phrase(sep, arg[arg_index], &cmd_arg);
        if (mac_addr) {
            for (i = 0; i < 6; i++) {
                req.mac_ta[i] = simple_strtoul(mac_addr[i], NULL, 16);
            }
        }
        kfree(mac_addr);
    }

    AML_PRINT(AML_DBG_MODULES_IWPRIV,"csi set mask:0x%x nss:%x mode:%x bw:%x type:%x addr1:%02x%02x:%02x%02x:%02x%02x addr2:%02x%02x:%02x%02x:%02x%02x",
            req.mask, req.rxv2_nss, req.protocol_mode, req.bw, req.frame_type,
            req.mac_ra[0],req.mac_ra[1],req.mac_ra[2],req.mac_ra[3],req.mac_ra[4],req.mac_ra[5],
            req.mac_ta[0],req.mac_ta[1],req.mac_ta[2],req.mac_ta[3],req.mac_ta[4],req.mac_ta[5]);

    aml_csi_set(dev, &req);
    kfree(arg);
    return 0;
}


int aml_iwpriv_set_debug_switch(char *switch_str)
{
    int debug_switch = 0;
    if (strstr(switch_str,"_off") != NULL)
        debug_switch = AML_DBG_OFF;
    else if (strstr(switch_str,"_on") != NULL)
        debug_switch = AML_DBG_ON;
    else
        ERROR_DEBUG_OUT("input error\n");
    return debug_switch;
}



int aml_set_debug(struct net_device *dev, char *debug_str)
{
    if (debug_str == NULL || strlen(debug_str) <= 0) {
        ERROR_DEBUG_OUT("debug modules is NULL\n");
        return -1;
    }
    if (strstr(debug_str,"tx") != NULL) {
        if (aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
            g_dbg_modules &= ~(AML_DBG_MODULES_TX);
            return 0;
        }
        g_dbg_modules |= AML_DBG_MODULES_TX;
    } else if (strstr(debug_str,"rx") != NULL) {
       if (aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
            g_dbg_modules &= ~(AML_DBG_MODULES_RX);
            return 0;
        }
        g_dbg_modules |= AML_DBG_MODULES_RX;
    } else if (strstr(debug_str, "cmd") != NULL) {
        if (aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
             g_dbg_modules &= ~(AML_DBG_MODULES_CMD);
             return 0;
         }
         g_dbg_modules |= AML_DBG_MODULES_CMD;
    } else if (strstr(debug_str, "trace") != NULL) {
        if (aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
             g_dbg_modules &= ~(AML_DBG_MODULES_TRACE);
             return 0;
         }
         g_dbg_modules |= AML_DBG_MODULES_TRACE;
    } else if (strstr(debug_str, "interface") != NULL) {
        if (aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
             g_dbg_modules &= ~(AML_DBG_MODULES_INTERFACE);
             return 0;
         }
         g_dbg_modules |= AML_DBG_MODULES_INTERFACE;
    } else if (strstr(debug_str, "iwpriv") != NULL) {
        if (aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
             g_dbg_modules &= ~(AML_DBG_MODULES_IWPRIV);
             return 0;
         }
         g_dbg_modules |= AML_DBG_MODULES_IWPRIV;
    } else if (strstr(debug_str, "main") != NULL) {
        if (aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
             g_dbg_modules &= ~(AML_DBG_MODULES_MAIN);
             return 0;
         }
         g_dbg_modules |= AML_DBG_MODULES_MAIN;
    } else if (strstr(debug_str, "mdns") != NULL) {
        if (aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
             g_dbg_modules &= ~(AML_DBG_MODULES_MDNS);
             return 0;
         }
         g_dbg_modules |= AML_DBG_MODULES_MDNS;
    } else if (strstr(debug_str, "msgrx") != NULL) {
        if (aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
             g_dbg_modules &= ~(AML_DBG_MODULES_MSG_RX);
             return 0;
         }
         g_dbg_modules |= AML_DBG_MODULES_MSG_RX;
    } else if (strstr(debug_str, "msgtx") != NULL) {
        if (aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
             g_dbg_modules &= ~(AML_DBG_MODULES_MSG_TX);
             return 0;
         }
         g_dbg_modules |= AML_DBG_MODULES_MSG_TX;
    } else if (strstr(debug_str, "platf") != NULL) {
        if (aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
             g_dbg_modules &= ~(AML_DBG_MODULES_PLATF);
             return 0;
         }
         g_dbg_modules |= AML_DBG_MODULES_PLATF;
    } else if (strstr(debug_str, "testm") != NULL) {
        if (aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
             g_dbg_modules &= ~(AML_DBG_MODULES_TESTM);
             return 0;
         }
         g_dbg_modules |= AML_DBG_MODULES_TESTM;
    } else if (strstr(debug_str, "pci") != NULL) {
        if (aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
             g_dbg_modules &= ~(AML_DBG_MODULES_PCI);
             return 0;
         }
         g_dbg_modules |= AML_DBG_MODULES_PCI;
    } else if (strstr(debug_str, "common") != NULL) {
        if (aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
             g_dbg_modules &= ~(AML_DBG_MODULES_COMMON);
             return 0;
         }
         g_dbg_modules |= AML_DBG_MODULES_COMMON;
    } else if (strstr(debug_str, "sdio") != NULL) {
        if (aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
             g_dbg_modules &= ~(AML_DBG_MODULES_SDIO);
             return 0;
         }
         g_dbg_modules |= AML_DBG_MODULES_SDIO;
    } else if (strstr(debug_str, "usb") != NULL) {
        if (aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
             g_dbg_modules &= ~(AML_DBG_MODULES_USB);
             return 0;
         }
         g_dbg_modules |= AML_DBG_MODULES_USB;
    } else if (strstr(debug_str, "utils") != NULL) {
        if (aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
             g_dbg_modules &= ~(AML_DBG_MODULES_UTILS);
             return 0;
         }
         g_dbg_modules |= AML_DBG_MODULES_UTILS;
    } else if (strstr(debug_str, "csi") != NULL) {
        if (aml_iwpriv_set_debug_switch(debug_str) == AML_DBG_OFF) {
             g_dbg_modules &= ~(AML_DBG_MODULES_CSI);
             return 0;
         }
         g_dbg_modules |= AML_DBG_MODULES_CSI;
    } else {
        ERROR_DEBUG_OUT("input error\n");
    }
    return 0;
}

static int aml_set_p2p_oppps(struct net_device *dev, int ctw)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw * aml_hw = aml_vif->aml_hw;
    struct mm_set_p2p_oppps_cfm cfm;
    if (AML_VIF_TYPE(aml_vif) == NL80211_IFTYPE_P2P_GO) {
        /* Forward request to the embedded and wait for confirmation */
        aml_send_p2p_oppps_req(aml_hw, aml_vif, (u8)ctw, &cfm);
    }
    return 0;
}

static int aml_set_amsdu_max(struct net_device *dev, int amsdu_max)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw * aml_hw = aml_vif->aml_hw;

    aml_hw->mod_params->amsdu_maxnb = amsdu_max;
    aml_adjust_amsdu_maxnb(aml_hw);
    return 0;
}

static int aml_set_amsdu_tx(struct net_device *dev, int amsdu_tx)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw * aml_hw = aml_vif->aml_hw;

    if (aml_hw->mod_params->amsdu_tx == amsdu_tx) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "amsdu tx did not change, ignore\n");
        return 0;
    }

    aml_hw->mod_params->amsdu_tx = amsdu_tx;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set amsdu_tx:0x%x success\n", amsdu_tx);
    return _aml_set_amsdu_tx(aml_hw, amsdu_tx);
}

static int aml_set_ldpc(struct net_device *dev, int ldpc)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw * aml_hw = aml_vif->aml_hw;

    if (aml_hw->mod_params->ldpc_on == ldpc) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "ldpc did not change, ignore\n");
        return 0;
    }
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set ldpc: 0x%x success\n", ldpc);
    aml_hw->mod_params->ldpc_on = ldpc;
    // LDPC is mandatory for HE40 and above, so if LDPC is not supported, then disable
    // support for 40 and 80MHz
    if (aml_hw->mod_params->he_on && !aml_hw->mod_params->ldpc_on)
    {
        aml_hw->mod_params->use_80 = false;
        aml_hw->mod_params->use_2040 = false;
    }
    aml_set_he_capa(aml_hw, aml_hw->wiphy);
    aml_set_vht_capa(aml_hw, aml_hw->wiphy);
    aml_set_ht_capa(aml_hw, aml_hw->wiphy);

    return aml_set_ldpc_tx(aml_hw, aml_vif);
}

static int aml_set_tx_lft(struct net_device *dev, int tx_lft)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw * aml_hw = aml_vif->aml_hw;

    if (aml_hw->mod_params->tx_lft == tx_lft) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "tx_lft did not change, ignore\n");
        return 0;
    }

    aml_hw->mod_params->tx_lft= tx_lft;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set tx_lft:0x%x success\n", tx_lft);
    return _aml_set_tx_lft(aml_hw, tx_lft);
}

static int aml_set_ps_mode(struct net_device *dev, int ps_mode)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw * aml_hw = aml_vif->aml_hw;
    if ((ps_mode != MM_PS_MODE_OFF) && (ps_mode != MM_PS_MODE_ON) && (ps_mode != MM_PS_MODE_ON_DYN))
    {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "param err, please reset\n");
        return -1;
    }
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set ps_mode:0x%x success\n", ps_mode);
    return aml_send_me_set_ps_mode(aml_hw, ps_mode);
}

int aml_set_early_bcn_mode(struct net_device *dev, char *str_param, union iwreq_data *wrqu, char *extra)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw * aml_hw = aml_vif->aml_hw;
    struct early_bcn early_beacon;

    early_beacon.early_bcn_mode = 0;
    early_beacon.ie_counter = 0;
    early_beacon.element_1 = 0;
    early_beacon.element_2 = 0;
    early_beacon.element_3 = 0;
    early_beacon.element_4 = 0;

    if (sscanf(str_param, "%d %d %d %d %d %d", &early_beacon.early_bcn_mode,
        &early_beacon.ie_counter, &early_beacon.element_1, &early_beacon.element_2, &early_beacon.element_3, &early_beacon.element_4) != 6) {
        printk("set_early_beacon_end erro \n");
    }

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set early beacon mode: 0x%x success\n", early_beacon.early_bcn_mode);
    return aml_send_early_beacon_mode(aml_hw, &early_beacon);
}

static int aml_send_twt_req(struct net_device *dev, char *str_param, union iwreq_data *wrqu, char *extra)
{
    struct twt_conf_tag twt_conf;
    struct twt_setup_cfm twt_setup_cfm;
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    u8 setup_type = MAC_TWT_SETUP_REQ;
    u8 vif_idx = aml_vif->vif_index;

    twt_conf.flow_type = 0;
    twt_conf.wake_int_exp = 0;
    twt_conf.wake_dur_unit = 0;
    twt_conf.min_twt_wake_dur = 0;
    twt_conf.wake_int_mantissa = 0;

    if (sscanf(str_param, "%d %d %d %d %d %d", &setup_type,
        &twt_conf.flow_type, &twt_conf.wake_int_exp, &twt_conf.wake_dur_unit, &twt_conf.min_twt_wake_dur, &twt_conf.wake_int_mantissa) != 6) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "param erro \n");
    }

    wrqu->data.length = scnprintf(extra, IW_PRIV_SIZE_MASK, "setup_type=%d flow_type=%d wake_int_exp=%d wake_dur_unit=%d min_twt_wake_dur=%d wake_int_mantissa=%d",
        setup_type, twt_conf.flow_type, twt_conf.wake_int_exp,  twt_conf.wake_dur_unit,  twt_conf.min_twt_wake_dur, twt_conf.wake_int_mantissa);
    wrqu->data.length++;

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "%s [%s]\n", __func__, extra);
    return aml_send_twt_request(aml_hw, setup_type, vif_idx, &twt_conf, &twt_setup_cfm);
}

void aml_print_buf(char *buf, int len)
{
    char tmp[PRINT_BUF_SIZE] = {0};
    while (1) {
        if (len > PRINT_BUF_SIZE) {
            memcpy(tmp, buf, PRINT_BUF_SIZE);
            AML_INFO("%s", tmp);
            len -= PRINT_BUF_SIZE;
            buf += PRINT_BUF_SIZE;
        } else {
            AML_INFO("%s\n", buf);
            break;
        }
    }
}

int aml_print_acs_info(struct aml_hw *priv)
{
    struct wiphy *wiphy = priv->wiphy;
    char *buf = NULL;
    int buf_size = (SCAN_CHANNEL_MAX + 1) * 43;
    int survey_cnt = 0;
    int len = 0;
    int band, chan_cnt;

    if ((buf = vmalloc(buf_size)) == NULL)
        return -1;

    mutex_lock(&priv->dbgdump.mutex);
    len += scnprintf(buf, buf_size - 1,  "FREQ      TIME(ms)     BUSY(ms)     NOISE(dBm)\n");

    for (band = NL80211_BAND_2GHZ; band <= NL80211_BAND_5GHZ; band++) {
        for (chan_cnt = 0; chan_cnt < wiphy->bands[band]->n_channels; chan_cnt++) {
            struct aml_survey_info *p_survey_info = &priv->survey[survey_cnt];
            struct ieee80211_channel *p_chan = &wiphy->bands[band]->channels[chan_cnt];

            if (p_survey_info->filled) {
                len += scnprintf(&buf[len], buf_size - len - 1,
                                  "%d    %03d         %03d          %d\n",
                                  p_chan->center_freq,
                                  p_survey_info->chan_time_ms,
                                  p_survey_info->chan_time_busy_ms,
                                  p_survey_info->noise_dbm);
            } else {
                len += scnprintf(&buf[len], buf_size -len -1,
                                  "%d    NOT AVAILABLE\n",
                                  p_chan->center_freq);
            }

            survey_cnt++;
        }
    }

    mutex_unlock(&priv->dbgdump.mutex);
    aml_print_buf(buf, len);
    vfree(buf);
    return 0;
}

int aml_print_last_rx_info(struct aml_hw *priv, struct aml_sta *sta)
{
#ifdef CONFIG_AML_DEBUGFS
    struct aml_rx_rate_stats *rate_stats;
    char *buf;
    int bufsz, i, len = 0;
    unsigned int fmt, pre, bw, nss, mcs, gi, dcm = 0;
    struct rx_vector_1 *last_rx;
    char hist[] = "##################################################";
    int hist_len = sizeof(hist) - 1;
    u8 nrx;
    rate_stats = &sta->stats.rx_rate;
    bufsz = (rate_stats->rate_cnt * ( 50 + hist_len) + 200);
    buf = kmalloc(bufsz + 1, GFP_ATOMIC);
    if (buf == NULL)
        return 0;

    // Get number of RX paths
    nrx = (priv->version_cfm.version_phy_1 & MDM_NRX_MASK) >> MDM_NRX_LSB;

    len += scnprintf(buf, bufsz,
                        "\nRX rate info for %02X:%02X:%02X:%02X:%02X:%02X:\n",
                        sta->mac_addr[0], sta->mac_addr[1], sta->mac_addr[2],
                        sta->mac_addr[3], sta->mac_addr[4], sta->mac_addr[5]);

    // Display Statistics
    for (i = 0; i < rate_stats->size; i++) {
        if (rate_stats->table && rate_stats->table[i]) {
            union aml_rate_ctrl_info rate_config;
            u64 percent = div_u64(rate_stats->table[i] * 1000, rate_stats->cpt);
            u64 p;
            int ru_size;
            u32 rem;

            idx_to_rate_cfg(i, &rate_config, &ru_size);
            len += print_rate_from_cfg(&buf[len], bufsz - len,
                                       rate_config.value, NULL, ru_size);
            p = div_u64((percent * hist_len), 1000);
            len += scnprintf(&buf[len], bufsz - len, ": %9d(%2d.%1d%%)%.*s\n",
                             rate_stats->table[i],
                             div_u64_rem(percent, 10, &rem), rem, p, hist);
        }
    }

    // Display detailed info of the last received rate
    last_rx = &sta->stats.last_rx.rx_vect1;
    len += scnprintf(&buf[len], bufsz - len,"\nLast received rate\n"
                     "type               rate     LDPC STBC BEAMFM DCM DOPPLER %s\n",
                     (nrx > 1) ? "rssi1(dBm) rssi2(dBm)" : "rssi(dBm)");

    fmt = last_rx->format_mod;
    bw = last_rx->ch_bw;
    pre = last_rx->pre_type;
    if (fmt >= FORMATMOD_HE_SU) {
        mcs = last_rx->he.mcs;
        nss = last_rx->he.nss;
        gi = last_rx->he.gi_type;
        if ((fmt == FORMATMOD_HE_MU) || (fmt == FORMATMOD_HE_ER))
            bw = last_rx->he.ru_size;
        dcm = last_rx->he.dcm;
    } else if (fmt == FORMATMOD_VHT) {
        mcs = last_rx->vht.mcs;
        nss = last_rx->vht.nss;
        gi = last_rx->vht.short_gi;
    } else if (fmt >= FORMATMOD_HT_MF) {
        mcs = last_rx->ht.mcs % 8;
        nss = last_rx->ht.mcs / 8;
        gi = last_rx->ht.short_gi;
    } else {
        BUG_ON((mcs = legrates_lut[last_rx->leg_rate].idx) == -1);
        nss = 0;
        gi = 0;
    }

    len += print_rate(&buf[len], bufsz - len, fmt, nss, mcs, bw, gi, pre, dcm, NULL);

    /* flags for HT/VHT/HE */
    if (fmt >= FORMATMOD_HE_SU) {
        len += scnprintf(&buf[len], bufsz - len, "  %c    %c     %c    %c     %c",
                         last_rx->he.fec ? 'L' : ' ',
                         last_rx->he.stbc ? 'S' : ' ',
                         last_rx->he.beamformed ? 'B' : ' ',
                         last_rx->he.dcm ? 'D' : ' ',
                         last_rx->he.doppler ? 'D' : ' ');
    } else if (fmt == FORMATMOD_VHT) {
        len += scnprintf(&buf[len], bufsz - len, "  %c    %c     %c           ",
                         last_rx->vht.fec ? 'L' : ' ',
                         last_rx->vht.stbc ? 'S' : ' ',
                         last_rx->vht.beamformed ? 'B' : ' ');
    } else if (fmt >= FORMATMOD_HT_MF) {
        len += scnprintf(&buf[len], bufsz - len, "  %c    %c                  ",
                         last_rx->ht.fec ? 'L' : ' ',
                         last_rx->ht.stbc ? 'S' : ' ');
    } else {
        len += scnprintf(&buf[len], bufsz - len, "                         ");
    }
    if (nrx > 1) {
        /* coverity[assigned_value] - len is used */
        len += scnprintf(&buf[len], bufsz - len, "       %-4d       %d\n",
                         last_rx->rssi1, last_rx->rssi1);
    } else {
        /* coverity[assigned_value] - len is used */
        len += scnprintf(&buf[len], bufsz - len, "      %d\n", last_rx->rssi1);
    }

    aml_print_buf(buf, len);
    kfree(buf);
#endif
    return 0;
}

int aml_print_stats(struct aml_hw *priv)
{
    char *buf;
    int ret;
    int i, skipped;
#ifdef CONFIG_AML_SPLIT_TX_BUF
    int per;
#endif
    int bufsz = (NX_TXQ_CNT) * 20 + (ARRAY_SIZE(priv->stats->amsdus_rx) + 1) * 40
        + (ARRAY_SIZE(priv->stats->ampdus_tx) * 30);

    buf = kmalloc(bufsz, GFP_ATOMIC);
    if (buf == NULL)
        return 0;

    ret = scnprintf(buf, bufsz, "TXQs CFM balances ");
    for (i = 0; i < NX_TXQ_CNT; i++)
        ret += scnprintf(&buf[ret], bufsz - ret,
                            "    [%1d]:%3d", i,
                            priv->stats->cfm_balance[i]);

    ret += scnprintf(&buf[ret], bufsz - ret, "\n");

#ifdef CONFIG_AML_SPLIT_TX_BUF
    ret += scnprintf(&buf[ret], bufsz - ret,
                       "\nAMSDU[len]         done          failed   received\n");
    for (i = skipped = 0; i < NX_TX_PAYLOAD_MAX; i++) {
        if (priv->stats->amsdus[i].done) {
            per = DIV_ROUND_UP((priv->stats->amsdus[i].failed) *
                                100, priv->stats->amsdus[i].done);
        } else if (priv->stats->amsdus_rx[i]) {
            per = 0;
        } else {
            per = 0;
            skipped = 1;
            continue;
        }
        if (skipped) {
            ret += scnprintf(&buf[ret], bufsz - ret, "     ...\n");
            skipped = 0;
        }

        ret += scnprintf(&buf[ret], bufsz - ret,
                          "     [%2d]      %10d %8d(%3d%%) %10d\n",    i ? i + 1 : i,
                          priv->stats->amsdus[i].done,
                          priv->stats->amsdus[i].failed, per,
                          priv->stats->amsdus_rx[i]);
    }

    for (; i < ARRAY_SIZE(priv->stats->amsdus_rx); i++) {
        if (!priv->stats->amsdus_rx[i]) {
            skipped = 1;
            continue;
        }
        if (skipped) {
            ret += scnprintf(&buf[ret], bufsz - ret, "     ...\n");
            skipped = 0;
        }

        ret += scnprintf(&buf[ret], bufsz - ret,
                          "     [%2d]                                %10d\n",
                          i + 1, priv->stats->amsdus_rx[i]);
    }
#else
    ret += scnprintf(&buf[ret], bufsz - ret,
                      "\nAMSDU[len]     received\n");
    for (i = skipped = 0; i < ARRAY_SIZE(priv->stats->amsdus_rx); i++) {
        if (!priv->stats->amsdus_rx[i]) {
            skipped = 1;
            continue;
        }
        if (skipped) {
            ret += scnprintf(&buf[ret], bufsz - ret,  " ...\n");
            skipped = 0;
        }

        ret += scnprintf(&buf[ret], bufsz - ret,
                          "     [%2d]      %10d\n",
                          i + 1, priv->stats->amsdus_rx[i]);
    }

#endif /* CONFIG_AML_SPLIT_TX_BUF */

    ret += scnprintf(&buf[ret], bufsz - ret,
                       "\nAMPDU[len]       done  received\n");
    for (i = skipped = 0; i < ARRAY_SIZE(priv->stats->ampdus_tx); i++) {
        if (!priv->stats->ampdus_tx[i] && !priv->stats->ampdus_rx[i]) {
            skipped = 1;
            continue;
        }
        if (skipped) {
            ret += scnprintf(&buf[ret], bufsz - ret,  "   ...\n");
            skipped = 0;
        }

        ret += scnprintf(&buf[ret], bufsz - ret,
                         "     [%2d]     %9d %9d\n", i ? i + 1 : i,
                         priv->stats->ampdus_tx[i], priv->stats->ampdus_rx[i]);
    }
    /* coverity[assigned_value] - len is used */
    ret += scnprintf(&buf[ret], bufsz - ret,
                     "#mpdu missed          %9d\n",
                     priv->stats->ampdus_rx_miss);

    aml_print_buf(buf, ret);
    kfree(buf);
    return 0;

}

int aml_print_rate_info( struct aml_hw *aml_hw, struct aml_sta *sta)
{
    char *buf;
    int bufsz, len = 0;
    int i = 0;
    int error = 0;
    struct me_rc_stats_cfm me_rc_stats_cfm;
    unsigned int no_samples;
    struct st *st;
     /* Forward the information to the LMAC */
    if ((error = aml_send_me_rc_stats(aml_hw, sta->sta_idx, &me_rc_stats_cfm)))
        return error;

    no_samples = me_rc_stats_cfm.no_samples;
    if (no_samples == 0)
        return 0;

    bufsz = no_samples * LINE_MAX_SZ + 500;

    buf = kmalloc(bufsz + 1, GFP_ATOMIC);
    if (buf == NULL)
        return 0;

    st = kmalloc(sizeof(struct st) * no_samples, GFP_ATOMIC);
    if (st == NULL) {
        kfree(buf);
        return 0;
    }

    for (i = 0; i < no_samples; i++) {
        unsigned int tp, eprob;
        len = print_rate_from_cfg(st[i].line, LINE_MAX_SZ,
                                  me_rc_stats_cfm.rate_stats[i].rate_config,
                                  (int *)&st[i].r_idx, 0);

        if (me_rc_stats_cfm.sw_retry_step != 0) {
            len += scnprintf(&st[i].line[len], LINE_MAX_SZ - len,  "%c",
                    me_rc_stats_cfm.retry_step_idx[me_rc_stats_cfm.sw_retry_step] == i ? '*' : ' ');
        }
        else {
            len += scnprintf(&st[i].line[len], LINE_MAX_SZ - len, " ");
        }
        len += scnprintf(&st[i].line[len], LINE_MAX_SZ - len, "%c",
                me_rc_stats_cfm.retry_step_idx[0] == i ? 'T' : ' ');
        len += scnprintf(&st[i].line[len], LINE_MAX_SZ - len, "%c",
                me_rc_stats_cfm.retry_step_idx[1] == i ? 't' : ' ');
        len += scnprintf(&st[i].line[len], LINE_MAX_SZ - len, "%c ",
                me_rc_stats_cfm.retry_step_idx[2] == i ? 'P' : ' ');

        tp = me_rc_stats_cfm.tp[i] / 10;
        len += scnprintf(&st[i].line[len], LINE_MAX_SZ - len, " %4u.%1u",
                         tp / 10, tp % 10);

        eprob = ((me_rc_stats_cfm.rate_stats[i].probability * 1000) >> 16) + 1;
        scnprintf(&st[i].line[len],LINE_MAX_SZ - len,
                         "  %4u.%1u %5u(%6u)  %6u",
                         eprob / 10, eprob % 10,
                         me_rc_stats_cfm.rate_stats[i].success,
                         me_rc_stats_cfm.rate_stats[i].attempts,
                         me_rc_stats_cfm.rate_stats[i].sample_skipped);
    }
    len = scnprintf(buf, bufsz ,
                     "\nTX rate info for %02X:%02X:%02X:%02X:%02X:%02X:\n",
                     sta->mac_addr[0], sta->mac_addr[1], sta->mac_addr[2],
                     sta->mac_addr[3], sta->mac_addr[4], sta->mac_addr[5]);

    len += scnprintf(&buf[len], bufsz - len,
            "   # type               rate             tpt   eprob    ok(   tot)   skipped\n");

    // add sorted statistics to the buffer
    sort(st, no_samples, sizeof(st[0]), compare_idx, NULL);
    for (i = 0; i < no_samples; i++) {
        len += scnprintf(&buf[len], bufsz - len, "%s\n", st[i].line);
    }

    // display HE TB statistics if any
    if (me_rc_stats_cfm.rate_stats[RC_HE_STATS_IDX].rate_config != 0) {
        unsigned int tp, eprob;
        struct rc_rate_stats *rate_stats = &me_rc_stats_cfm.rate_stats[RC_HE_STATS_IDX];
        int ru_index = rate_stats->ru_and_length & 0x07;
        int ul_length = rate_stats->ru_and_length >> 3;

        len += scnprintf(&buf[len], bufsz - len,
                         "\nHE TB rate info:\n");

        len += scnprintf(&buf[len], bufsz - len,
                "     type               rate             tpt   eprob    ok(   tot)   ul_length\n     ");
        len += print_rate_from_cfg(&buf[len], bufsz - len, rate_stats->rate_config,
                                   NULL, ru_index);

        tp = me_rc_stats_cfm.tp[RC_HE_STATS_IDX] / 10;
        len += scnprintf(&buf[len], bufsz - len, "      %4u.%1u",
                         tp / 10, tp % 10);

        eprob = ((rate_stats->probability * 1000) >> 16) + 1;
        len += scnprintf(&buf[len],bufsz - len,
                         "  %4u.%1u %5u(%6u)  %6u\n",
                         eprob / 10, eprob % 10,
                         rate_stats->success,
                         rate_stats->attempts,
                         ul_length);
    }

    len += scnprintf(&buf[len], bufsz - len, "\n MPDUs AMPDUs AvLen trialP");
    /* coverity[assigned_value] - len is used */
    len += scnprintf(&buf[len], bufsz - len, "\n%6u %6u %3d.%1d %6u\n",
                     me_rc_stats_cfm.ampdu_len,
                     me_rc_stats_cfm.ampdu_packets,
                     me_rc_stats_cfm.avg_ampdu_len >> 16,
                     ((me_rc_stats_cfm.avg_ampdu_len * 10) >> 16) % 10,
                     me_rc_stats_cfm.sample_wait);

    aml_print_buf(buf, len);
    kfree(buf);
    kfree(st);

    return 0;
}
static int aml_get_rate_info(struct net_device *dev)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    u8 i = 0;
    struct aml_sta *sta = NULL;
    for (i = 0; i < NX_REMOTE_STA_MAX; i++) {
        sta = aml_hw->sta_table + i;
        if (sta && sta->valid && (aml_vif->vif_index == sta->vif_idx)) {
            aml_print_rate_info(aml_hw, sta);
        }
    }
    return 0;
}

static int aml_cca_check(struct net_device *dev)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    reg_cca_cond_get(aml_hw);
    return 0;
}

static int aml_get_tx_stats(struct net_device *dev)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;

    aml_print_stats(aml_hw);
    return 0;
}

static int aml_get_acs_info(struct net_device *dev)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;

    aml_print_acs_info(aml_hw);
    return 0;
}

static int aml_get_clock(struct net_device *dev)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    struct aml_plat *aml_plat = aml_hw->plat;
    uint32_t temp_value =0;


    if (PCIE_MODE == aml_bus_type)
    {
        AML_REG_WRITE(0xffffffff, aml_plat, AML_ADDR_SYSTEM, CLK_ADDR0 );
        AML_REG_WRITE(0xffffffff, aml_plat, AML_ADDR_SYSTEM, CLK_ADDR1 );
        temp_value = AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, ENA_CLK_ADDR);
        temp_value |= BIT(1);
        AML_REG_WRITE(temp_value, aml_plat, AML_ADDR_SYSTEM, ENA_CLK_ADDR );
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "clock measure start (MHz):\n 0x60805344 dac\t: %d \n 0x60805340 plf\t: %d \n 0x6080533c macwt\t: %d \n 0x60805338 macdaccore: %d \n\
               0x60805334 la\t: %d\n 0x60805330 mpif\t: %d\n 0x6080532c phy\t: %d\n 0x60805328 vtb\t: %d\n 0x60805324 feref\t: %d\n\
               0x60805320 ref80\t: %d\n 0x6080531c ref40\t: %d\n 0x60805314 ldpc_rx\t: %d \n 0x60805310 ref_44\t: %d\nclock measure end",
               AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, DAC_CLK_ADDR)/1000,
               AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, PLF_CLK_ADDR)/1000,
               AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, MACWT_CLK_ADDR)/1000,
               AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, MACCORE_CLK_ADDR)/1000,
               AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, LA_CLK_ADDR)/1000,
               AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, MPIF_CLK_ADDR)/1000,
               AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, PHY_CLK_ADDR)/1000,
               AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, VTB_CLK_ADDR)/1000,
               AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, FEREF_CLK_ADDR)/1000,
               AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, REF80_CLK_ADDR)/1000,
               AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, REF40_CLK_ADDR)/1000,
               AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, LDPC_RX_CLK_ADDR)/1000,
               AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, REF_44_CLK_ADDR)/1000);
    }
    return 0;
}


static int aml_get_chan_list_info(struct net_device *dev)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    struct wiphy *wiphy = aml_hw->wiphy;
    int i;
    const struct ieee80211_reg_rule *reg_rule;

    if (wiphy->bands[NL80211_BAND_2GHZ] != NULL) {
        struct ieee80211_supported_band *b = wiphy->bands[NL80211_BAND_2GHZ];
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "2.4G channels\n");
        for (i = 0; i < b->n_channels; i++) {
            if (b->channels[i].flags & IEEE80211_CHAN_DISABLED)
                continue;
            reg_rule = freq_reg_info(wiphy, MHZ_TO_KHZ(b->channels[i].center_freq));
            if (IS_ERR(reg_rule))
                continue;
            AML_PRINT(AML_DBG_MODULES_IWPRIV, "channel:%d\tfrequency:%d\tmax_bandwidth:%dMHz\t\n",
                aml_ieee80211_freq_to_chan(b->channels[i].center_freq, NL80211_BAND_2GHZ),
                b->channels[i].center_freq, KHZ_TO_MHZ(reg_rule->freq_range.max_bandwidth_khz));
            if (i == MAC_DOMAINCHANNEL_24G_MAX)
                break;
        }
    }

    if (wiphy->bands[NL80211_BAND_5GHZ] != NULL) {
        struct ieee80211_supported_band *b = wiphy->bands[NL80211_BAND_5GHZ];
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "5G channels:\n");
        for (i = 0; i < b->n_channels; i++) {
            if (b->channels[i].flags & IEEE80211_CHAN_DISABLED)
                continue;
            reg_rule = freq_reg_info(wiphy, MHZ_TO_KHZ(b->channels[i].center_freq));
            if (IS_ERR(reg_rule))
                continue;
            AML_PRINT(AML_DBG_MODULES_IWPRIV, "channel:%d\tfrequency:%d\tmax_bandwidth:%dMHz\t\n",
                aml_ieee80211_freq_to_chan(b->channels[i].center_freq, NL80211_BAND_5GHZ),
                b->channels[i].center_freq, KHZ_TO_MHZ(reg_rule->freq_range.max_bandwidth_khz));
            if (i == MAC_DOMAINCHANNEL_5G_MAX)
                break;
        }
    }

    return 0;
}

static void aml_get_rx_regvalue(struct aml_plat *aml_plat)
{
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "<-------------------rx reg value start --------------------->\n");
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "rx_end     :0x%x\n", AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, 0xc06088));
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "frame_ok   :0x%x\n", AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, 0xc06080));
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "frame_bad  :0x%x\n", AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, 0xc06084));
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "rx_error   :0x%x\n", AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, 0xc0608c));
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "phy_error  :0x%x\n", AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, 0xc06098));

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "rxbuffer1--->:\n");
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "start      :0x%x\n", AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, 0xb081c8));
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "end        :0x%x\n", AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, 0xb081cc));
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "read       :0x%x\n", AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, 0xb081d0));
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "write      :0x%x\n", AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, 0xb081d4));

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "rxbuffer2--->:\n");
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "start      :0x%x\n", AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, 0xb081d8));
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "end        :0x%x\n", AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, 0xb081dc));
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "read       :0x%x\n", AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, 0xb081e0));
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "write      :0x%x\n", AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, 0xb081e4));

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "SNR        :0x%x\n", AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, 0xc0005c)&0xfff);

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "data avg rssi :%d dBm\n", ((AML_REG_READ(aml_plat, AML_ADDR_MAC_PHY, REG_OF_SYNC_RSSI)&0xffff0000) >> 16) - 256);

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "bcn  avg rssi :%d dBm\n", (AML_REG_READ(aml_plat, AML_ADDR_MAC_PHY, REG_OF_SYNC_RSSI)&0xffff) - 256);

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "<-------------------rx reg value end ---------------------->\n");
}
static int aml_get_last_rx(struct net_device *dev)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    struct aml_plat *aml_plat = aml_hw->plat;
    u8 i = 0;
    struct aml_sta *sta = NULL;
    for (i = 0; i < NX_REMOTE_STA_MAX; i++) {
        sta = aml_hw->sta_table + i;
        if (sta && sta->valid && (aml_vif->vif_index == sta->vif_idx)) {
            aml_print_last_rx_info(aml_hw, sta);
            aml_get_rx_regvalue(aml_plat);
        }
    }
    return 0;
}

static int aml_clear_last_rx(struct net_device *dev)
{
#ifdef CONFIG_AML_DEBUGFS
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    u8 i = 0;
    struct aml_sta *sta = NULL;

    for (i = 0; i < NX_REMOTE_STA_MAX; i++) {
        sta = aml_hw->sta_table + i;
        if (sta && sta->valid && (aml_vif->vif_index == sta->vif_idx)) {
            /* Prevent from interrupt preemption as these statistics are updated under
             * interrupt */
            spin_lock_bh(&aml_hw->tx_lock);
            memset(sta->stats.rx_rate.table, 0,
                   sta->stats.rx_rate.size * sizeof(sta->stats.rx_rate.table[0]));
            sta->stats.rx_rate.cpt = 0;
            sta->stats.rx_rate.rate_cnt = 0;
            spin_unlock_bh(&aml_hw->tx_lock);
        }
    }
#endif
    return 0;
}


static int aml_get_amsdu_max(struct net_device *dev)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "current amsdu_max: %d\n", aml_hw->mod_params->amsdu_maxnb);
    return 0;
}

#ifdef CONFIG_SDIO_TX_ENH
#ifdef SDIO_TX_ENH_DBG
extern block_log blog;
extern cfm_log cfmlog;
static int aml_get_sdio_tx_enh_stats(struct net_device *dev)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    int i = 0, j = 0;
    uint32_t delta_tsf;

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "<-------------------intface block info-------------------->\n");
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "avg_page    :%u\n", blog.avg_page);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "block_cnt   :%u\n", blog.block_cnt);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "avg_blk_time:%u\n", blog.avg_block);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "block_rate  :%u\n", blog.block_rate);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "avg_blk_rate:%u\n", blog.avg_blk_rate);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "<-------------------intface cfm info --------------------->\n");
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "cfm rx cnt  :%u\n", cfmlog.cfm_rx_cnt);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "avg_cfm     :%u\n", cfmlog.avg_cfm);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "avg_cfm_page:%u\n", cfmlog.avg_cfm_page);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "cur_cfm_num :%u\n", cfmlog.cfm_num);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "cfm_read_cnt :%u\n", cfmlog.cfm_read_cnt);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "cfm_read_avg_blk :%u\n", cfmlog.cfm_read_blk_cnt/cfmlog.cfm_read_cnt);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "<------------------------rx info-------------------------->\n");
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "rx cnt in rx:%u\n", cfmlog.rx_cnt_in_rx);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "mpdu in rx      :%u\n", cfmlog.mpdu_in_rx);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "avg_mpdu in one rx:%u\n", cfmlog.avg_mpdu_in_one_rx);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "<---------------------- amsdu log ------------------------>\n");
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "tx total count      :%u\n", blog.tx_tot_cnt);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "tx amsdu rate       :%u\n", blog.tx_amsdu_cnt*1000/blog.tx_tot_cnt);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "tx non-amsdu rate   :%u\n", (blog.tx_tot_cnt - blog.tx_amsdu_cnt) * 1000/blog.tx_tot_cnt);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "AMSDU_NUM: 1:%u, 2:%u, 3:%u, 4:%u, 5:%u, 6:%u\n",
        blog.amsdu_num[0], blog.amsdu_num[1], blog.amsdu_num[2], blog.amsdu_num[3], blog.amsdu_num[4], blog.amsdu_num[5]);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "AMSDU_NUM ratio: 1:%u, 2:%u, 3:%u, 4:%u, 5:%u, 6:%u\n",
        blog.amsdu_num[0]*1000/blog.tx_tot_cnt, blog.amsdu_num[1]*1000/blog.tx_tot_cnt,
        blog.amsdu_num[2]*1000/blog.tx_tot_cnt, blog.amsdu_num[3]*1000/blog.tx_tot_cnt,
        blog.amsdu_num[4]*1000/blog.tx_tot_cnt, blog.amsdu_num[5]*1000/blog.tx_tot_cnt);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "<-------------------intface log end ---------------------->\n");

    return 0;
}

static int aml_reset_sdio_tx_enh_stats(struct net_device *dev)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;

    spin_lock_bh(&aml_hw->tx_lock);
    memset(&blog, 0, sizeof(blog));
    memset(&cfmlog, 0, sizeof(cfmlog));
    spin_unlock_bh(&aml_hw->tx_lock);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "SDIO TX enhance stats reset done\n");

    return 0;
}
#endif

static int aml_set_txcfm_read_thresh(struct net_device *dev, int thresh)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw * aml_hw = aml_vif->aml_hw;

    if (aml_hw->txcfm_param.read_thresh == thresh) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "txcfm read threshold isn't changed, ignore\n");
        return 0;
    }

    aml_hw->txcfm_param.read_thresh = thresh;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set txcfm_read_thresh:0x%x success\n", thresh);
    return 0;
}

static int aml_set_irqless_flag(struct net_device *dev, int flag)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw * aml_hw = aml_vif->aml_hw;

    if (aml_hw->irqless_flag == !!flag) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "irqless flag isn't changed, ignore\n");
        return 0;
    }

    aml_hw->irqless_flag = !!flag;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set irqless flag:0x%x success\n", flag);
    return 0;
}

static int aml_set_dyn_txcfm(struct net_device *dev, int en)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw * aml_hw = aml_vif->aml_hw;

    if (aml_hw->txcfm_param.dyn_en == !!en) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "txcfm dyn_en isn't changed, ignore\n");
        return 0;
    }

    spin_lock_bh(&aml_hw->txcfm_rd_lock);
    memset(&aml_hw->txcfm_param, 0, sizeof(txcfm_param_t));
    aml_hw->txcfm_param.read_blk = 6;
    aml_hw->txcfm_param.read_thresh = TXCFM_THRESH;
    spin_unlock_bh(&aml_hw->txcfm_rd_lock);
    spin_lock_init(&aml_hw->txcfm_rd_lock);

    aml_hw->txcfm_param.dyn_en = !!en;

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "txcfm dyn_en:0x%x success\n", en);

    return 0;
}
#endif

static int aml_get_amsdu_tx(struct net_device *dev)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "current amsdu_tx: %d\n", aml_hw->mod_params->amsdu_tx);
    return 0;
}

static int aml_get_ldpc(struct net_device *dev)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "current ldpc: %d\n", aml_hw->mod_params->ldpc_on);
    return 0;
}

static int aml_get_tx_lft(struct net_device *dev)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "current tx_lft: %d\n", aml_hw->mod_params->tx_lft);
    return 0;
}


int aml_get_txq(struct net_device *dev)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    struct aml_vif *vif;
    char *buf;
    int idx, res;
    size_t bufsz = ((NX_VIRT_DEV_MAX * (VIF_HDR_MAX_LEN + 2 * VIF_SEP_LEN)) +
                    (NX_REMOTE_STA_MAX * STA_HDR_MAX_LEN) +
                    ((NX_REMOTE_STA_MAX + NX_VIRT_DEV_MAX + NX_NB_TXQ) *
                     TXQ_HDR_MAX_LEN) + CAPTION_LEN);

    buf = kmalloc(bufsz, GFP_ATOMIC);
    if (buf == NULL)
        return 0;

    bufsz--;
    idx = 0;

    res = scnprintf(&buf[idx], bufsz, CAPTION);
    idx += res;
    bufsz -= res;

    //spin_lock_bh(&aml_hw->tx_lock);
    list_for_each_entry(vif, &aml_hw->vifs, list) {
        res = scnprintf(&buf[idx], bufsz, "\n"VIF_SEP);
        idx += res;
        bufsz -= res;
        res = aml_dbgfs_txq_vif(&buf[idx], bufsz, vif, aml_hw);
        idx += res;
        bufsz -= res;
        res = scnprintf(&buf[idx], bufsz, VIF_SEP);
        idx += res;
        bufsz -= res;
    }
    //spin_unlock_bh(&aml_hw->tx_lock);

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "%s\n", buf);
    kfree(buf);

    return 0;
}

static int aml_get_buf_state(struct net_device *dev)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    if (aml_hw->rx_buf_state & FW_BUFFER_EXPAND) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "rxbuf: 256k, txbuf: 128k\n");
    } else if (aml_hw->rx_buf_state & FW_BUFFER_NARROW) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "rxbuf: 128k, txbuf: 256k\n");
    } else {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "err: rx_buf_state[%x]\n", aml_hw->rx_buf_state);
    }

    return 0;
}

/*
    buf_state: set sdio rx&tx Dynamic buf state
    param buf_state:
                                      0:disable force buf state
    BUFFER_RX_FORCE_REDUCE   BIT(9)   512:force txbuf large
    BUFFER_RX_FORCE_ENLARGE BIT(10)   1024:force rxbuf large
*/
static int aml_set_buf_state(struct net_device *dev, int buf_state)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;

    if (buf_state != 0 && buf_state != BUFFER_RX_FORCE_REDUCE && buf_state != BUFFER_RX_FORCE_ENLARGE)
    {
        AML_INFO("param error!\n")
        return -1;
    }
    return aml_send_set_buf_state_req(aml_hw, buf_state);
}

static int aml_get_tcp_ack_info(struct net_device *dev)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    struct aml_tcp_sess_mgr *ack_mgr = &aml_hw->ack_mgr;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "ack_mgr->max_drop_cnt=%u\n", atomic_read(&ack_mgr->max_drop_cnt));
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "ack_mgr->enable=%u\n", atomic_read(&ack_mgr->enable));
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "ack_mgr->max_timeout=%u\n", atomic_read(&ack_mgr->max_timeout));
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "ack_mgr->dynamic_adjust=%u\n", atomic_read(&ack_mgr->dynamic_adjust));
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "ack_mgr->total_drop_cnt=%u\n", ack_mgr->total_drop_cnt);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "ack_mgr->ack_winsize=%u\n", atomic_read(&ack_mgr->ack_winsize));
    return 0;
}

static int aml_set_txpage_once(struct net_device *dev, int txpage)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw * aml_hw = aml_vif->aml_hw;

    if (aml_hw->g_tx_param.tx_page_once == txpage) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "txpage did not change, ignore\n");
        return 0;
    }

    aml_hw->g_tx_param.tx_page_once = txpage;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set tx_page_once:0x%x success\n", txpage);
    return 0;
}

static int aml_set_txcfm_tri_tx(struct net_device *dev, int tri_tx_thr)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw * aml_hw = aml_vif->aml_hw;

    if (aml_hw->g_tx_param.txcfm_trigger_tx_thr == tri_tx_thr) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "tri_tx_thr did not change, ignore\n");
        return 0;
    }

    aml_hw->g_tx_param.txcfm_trigger_tx_thr = tri_tx_thr;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set tri_tx_thr:0x%x success\n", tri_tx_thr);
    return 0;
}

static int aml_set_tsq(struct net_device *dev, int tsq)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw * aml_hw = aml_vif->aml_hw;

    if (aml_hw->tsq == tsq) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "tcp tsq did not change, ignore\n");
        return 0;
    }

    aml_hw->tsq = tsq;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set tcp tsq:0x%x success\n", aml_hw->tsq);
    return 0;
}

static int aml_set_tcp_delay_ack(struct net_device *dev, int enable,int min_size)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw * aml_hw = aml_vif->aml_hw;
    struct aml_tcp_sess_mgr *ack_mgr = &aml_hw->ack_mgr;

    atomic_set(&ack_mgr->enable, enable);
    ack_mgr->ack_winsize = min_size;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set tcp delay ack:ack_mgr->enable=%u,ack_mgr->ack_winsize is %dK\n", atomic_read(&ack_mgr->enable),ack_mgr->ack_winsize);
    return 0;
}

static int aml_set_max_drop_num(struct net_device *dev, int num)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw * aml_hw = aml_vif->aml_hw;
    struct aml_tcp_sess_mgr *ack_mgr = &aml_hw->ack_mgr;

    atomic_set(&ack_mgr->max_drop_cnt, num);
    atomic_set(&ack_mgr->dynamic_adjust, 0);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set tcp delay ack:ack_mgr->max_drop_cnt=%u,dynamic adjust=%d\n", atomic_read(&ack_mgr->max_drop_cnt), atomic_read(&ack_mgr->dynamic_adjust));
    return 0;
}

static int aml_set_max_timeout(struct net_device *dev, int time)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw * aml_hw = aml_vif->aml_hw;
    struct aml_tcp_sess_mgr *ack_mgr = &aml_hw->ack_mgr;

    atomic_set(&ack_mgr->max_timeout, time);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set tcp delay ack:ack_mgr->max_timeout=%u\n", atomic_read(&ack_mgr->max_timeout));
    return 0;
}

#ifdef CONFIG_AML_NAPI
static int aml_set_napi_enable(struct net_device *dev, int enable)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw * aml_hw = aml_vif->aml_hw;
    aml_hw->napi_enable = enable;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set napi_enable=%u\n", aml_hw->napi_enable);
    return 0;
}

static int aml_set_gro_enable(struct net_device *dev, int enable)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw * aml_hw = aml_vif->aml_hw;
    aml_hw->gro_enable = enable;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set gro_enable=%u\n", aml_hw->gro_enable);
    return 0;
}

static int aml_set_napi_num(struct net_device *dev, int num)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw * aml_hw = aml_vif->aml_hw;
    aml_hw->napi_pend_pkt_num = num;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set aml_hw->napi_pend_pkt_num=%u\n", aml_hw->napi_pend_pkt_num);
    return 0;
}
#endif

static int aml_set_txdesc_trigger_ths(struct net_device *dev, int cnt)
{
    g_txdesc_trigger.ths_enable = cnt;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "g_txdesc_trigger.ths_enable=%u\n", g_txdesc_trigger.ths_enable);
    return 0;
}

extern void extern_wifi_set_enable(int is_on);
static int aml_set_bus_timeout_test(struct net_device *dev, int enable)
{
#ifndef CONFIG_PT_MODE
    int reg = 0;
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    struct aml_plat *aml_plat = aml_hw->plat;

    if (aml_bus_type == PCIE_MODE) {
        return 0;
     }
    if (enable) {
#ifndef CONFIG_LINUXPC_VERSION
        extern_wifi_set_enable(0);
#endif
        reg = AML_REG_READ(aml_plat, AML_ADDR_SYSTEM, AGCCCCACAL0_ADDR_CT);
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "%s: ++++++++++++++++enable bus timeout for recovery test !!!\n", __func__);
    }
#endif
    return 0;
}
static int aml_send_twt_teardown(struct net_device *dev, int id)
{
    struct twt_teardown_req twt_teardown;
    struct twt_teardown_cfm twt_teardown_cfm;
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;

    twt_teardown.neg_type = 0; //Individual TWT
    twt_teardown.all_twt = 0;
    twt_teardown.vif_idx = aml_vif->vif_index;
    twt_teardown.id = id;

    AML_INFO("flow id:%d\n", twt_teardown.id);
    return _aml_send_twt_teardown(aml_hw, &twt_teardown, &twt_teardown_cfm);
}

int aml_set_macbypass(struct net_device *dev, int format_type, int bandwidth, int rate, int siso_or_mimo)
{
    struct aml_vif *aml_vif = netdev_priv(dev);

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set macbypass, format_type:%d, bandwidth:%d, rate:%d, siso_or_mimo:%d!\n",
           format_type, bandwidth, rate, siso_or_mimo);

    _aml_set_macbypass(aml_vif, format_type, bandwidth, rate, siso_or_mimo);

    return 0;
}

int aml_set_stop_macbypass(struct net_device *dev)
{
    struct aml_vif *aml_vif = netdev_priv(dev);

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "Stop macbypass!\n");

    _aml_set_stop_macbypass(aml_vif);

    return 0;
}

int aml_set_stbc(struct net_device *dev, int stbc_on)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;

    if (aml_hw->mod_params->stbc_on == stbc_on) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "stbc_on did not change, ignore\n");
        return 0;
    }

    aml_hw->mod_params->stbc_on = stbc_on;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set stbc_on:%d success\n", stbc_on);

    /* Set VHT capabilities */
    aml_set_vht_capa(aml_hw, aml_hw->wiphy);

    /* Set HE capabilities */
    aml_set_he_capa(aml_hw,  aml_hw->wiphy);

    /* Set HT capabilities */
    aml_set_ht_capa(aml_hw,  aml_hw->wiphy);

    return _aml_set_stbc(aml_hw, aml_vif->vif_index, stbc_on);
}

static int aml_get_stbc(struct net_device *dev)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "current stbc: %d\n", aml_hw->mod_params->stbc_on);
    return 0;
}

static int aml_dump_reg(struct net_device *dev, int addr, int size)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    struct aml_plat *aml_plat = aml_hw->plat;

    int len = 0, i = 0;
    char *la_buf = NULL;

    u8 *map_address = NULL;
    u8 *address = (u8 *)(unsigned long)addr;

    if (aml_bus_type == PCIE_MODE) {
        map_address = aml_pci_get_map_address(dev, addr);
        if (!map_address) {
            AML_PRINT(AML_DBG_MODULES_IWPRIV, "%s: map_address erro\n", __func__);
            return 0;
        }
    }

    la_buf = kmalloc(REG_DUMP_SIZE, GFP_ATOMIC);
    if (!la_buf) {
         AML_PRINT(AML_DBG_MODULES_IWPRIV, "%s: malloc buf erro\n", __func__);
         return 0;
    }

    memset(la_buf, 0, REG_DUMP_SIZE);
    len += scnprintf(&la_buf[len], (REG_DUMP_SIZE - len), "========dump range [0x%x ---- 0x%x], Size 0x%x========\n",
            address, address + size, size);

    if (aml_bus_type == PCIE_MODE) {
        for (i = 0; i < size / 4; i++) {
            len += scnprintf(&la_buf[len], (REG_DUMP_SIZE - len), "addr 0x%x ----- value 0x%x\n",
                address + i * 4, aml_pci_readl(map_address+ i*4));

            if ((REG_DUMP_SIZE - len) < 38) {
                aml_send_log_to_user(la_buf, len, AML_MEM_DUMP_UPLOAD);

                len = 0;
                memset(la_buf, 0, REG_DUMP_SIZE);
            }
        }
    } else {
        for (i = 0; i < size / 4; i++) {
            len += scnprintf(&la_buf[len], (REG_DUMP_SIZE - len), "addr 0x%x ----- value 0x%x\n",
                address + i * 4, AML_REG_READ(aml_plat, 0, (u64)(address + i*4)));

            if ((REG_DUMP_SIZE - len) < 38) {
                aml_send_log_to_user(la_buf, len, AML_MEM_DUMP_UPLOAD);

                len = 0;
                memset(la_buf, 0, REG_DUMP_SIZE);
            }
        }
    }
    if (len != 0) {
        aml_send_log_to_user(la_buf, len, AML_MEM_DUMP_UPLOAD);
    }

err:
    kfree(la_buf);
    return 0;
}

static int aml_emb_la_dump(struct net_device *dev)
{
    struct file *fp = NULL;
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    struct aml_plat *aml_plat = aml_hw->plat;

    int len = 0, i = 0;
    char *la_buf = NULL;
    u8 *map_address = NULL;
    map_address = aml_pci_get_map_address(dev, LA_MEMORY_BASE_ADDRESS);
    if (!map_address) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "%s: map_address erro\n", __func__);
        return 0;
    }

    la_buf = kmalloc(LA_BUF_SIZE, GFP_ATOMIC);
    if (!la_buf) {
         AML_PRINT(AML_DBG_MODULES_IWPRIV, "%s: malloc buf erro\n", __func__);
         return 0;
    }

    memset(la_buf, 0, LA_BUF_SIZE);

    if (aml_bus_type == PCIE_MODE) {
        for (i=0; i < 0x3fff; i+=2) {
            len += scnprintf(&la_buf[len], (LA_BUF_SIZE - len), "%08x%08x\n",
                aml_pci_readl(map_address+((1+i)*4)), aml_pci_readl(map_address+(i*4)));

            if ((LA_BUF_SIZE - len) < 20) {
                aml_send_log_to_user(la_buf, len, AML_LA_MACTRACE_UPLOAD);

                len = 0;
                memset(la_buf, 0, LA_BUF_SIZE);
            }
        }

        if (len != 0) {
             aml_send_log_to_user(la_buf, len, AML_LA_MACTRACE_UPLOAD);
        }
    } else {
         for (i=0; i < 0x3fff; i+=2) {
             len += scnprintf(&la_buf[len], (LA_BUF_SIZE - len), "%08x%08x\n",
                 AML_REG_READ(aml_plat, 0, LA_MEMORY_BASE_ADDRESS+((1+i)*4)),
                 AML_REG_READ(aml_plat, 0, LA_MEMORY_BASE_ADDRESS+(i*4)));

             if ((LA_BUF_SIZE - len) < 20) {
                 aml_send_log_to_user(la_buf, len, AML_LA_MACTRACE_UPLOAD);

                 len = 0;
                 memset(la_buf, 0, LA_BUF_SIZE);
             }
         }

         if (len != 0) {
             aml_send_log_to_user(la_buf, len, AML_LA_MACTRACE_UPLOAD);
         }
    }

err:
    kfree(la_buf);
    return 0;
}

int aml_set_pt_calibration(struct net_device *dev, int pt_cali_val)
{
    struct aml_vif *aml_vif = netdev_priv(dev);

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set pt calibration, pt calibration conf:%x\n", pt_cali_val);
    if (pt_cali_val & BIT(21))
        pt_mode = 1;

    _aml_set_pt_calibration(aml_vif, pt_cali_val);

    return 0;
}
int aml_get_all_efuse(struct net_device *dev,union iwreq_data *wrqu, char *extra)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    unsigned int reg_val = 0;
    unsigned int production_vendor_id = 0;
    unsigned int efuse_map_version = 0;
    U8 xosc_ctune = 0;
    U8 offset_power_wf0_2g_l = 0;
    U8 offset_power_wf0_2g_m = 0;
    U8 offset_power_wf0_2g_h = 0;
    U8 offset_power_wf0_5200 = 0;
    U8 offset_power_wf0_5300 = 0;
    U8 offset_power_wf0_5530 = 0;
    U8 offset_power_wf0_5660 = 0;
    U8 offset_power_wf0_5780 = 0;
    U8 offset_power_wf1_2g_l = 0;
    U8 offset_power_wf1_2g_m = 0;
    U8 offset_power_wf1_2g_h = 0;
    U8 offset_power_wf1_5200 = 0;
    U8 offset_power_wf1_5300 = 0;
    U8 offset_power_wf1_5530 = 0;
    U8 offset_power_wf1_5660 = 0;
    U8 offset_power_wf1_5780 = 0;
    unsigned int wifi_efuse_data_l = 0;
    unsigned int wifi_efuse_data_h = 0;
    unsigned int wifi_efuse_data = 0;
    unsigned int bt_efuse_data_l = 0;
    unsigned int bt_efuse_data_h = 0;
    unsigned int bt_efuse_data = 0;

    reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_00);
    production_vendor_id = reg_val;

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "production&vendor id:0x%x\n", production_vendor_id);

    reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_06);
    efuse_map_version = (reg_val >> 16) & 0x7F;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "efuse map version:0x%02x\n", efuse_map_version);

    reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_07);
    // xosc second times vld disable, read the value written for the first time
    if ((reg_val & 0x80000000) == 0) {
        reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_0F);
        xosc_ctune = (reg_val & 0xFF000000) >> 24;
        reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1A);
        offset_power_wf0_2g_l = (reg_val & 0x1F0000) >> 16;
        offset_power_wf0_2g_m = (reg_val & 0x1F000000) >> 24;
        reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1B);
        offset_power_wf0_2g_h = (reg_val & 0x1F);
        offset_power_wf0_5200 = (reg_val & 0x1F00) >> 8;
        offset_power_wf0_5300 = (reg_val & 0x1F0000) >> 16;
        offset_power_wf0_5530 = (reg_val & 0x1F000000) >> 24;
        reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1C);
        offset_power_wf0_5660 = (reg_val & 0x1F);
        offset_power_wf0_5780 = (reg_val & 0x1F00) >> 8;
        reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1E);
        offset_power_wf1_2g_l = (reg_val & 0x1F);
        offset_power_wf1_2g_m = (reg_val & 0x1F00) >> 8;
        offset_power_wf1_2g_h = (reg_val & 0x1F0000) >> 16;
        offset_power_wf1_5200 = (reg_val & 0x1F000000) >> 24;
        reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1F);
        offset_power_wf1_5300 = (reg_val & 0x1F);
        offset_power_wf1_5530 = (reg_val & 0x1F00) >> 8;
        offset_power_wf1_5660 = (reg_val & 0x1F0000) >> 16;
        offset_power_wf1_5780 = (reg_val & 0x1F000000) >> 24;

        AML_PRINT(AML_DBG_MODULES_IWPRIV, "xosc_ctune=0x%02x\n", xosc_ctune);
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "offset_power_wf0_2g_l=0x%02x,offset_power_wf0_2g_m=0x%02x,offset_power_wf0_2g_h=0x%02x\n",
               offset_power_wf0_2g_l, offset_power_wf0_2g_m, offset_power_wf0_2g_h);
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "offset_power_wf0_5200=0x%02x,offset_power_wf0_5300=0x%02x,offset_power_wf0_5530=0x%02x,offset_power_wf0_5660=0x%02x,offset_power_wf0_5780=0x%02x\n",
               offset_power_wf0_5200, offset_power_wf0_5300, offset_power_wf0_5530,offset_power_wf0_5660, offset_power_wf0_5780);
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "offset_power_wf1_2g_l=0x%02x,offset_power_wf1_2g_m=0x%02x,offset_power_wf1_2g_h=0x%02x\n",
               offset_power_wf1_2g_l, offset_power_wf1_2g_m, offset_power_wf1_2g_h);
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "offset_power_wf1_5200=0x%02x,offset_power_wf1_5300=0x%02x,offset_power_wf1_5530=0x%02x,offset_power_wf1_5660=0x%02x,offset_power_wf1_5780=0x%02x\n",
               offset_power_wf1_5200, offset_power_wf1_5300, offset_power_wf1_5530,offset_power_wf1_5660, offset_power_wf1_5780);
    } else {
        reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_0F);
        xosc_ctune = (reg_val & 0x00FF0000) >> 16;
        reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_15);
        offset_power_wf0_2g_l = (reg_val & 0x0000001F);
        offset_power_wf0_2g_m = (reg_val & 0x000003E0) >> 5;
        offset_power_wf0_2g_h = (reg_val & 0x00007C00) >> 10;
        offset_power_wf0_5200 = (reg_val & 0x001F0000) >> 16;
        offset_power_wf0_5300 = (reg_val & 0x03E00000) >> 21;
        offset_power_wf0_5530 = (reg_val & 0x7C000000) >> 26;
        reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_16);
        offset_power_wf0_5660 = (reg_val & 0x0000001F);
        offset_power_wf0_5780 = (reg_val & 0x000003E0) >> 5;
        offset_power_wf1_2g_l = (reg_val & 0x001F0000) >> 16;
        offset_power_wf1_2g_m = (reg_val & 0x03E00000) >> 21;
        offset_power_wf1_2g_h = (reg_val & 0x7C000000) >> 26;
        reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_17);
        offset_power_wf1_5200 = (reg_val & 0x0000001F);
        offset_power_wf1_5300 = (reg_val & 0x000003E0) >> 5;
        offset_power_wf1_5530 = (reg_val & 0x00007C00) >> 10;
        offset_power_wf1_5660 = (reg_val & 0x001F0000) >> 16;
        offset_power_wf1_5780 = (reg_val & 0x03E00000) >> 21;

        AML_PRINT(AML_DBG_MODULES_IWPRIV, "second xosc_ctune=0x%02x\n", xosc_ctune);
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "offset_power_wf0_2g_l=0x%02x,offset_power_wf0_2g_m=0x%02x,offset_power_wf0_2g_h=0x%02x\n",
               offset_power_wf0_2g_l, offset_power_wf0_2g_m, offset_power_wf0_2g_h);
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "offset_power_wf0_5200=0x%02x,offset_power_wf0_5300=0x%02x,offset_power_wf0_5530=0x%02x,offset_power_wf0_5660=0x%02x,offset_power_wf0_5780=0x%02x\n",
               offset_power_wf0_5200, offset_power_wf0_5300, offset_power_wf0_5530,offset_power_wf0_5660, offset_power_wf0_5780);
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "offset_power_wf1_2g_l=0x%02x,offset_power_wf1_2g_m=0x%02x,offset_power_wf1_2g_h=0x%02x\n",
               offset_power_wf1_2g_l, offset_power_wf1_2g_m, offset_power_wf1_2g_h);
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "offset_power_wf1_5200=0x%02x,offset_power_wf1_5300=0x%02x,offset_power_wf1_5530=0x%02x,offset_power_wf1_5660=0x%02x,offset_power_wf1_5780=0x%02x\n",
               offset_power_wf1_5200, offset_power_wf1_5300, offset_power_wf1_5530,offset_power_wf1_5660, offset_power_wf1_5780);
    }

    wifi_efuse_data = _aml_get_efuse(aml_vif, EFUSE_BASE_07);

    if (wifi_efuse_data & BIT17) {
        wifi_efuse_data_l = _aml_get_efuse(aml_vif, EFUSE_BASE_10);
        wifi_efuse_data_h = _aml_get_efuse(aml_vif, EFUSE_BASE_11);
    } else {
        wifi_efuse_data_l = _aml_get_efuse(aml_vif, EFUSE_BASE_01);
        wifi_efuse_data_h = _aml_get_efuse(aml_vif, EFUSE_BASE_02);
    }
    if (wifi_efuse_data_l != 0 || wifi_efuse_data_h != 0) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "efuse addr:%08x,%08x, wifi MAC addr is: %02x:%02x:%02x:%02x:%02x:%02x\n", EFUSE_BASE_01, EFUSE_BASE_02,
            (wifi_efuse_data_h & 0xff00) >> 8,wifi_efuse_data_h & 0x00ff, (wifi_efuse_data_l & 0xff000000) >> 24,
            (wifi_efuse_data_l & 0x00ff0000) >> 16,(wifi_efuse_data_l & 0xff00) >> 8,wifi_efuse_data_l & 0xff);
    }

    bt_efuse_data_l = _aml_get_efuse(aml_vif, EFUSE_BASE_02);
    bt_efuse_data_h = _aml_get_efuse(aml_vif, EFUSE_BASE_03);
    if ((((bt_efuse_data_l >> 16) & 0xffff) == 0) && (bt_efuse_data_h == 0)) {
        bt_efuse_data_l = _aml_get_efuse(aml_vif, EFUSE_BASE_11);
        bt_efuse_data_h = _aml_get_efuse(aml_vif, EFUSE_BASE_12);
    }
    if (bt_efuse_data_l != 0 || bt_efuse_data_h != 0) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "BT MAC addr is: %02x:%02x:%02x:%02x:%02x:%02x\n",
            (bt_efuse_data_h & 0xff000000) >> 24,(bt_efuse_data_h & 0x00ff0000) >> 16,
            (bt_efuse_data_h & 0xff00) >> 8, bt_efuse_data_h & 0xff,
            (bt_efuse_data_l & 0xff000000) >> 24, (bt_efuse_data_l & 0x00ff0000) >> 16);
    }
    wrqu->data.length = scnprintf(extra, IW_PRIV_SIZE_MASK, "production_vendor_id:0x%08x, efuse_map_version:0x%02x\n\
        xosc_ctune=0x%02x\n\
        offset_power_wf0_2g_l=0x%02x,offset_power_wf0_2g_m=0x%02x,offset_power_wf0_2g_h=0x%02x\n\
        offset_power_wf0_5200=0x%02x,offset_power_wf0_5300=0x%02x,offset_power_wf0_5530=0x%02x,offset_power_wf0_5660=0x%02x,offset_power_wf0_5780=0x%02x\n\
        offset_power_wf1_2g_l=0x%02x,offset_power_wf1_2g_m=0x%02x,offset_power_wf1_2g_h=0x%02x\n\
        offset_power_wf1_5200=0x%02x,offset_power_wf1_5300=0x%02x,offset_power_wf1_5530=0x%02x,offset_power_wf1_5660=0x%02x,offset_power_wf1_5780=0x%02x\n\
        wifi_mac=%02x:%02x:%02x:%02x:%02x:%02x\n\
        bt_mac=%02x:%02x:%02x:%02x:%02x:%02x\n",
        production_vendor_id, efuse_map_version, xosc_ctune, offset_power_wf0_2g_l, offset_power_wf0_2g_m, offset_power_wf0_2g_h,
        offset_power_wf0_5200, offset_power_wf0_5300, offset_power_wf0_5530,offset_power_wf0_5660, offset_power_wf0_5780,
        offset_power_wf1_2g_l, offset_power_wf1_2g_m, offset_power_wf1_2g_h,
        offset_power_wf1_5200, offset_power_wf1_5300, offset_power_wf1_5530,offset_power_wf1_5660, offset_power_wf1_5780,
        (wifi_efuse_data_h & 0xff00) >> 8,wifi_efuse_data_h & 0x00ff, (wifi_efuse_data_l & 0xff000000) >> 24,
        (wifi_efuse_data_l & 0x00ff0000) >> 16,(wifi_efuse_data_l & 0xff00) >> 8,wifi_efuse_data_l & 0xff,
        (bt_efuse_data_h & 0xff000000) >> 24, (bt_efuse_data_h & 0x00ff0000) >> 16, (bt_efuse_data_h & 0xff00) >> 8,
        bt_efuse_data_h & 0xff, (bt_efuse_data_l & 0xff000000) >> 24, (bt_efuse_data_l & 0x00ff0000) >> 16);
    wrqu->data.length++;
}


int aml_get_xosc_offset(struct net_device *dev,union iwreq_data *wrqu, char *extra)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    unsigned int reg_val = 0;
    U8 xosc_ctune = 0;
    U8 offset_power_wf0_2g_l = 0;
    U8 offset_power_wf0_2g_m = 0;
    U8 offset_power_wf0_2g_h = 0;
    U8 offset_power_wf0_5200 = 0;
    U8 offset_power_wf0_5300 = 0;
    U8 offset_power_wf0_5530 = 0;
    U8 offset_power_wf0_5660 = 0;
    U8 offset_power_wf0_5780 = 0;
    U8 offset_power_wf1_2g_l = 0;
    U8 offset_power_wf1_2g_m = 0;
    U8 offset_power_wf1_2g_h = 0;
    U8 offset_power_wf1_5200 = 0;
    U8 offset_power_wf1_5300 = 0;
    U8 offset_power_wf1_5530 = 0;
    U8 offset_power_wf1_5660 = 0;
    U8 offset_power_wf1_5780 = 0;

    reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_07);
    // xosc second times vld disable, read the value written for the first time
    if ((reg_val & 0x80000000) == 0) {
        reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_0F);
        xosc_ctune = (reg_val & 0xFF000000) >> 24;
        reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1A);
        offset_power_wf0_2g_l = (reg_val & 0x1F0000) >> 16;
        offset_power_wf0_2g_m = (reg_val & 0x1F000000) >> 24;
        reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1B);
        offset_power_wf0_2g_h = (reg_val & 0x1F);
        offset_power_wf0_5200 = (reg_val & 0x1F00) >> 8;
        offset_power_wf0_5300 = (reg_val & 0x1F0000) >> 16;
        offset_power_wf0_5530 = (reg_val & 0x1F000000) >> 24;
        reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1C);
        offset_power_wf0_5660 = (reg_val & 0x1F);
        offset_power_wf0_5780 = (reg_val & 0x1F00) >> 8;
        reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1E);
        offset_power_wf1_2g_l = (reg_val & 0x1F);
        offset_power_wf1_2g_m = (reg_val & 0x1F00) >> 8;
        offset_power_wf1_2g_h = (reg_val & 0x1F0000) >> 16;
        offset_power_wf1_5200 = (reg_val & 0x1F000000) >> 24;
        reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1F);
        offset_power_wf1_5300 = (reg_val & 0x1F);
        offset_power_wf1_5530 = (reg_val & 0x1F00) >> 8;
        offset_power_wf1_5660 = (reg_val & 0x1F0000) >> 16;
        offset_power_wf1_5780 = (reg_val & 0x1F000000) >> 24;

        AML_PRINT(AML_DBG_MODULES_IWPRIV, "xosc_ctune=0x%02x\n", xosc_ctune);
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "offset_power_wf0_2g_l=0x%02x,offset_power_wf0_2g_m=0x%02x,offset_power_wf0_2g_h=0x%02x\n",
               offset_power_wf0_2g_l, offset_power_wf0_2g_m, offset_power_wf0_2g_h);
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "offset_power_wf0_5200=0x%02x,offset_power_wf0_5300=0x%02x,offset_power_wf0_5530=0x%02x,offset_power_wf0_5660=0x%02x,offset_power_wf0_5780=0x%02x\n",
               offset_power_wf0_5200, offset_power_wf0_5300, offset_power_wf0_5530,offset_power_wf0_5660, offset_power_wf0_5780);
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "offset_power_wf1_2g_l=0x%02x,offset_power_wf1_2g_m=0x%02x,offset_power_wf1_2g_h=0x%02x\n",
               offset_power_wf1_2g_l, offset_power_wf1_2g_m, offset_power_wf1_2g_h);
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "offset_power_wf1_5200=0x%02x,offset_power_wf1_5300=0x%02x,offset_power_wf1_5530=0x%02x,offset_power_wf1_5660=0x%02x,offset_power_wf1_5780=0x%02x\n",
               offset_power_wf1_5200, offset_power_wf1_5300, offset_power_wf1_5530,offset_power_wf1_5660, offset_power_wf1_5780);
    } else {
        reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_0F);
        xosc_ctune = (reg_val & 0x00FF0000) >> 16;
        reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_15);
        offset_power_wf0_2g_l = (reg_val & 0x1F);
        offset_power_wf0_2g_m = (reg_val >> 5) & 0x1F;
        offset_power_wf0_2g_h = (reg_val >> 10) & 0x1F;
        offset_power_wf0_5200 = (reg_val >> 16) & 0x1F;
        offset_power_wf0_5300 = (reg_val >> 21) & 0x1F;
        offset_power_wf0_5530 = (reg_val >> 26) & 0x1F;
        reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_16);
        offset_power_wf0_5660 = (reg_val & 0x1F);
        offset_power_wf0_5780 = (reg_val >> 5) & 0x1F;
        offset_power_wf1_2g_l = (reg_val >> 16) & 0x1F;
        offset_power_wf1_2g_m = (reg_val >> 21) & 0x1F;
        offset_power_wf1_2g_h = (reg_val >> 26)& 0x1F;
        reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_17);
        offset_power_wf1_5200 = (reg_val & 0x1F);
        offset_power_wf1_5300 = (reg_val >> 5) & 0x1F;
        offset_power_wf1_5530 = (reg_val >> 10) & 0x1F;
        offset_power_wf1_5660 = (reg_val >> 16) & 0x1F;
        offset_power_wf1_5780 = (reg_val >> 21) & 0x1F;

        AML_PRINT(AML_DBG_MODULES_IWPRIV, "second xosc_ctune=0x%02x\n", xosc_ctune);
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "offset_power_wf0_2g_l=0x%02x,offset_power_wf0_2g_m=0x%02x,offset_power_wf0_2g_h=0x%02x\n",
               offset_power_wf0_2g_l, offset_power_wf0_2g_m, offset_power_wf0_2g_h);
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "offset_power_wf0_5200=0x%02x,offset_power_wf0_5300=0x%02x,offset_power_wf0_5530=0x%02x,offset_power_wf0_5660=0x%02x,offset_power_wf0_5780=0x%02x\n",
               offset_power_wf0_5200, offset_power_wf0_5300, offset_power_wf0_5530,offset_power_wf0_5660, offset_power_wf0_5780);
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "offset_power_wf1_2g_l=0x%02x,offset_power_wf1_2g_m=0x%02x,offset_power_wf1_2g_h=0x%02x\n",
               offset_power_wf1_2g_l, offset_power_wf1_2g_m, offset_power_wf1_2g_h);
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "offset_power_wf1_5200=0x%02x,offset_power_wf1_5300=0x%02x,offset_power_wf1_5530=0x%02x,offset_power_wf1_5660=0x%02x,offset_power_wf1_5780=0x%02x\n",
               offset_power_wf1_5200, offset_power_wf1_5300, offset_power_wf1_5530,offset_power_wf1_5660, offset_power_wf1_5780);
    }

    wrqu->data.length = scnprintf(extra, IW_PRIV_SIZE_MASK, "&xosc_ctune=0x%02x\n\
        offset_power_wf0_2g_l=0x%02x,offset_power_wf0_2g_m=0x%02x,offset_power_wf0_2g_h=0x%02x\n\
        offset_power_wf0_5200=0x%02x,offset_power_wf0_5300=0x%02x,offset_power_wf0_5530=0x%02x,offset_power_wf0_5660=0x%02x,offset_power_wf0_5780=0x%02x\n\
        offset_power_wf1_2g_l=0x%02x,offset_power_wf1_2g_m=0x%02x,offset_power_wf1_2g_h=0x%02x\n\
        offset_power_wf1_5200=0x%02x,offset_power_wf1_5300=0x%02x,offset_power_wf1_5530=0x%02x,offset_power_wf1_5660=0x%02x,offset_power_wf1_5780=0x%02x\n",
        xosc_ctune, offset_power_wf0_2g_l, offset_power_wf0_2g_m, offset_power_wf0_2g_h,
        offset_power_wf0_5200, offset_power_wf0_5300, offset_power_wf0_5530,offset_power_wf0_5660, offset_power_wf0_5780,
        offset_power_wf1_2g_l, offset_power_wf1_2g_m, offset_power_wf1_2g_h,
        offset_power_wf1_5200, offset_power_wf1_5300, offset_power_wf1_5530,offset_power_wf1_5660, offset_power_wf1_5780);
    wrqu->data.length++;
    return 0;
}
int aml_set_rx_start(struct net_device *dev)
{
    aml_set_reg(dev, 0X60C0600C, 0x00000001);
    aml_set_reg(dev, 0X60C06000, 0x00000117);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "PT Rx Start\n");

    return 0;
}

int aml_set_rx_end(struct net_device *dev,union iwreq_data *wrqu, char *extra)
{
    int fcs_ok = 0;
    int fcs_err = 0;
    int fcs_rx_end = 0;
    int rx_err = 0;

    fcs_ok = aml_get_reg_2(dev, 0x60c06080, wrqu, extra);
    fcs_err = aml_get_reg_2(dev, 0x60c06084, wrqu, extra);
    fcs_rx_end = aml_get_reg_2(dev, 0x60c06088, wrqu, extra);
    rx_err = aml_get_reg_2(dev, 0x60c0608c, wrqu, extra);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "PT Rx result:fcs_ok=%d, fcs_err=%d, fcs_rx_end=%d, rx_err=%d\n", fcs_ok, fcs_err, fcs_rx_end, rx_err);

    wrqu->data.length = scnprintf(extra, IW_PRIV_SIZE_MASK, "fcs_ok=%d, fcs_err=%d, fcs_rx_end=%d, rx_err=%d\n", fcs_ok, fcs_err, fcs_rx_end, rx_err);
    wrqu->data.length++;
    aml_set_reg(dev, 0x60c0b500, 0x00041000);
    //aml_set_reg(dev, 0x00f0007c, 0x2000d110);  //bt clock enable
    return 0;
}

int aml_set_rx(struct net_device *dev, int antenna, int channel)
{
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set antenna :%x\n", antenna);
    aml_set_reg(dev, 0x00a0b1b8, 0xc0003000);
    aml_set_reg(dev, 0x00f00078, 0x2000c1e0);  //wifi clock enable

    switch (antenna) {
        case 1: //wf0 siso
            aml_set_reg(dev, 0x60c0b004, 0x00000001);
            if (channel <= 14) {
                aml_rf_reg_write(dev, 0x80000008, 0x01393917); //2G rx
                aml_rf_reg_write(dev, 0x80001008, 0x01393914); //2g sleep
                aml_set_reg(dev, 0x60c0b500, 0x00041000); //11b wf0 mode
                aml_set_reg(dev, 0x60c0b390, 0x00010003);
            } else {
                aml_rf_reg_write(dev, 0x80000008, 0x00393913); //5g rx
                aml_rf_reg_write(dev, 0x80001008, 0x00393911); //5G sx
                aml_set_reg(dev, 0x60c0b390, 0x00010103);
            }
            break;
        case 2: //wf1 siso
            aml_set_reg(dev, 0x60c0b004, 0x00000002);
            if (channel <= 14) {
                aml_rf_reg_write(dev, 0x80000008, 0x01393915); //2g sx
                aml_rf_reg_write(dev, 0x80001008, 0x01393917); //2G rx
                aml_set_reg(dev, 0x60c0b500, 0x00041030); //11b wf1 mode
                aml_set_reg(dev, 0x60c0b390, 0x00010003);
            } else {
                aml_rf_reg_write(dev, 0x80000008, 0x00393910); //5g sleep
                aml_rf_reg_write(dev, 0x80001008, 0x00393913); //5g rx
                aml_set_reg(dev, 0x60c0b390, 0x00010103);
            }
            break;
        case 3: //mimo
            if (channel <= 14) {
                aml_rf_reg_write(dev, 0x80000008, 0x01393900); //2g auto
                aml_rf_reg_write(dev, 0x80001008, 0x01393900); //2g auto
                aml_set_reg(dev, 0x60c0b500, 0x00041000);  //11b wf0 mode
                aml_set_reg(dev, 0x60c0b390, 0x00010003);
            } else {
                aml_rf_reg_write(dev, 0x80000008, 0x00393900); //5g auto
                aml_rf_reg_write(dev, 0x80001008, 0x00393900); //5g auto
                aml_set_reg(dev, 0x60c0b390, 0x00010103);
            }
            break;
        default:
            AML_PRINT(AML_DBG_MODULES_IWPRIV, "set antenna error :%x\n", antenna);
            break;
    }
    aml_set_reg(dev, 0x00f00078, 0x0000c1e0);   //wifi clock auto
    //aml_set_reg(dev, 0x00f0007c, 0x7800d110);  //bt clock disable
    return 0;
}

int aml_set_2G_dc_tone(struct net_device *dev, int wf_mode)
{
    aml_set_reg(dev, 0x00a0e018, 0x01110f00);
    aml_set_reg(dev, 0x00a0e010, 0x00002110);
    aml_set_reg(dev, 0x00a0e010, 0x00003110);
    switch (wf_mode) {
        case 1: //wf0 siso dc
            aml_set_reg(dev, 0x00a0e408, 0xa8200000);
            aml_set_reg(dev, 0x00a0e40c, 0x88400000);
            aml_set_reg(dev, 0x00a0e410, 0x00000002);
            aml_set_reg(dev, 0x00a0e008, 0x11111111);
            break;
        case 2: //wf1 siso dc
            aml_set_reg(dev, 0x00a0f018, 0x01110f00);
            aml_set_reg(dev, 0x00a0f010, 0x00002110);
            aml_set_reg(dev, 0x00a0f010, 0x00003110);
            aml_set_reg(dev, 0x00a0f408, 0xa8200000);
            aml_set_reg(dev, 0x00a0f40c, 0x88400000);
            aml_set_reg(dev, 0x00a0f410, 0x00000002);
            aml_set_reg(dev, 0x00a0f008, 0x11111111);
            break;
        case 3: //wf0 siso tone
            aml_set_reg(dev, 0x00a0e408, 0xa8200000);
            aml_set_reg(dev, 0x00a0e40c, 0x88400000);
            aml_set_reg(dev, 0x00a0e410, 0x00000001);
            aml_set_reg(dev, 0x00a0e008, 0x11111111);
            break;
        case 4: //wf1 siso tone
            aml_set_reg(dev, 0x00a0f018, 0x01110f00);
            aml_set_reg(dev, 0x00a0f010, 0x00002110);
            aml_set_reg(dev, 0x00a0f010, 0x00003110);
            aml_set_reg(dev, 0x00a0f408, 0xa8200000);
            aml_set_reg(dev, 0x00a0f40c, 0x88400000);
            aml_set_reg(dev, 0x00a0f410, 0x00000001);
            aml_set_reg(dev, 0x00a0f008, 0x11111111);
            break;
        default:
            AML_PRINT(AML_DBG_MODULES_IWPRIV, "set wf mode error :%x\n", wf_mode);
            break;
    }

    return 0;
}

int aml_set_5G_dc_tone(struct net_device *dev, int wf_mode)
{
    aml_set_reg(dev, 0x00a0f018, 0x01110f00);
    aml_set_reg(dev, 0x00a0f010, 0x00002110);
    aml_set_reg(dev, 0x00a0f010, 0x00003110);
    switch (wf_mode) {
        case 1: //wf0 siso dc
            aml_set_reg(dev, 0x00a0e018, 0x01110f00);
            aml_set_reg(dev, 0x00a0e010, 0x00002110);
            aml_set_reg(dev, 0x00a0e010, 0x00003110);
            aml_set_reg(dev, 0x00a0e408, 0xa8200000);
            aml_set_reg(dev, 0x00a0e40c, 0x88400000);
            aml_set_reg(dev, 0x00a0e410, 0x00000002);
            aml_set_reg(dev, 0x00a0e008, 0x11111111);
            break;
        case 2: //wf1 siso dc
            aml_set_reg(dev, 0x00a0f408, 0xa8200000);
            aml_set_reg(dev, 0x00a0f40c, 0x88400000);
            aml_set_reg(dev, 0x00a0f410, 0x00000002);
            aml_set_reg(dev, 0x00a0f008, 0x11111111);
            break;
        case 3: //wf0 siso tone
            aml_set_reg(dev, 0x00a0e018, 0x01110f00);
            aml_set_reg(dev, 0x00a0e010, 0x00002110);
            aml_set_reg(dev, 0x00a0e010, 0x00003110);
            aml_set_reg(dev, 0x00a0e408, 0xa8200000);
            aml_set_reg(dev, 0x00a0e40c, 0x88400000);
            aml_set_reg(dev, 0x00a0e410, 0x00000001);
            aml_set_reg(dev, 0x00a0e008, 0x11111111);
            break;
        case 4: //wf1 siso tone
            aml_set_reg(dev, 0x00a0f408, 0xa8200000);
            aml_set_reg(dev, 0x00a0f40c, 0x88400000);
            aml_set_reg(dev, 0x00a0f410, 0x00000001);
            aml_set_reg(dev, 0x00a0f008, 0x11111111);
            break;
        default:
            AML_PRINT(AML_DBG_MODULES_IWPRIV, "set wf mode error :%x\n", wf_mode);
            break;
    }


    return 0;
}

int aml_set_tone(struct net_device *dev, int signal, int wf_mode)
{
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set %d G, signal_mode %d\n", signal, wf_mode);

    switch (signal) {
        case 2: //2.4 G
            aml_set_2G_dc_tone(dev, wf_mode);
            break;
        case 5: //5G
            aml_set_5G_dc_tone(dev, wf_mode);
            break;
        default:
            AML_PRINT(AML_DBG_MODULES_IWPRIV, "set 2G/5G error :%x\n", signal);
            break;
    }

    return 0;
}

int aml_stop_dc_tone(struct net_device *dev)
{
    aml_set_reg(dev, 0x00a0e408, 0x88200000);
    aml_set_reg(dev, 0x00a0e40c, 0x88200000);
    aml_set_reg(dev, 0x00a0e410, 0x00000000);
    aml_set_reg(dev, 0x00a0e008, 0x00000000);
    aml_set_reg(dev, 0x00a0f408, 0x88200000);
    aml_set_reg(dev, 0x00a0f40c, 0x88200000);
    aml_set_reg(dev, 0x00a0f410, 0x00000000);
    aml_set_reg(dev, 0x00a0f008, 0x00000000);
}

static unsigned int tx_path = 0;
static unsigned int tx_channel = 0;
static unsigned int tx_mode = 0;
static unsigned int tx_bw = 0;
static unsigned int tx_len = 0;
static unsigned int tx_len1 = 0;
static unsigned int tx_len2 = 0;
static unsigned int tx_pwr = 0;
static unsigned int tx_start = 0;
static unsigned int tx_rate = 0;
static unsigned int tx_param = 0;
static unsigned int tx_param1 = 0;

int aml_11b_siso_wf0_tx(struct net_device *dev, U8 rate, U8 tx_pwr, U8 length, U8 length1, U8 length2)
{
    if (rate == 1) {
        rate = 0x0;
    } else if (rate == 2) {
        rate = 0x1;
    } else if (rate == 5) {
        rate = 0x2;
    } else if (rate == 11) {
        rate = 0x3;
    } else {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "11b_siso_wf0_tx rate error :%x\n", rate);
        return -1;
    }
    if (length1 > 0xff) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "11b_siso_wf0_tx length1 error :%x\n", length1);
        return -1;
    }
    if (length > 0xf) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "11b_siso_wf0_tx length error :%x\n", length);
        return -1;
    }
    aml_set_reg(dev, 0x60c00840, 0x80010001);
    aml_set_reg(dev, 0x00a0d084, 0x00020001);
    aml_set_reg(dev, 0x00a0d090, 0x4f210033);
    aml_set_reg(dev, 0x60805010, 0xb9d70242);
    aml_set_reg(dev, 0x60805014, 0x0000013f);
    aml_set_reg(dev, 0x60805008, 0x00001100);
    aml_set_reg(dev, 0x60c0b004, 0x00000001);
    aml_set_reg(dev, 0x00a0b00c, 0x00000011);
    aml_set_reg(dev, 0x60c0b390, 0x00010101);
    aml_set_reg(dev, 0x00a0b1b8, 0xc0003000);
    aml_set_reg(dev, 0x60c00800, 0x00000110);
    aml_set_reg(dev, 0x60c0b100, 0x00303030);
    aml_set_reg(dev, 0x60c06000, 0x00000000);
    aml_set_reg(dev, 0x60c06004, 0x00020000);
    aml_set_reg(dev, 0x60c06008, 0x00000012);
    aml_set_reg(dev, 0x60c0600c, 0x00000001);
    aml_set_reg(dev, 0x60c06048, 0x00010000);
    aml_set_reg(dev, 0x60c06200, 0x00000080);
    aml_set_reg(dev, 0x60c06204, 0x00000001);
    aml_set_reg(dev, 0x60c06208, 0x00000000 | (tx_pwr + 9));
    aml_set_reg(dev, 0x60c0620c, 0x00000000);
    aml_set_reg(dev, 0x60c06210, 0x00000000 | length1);
    aml_set_reg(dev, 0x60c06214, 0x00000000 | (rate << 4) | (length & 0x0f));
    aml_set_reg(dev, 0x60c06218, 0x00000000);
    aml_set_reg(dev, 0x60c0621c, 0x00000000);
    aml_set_reg(dev, 0x60c06220, 0x00000000);
    aml_set_reg(dev, 0x60c06224, 0x00000000);
    aml_set_reg(dev, 0x60c06228, 0x00000000);
    aml_set_reg(dev, 0x60c0622c, 0x00000001);
    aml_set_reg(dev, 0x60c06230, 0x00000000);
    aml_set_reg(dev, 0x60c06234, 0x00000000);
    aml_set_reg(dev, 0x60c06238, 0x00000080);
    aml_set_reg(dev, 0x60c0623c, 0x00000000);
    aml_set_reg(dev, 0x60c06240, 0x00000000);
    aml_set_reg(dev, 0x60c06010, 0x00000000);
    aml_set_reg(dev, 0x60c06000, 0x00000317);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "11b_wf0_tx:rate = %d,length=0x%x, tx_pwr=%d\n", rate, (tx_param1 >> 8), tx_pwr);
    return 0;
}

int aml_11b_siso_wf1_tx(struct net_device *dev, U8 rate, U8 tx_pwr, U8 length, U8 length1, U8 length2)
{
    if (rate == 1) {
        rate = 0x0;
    } else if (rate == 2) {
        rate = 0x1;
    } else if (rate == 5) {
        rate = 0x2;
    } else if (rate == 11) {
        rate = 0x3;
    } else {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "11b_siso_wf1_tx rate error :%x\n", rate);
        return -1;
    }
    if (length1 > 0xff) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "11b_siso_wf0_tx length1 error :%x\n", length1);
        return -1;
    }
    if (length > 0xf) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "11b_siso_wf0_tx length error :%x\n", length);
        return -1;
    }
    aml_set_reg(dev, 0x60c00840, 0x80010001);
    aml_set_reg(dev, 0x00a0d084, 0x00020001);
    aml_set_reg(dev, 0x00a0d090, 0x4f210033);
    aml_set_reg(dev, 0x60805010, 0xb9d70242);
    aml_set_reg(dev, 0x60805014, 0x0000013f);
    aml_set_reg(dev, 0x60805008, 0x00001100);
    aml_set_reg(dev, 0x60c0b004, 0x00000001);
    aml_set_reg(dev, 0x00a0b00c, 0x00000012);
    aml_set_reg(dev, 0x60c0b390, 0x00010102);
    aml_set_reg(dev, 0x00a0b1b8, 0xc0003100);
    aml_set_reg(dev, 0x60c00800, 0x00000110);
    aml_set_reg(dev, 0x60c0b100, 0x00303030);
    aml_set_reg(dev, 0x60c06000, 0x00000000);
    aml_set_reg(dev, 0x60c06004, 0x00020000);
    aml_set_reg(dev, 0x60c06008, 0x00000012);
    aml_set_reg(dev, 0x60c0600c, 0x00000001);
    aml_set_reg(dev, 0x60c06048, 0x00010000);
    aml_set_reg(dev, 0x60c06200, 0x00000080);
    aml_set_reg(dev, 0x60c06204, 0x00000001);
    aml_set_reg(dev, 0x60c06208, 0x00000000 | (tx_pwr + 9));
    aml_set_reg(dev, 0x60c0620c, 0x00000000);
    aml_set_reg(dev, 0x60c06210, 0x00000000 | length1);
    aml_set_reg(dev, 0x60c06214, 0x00000000 | (rate << 4) | (length & 0x0f));
    aml_set_reg(dev, 0x60c06218, 0x00000000);
    aml_set_reg(dev, 0x60c0621c, 0x00000000);
    aml_set_reg(dev, 0x60c06220, 0x00000000);
    aml_set_reg(dev, 0x60c06224, 0x00000000);
    aml_set_reg(dev, 0x60c06228, 0x00000000);
    aml_set_reg(dev, 0x60c0622c, 0x00000001);
    aml_set_reg(dev, 0x60c06230, 0x00000000);
    aml_set_reg(dev, 0x60c06234, 0x00000000);
    aml_set_reg(dev, 0x60c06238, 0x00000080);
    aml_set_reg(dev, 0x60c0623c, 0x00000000);
    aml_set_reg(dev, 0x60c06240, 0x00000000);
    aml_set_reg(dev, 0x60c06010, 0x00000000);
    aml_set_reg(dev, 0x60c06000, 0x00000317);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "11b_wf1_tx:rate = %d, length=0x%x, tx_pwr=%d\n", rate, (tx_param1 >> 8), tx_pwr);
    return 0;
}

int aml_11ag_siso_wf0_tx(struct net_device *dev, U8 rate, U8 tx_pwr, U8 length, U8 length1, U8 length2)
{
    if (rate == 6) {//6M
        rate = 0xb;
    } else if (rate == 9) {//9M
        rate = 0xf;
    } else if (rate == 12) {//12M
        rate = 0xa;
    } else if (rate == 18) {//18M
        rate = 0xe;
    } else if (rate == 24) {//24M
        rate = 0x9;
    } else if (rate == 36) {//36M
        rate = 0xd;
    } else if (rate == 48) {//48M
        rate = 0x8;
    } else if (rate == 54) {//54M
        rate = 0xc;
    } else {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "11ag_siso_wf0_tx rate error :%x\n", rate);
        return -1;
    }
    if (length1 > 0xff) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "11b_siso_wf0_tx length1 error :%x\n", length1);
        return -1;
    }
    if (length > 0xff) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "11b_siso_wf0_tx length error :%x\n", length);
        return -1;
    }
    aml_set_reg(dev, 0x60c00840, 0x80010001);
    aml_set_reg(dev, 0x00a0d084, 0x00020001);
    aml_set_reg(dev, 0x00a0d090, 0x4f210033);
    aml_set_reg(dev, 0x60805010, 0xb9d70242);
    aml_set_reg(dev, 0x60805014, 0x0000013f);
    aml_set_reg(dev, 0x60805008, 0x00001100);
    aml_set_reg(dev, 0x60c0b004, 0x00000001);
    aml_set_reg(dev, 0x00a0b00c, 0x00000011);
    aml_set_reg(dev, 0x60c0b390, 0x00010101);
    aml_set_reg(dev, 0x00a0b1b8, 0xc0003000);
    aml_set_reg(dev, 0x60c00800, 0x00000110);
    aml_set_reg(dev, 0x60c0088c, 0x00005050);
    aml_set_reg(dev, 0x60c0b100, 0x00303030);
    aml_set_reg(dev, 0x60c06000, 0x00000000);
    aml_set_reg(dev, 0x60c06004, 0x00020000);
    aml_set_reg(dev, 0x60c06008, 0x00000012);
    aml_set_reg(dev, 0x60c0600c, 0x00000001);
    aml_set_reg(dev, 0x60c06048, 0x00010000);
    aml_set_reg(dev, 0x60c06200, 0x00000080);
    aml_set_reg(dev, 0x60c06204, 0x00000001);
    aml_set_reg(dev, 0x60c06208, 0x00000000 | tx_pwr);
    aml_set_reg(dev, 0x60c0620c, 0x00000000);
    aml_set_reg(dev, 0x60c06210, 0x00000000 | length1);
    aml_set_reg(dev, 0x60c06214, 0x00000000 | (rate << 4) | length);
    aml_set_reg(dev, 0x60c06218, 0x00000000);
    aml_set_reg(dev, 0x60c0621c, 0x00000000);
    aml_set_reg(dev, 0x60c06220, 0x00000000);
    aml_set_reg(dev, 0x60c06224, 0x00000000);
    aml_set_reg(dev, 0x60c06228, 0x00000000);
    aml_set_reg(dev, 0x60c0622c, 0x00000001);
    aml_set_reg(dev, 0x60c06230, 0x00000000);
    aml_set_reg(dev, 0x60c06234, 0x00000010);
    aml_set_reg(dev, 0x60c06238, 0x00000080);
    aml_set_reg(dev, 0x60c0623c, 0x00000000);
    aml_set_reg(dev, 0x60c06240, 0x00000000);
    aml_set_reg(dev, 0x60c06010, 0x00000000);
    aml_set_reg(dev, 0x60c06000, 0x00000317);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "11ag_tx:rate = %d, length=0x%x, tx_pwr=%d\n", rate, (tx_param1 >> 8), tx_pwr);
    return 0;
}

int aml_11ag_siso_wf1_tx(struct net_device *dev, U8 rate, U8 tx_pwr, U8 length, U8 length1, U8 length2)
{
    if (rate == 6) {
        rate = 0xb;
    } else if (rate == 9) {
        rate = 0xf;
    } else if (rate == 12) {
        rate = 0xa;
    } else if (rate == 18) {
        rate = 0xe;
    } else if (rate == 24) {
        rate = 0x9;
    } else if (rate == 36) {
        rate = 0xd;
    } else if (rate == 48) {
        rate = 0x8;
    } else if (rate == 54) {
        rate = 0xc;
    } else {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "11ag_siso_wf1_tx rate error :%x\n", rate);
        return -1;
    }
    if (length1 > 0xff) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "11b_siso_wf0_tx length1 error :%x\n", length1);
        return -1;
    }
    if (length > 0xff) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "11b_siso_wf0_tx length error :%x\n", length);
        return -1;
    }
    aml_set_reg(dev, 0x60c00840, 0x80010001);
    aml_set_reg(dev, 0x00a0d084, 0x00020001);
    aml_set_reg(dev, 0x00a0d090, 0x4f210033);
    aml_set_reg(dev, 0x60805010, 0xb9d70242);
    aml_set_reg(dev, 0x60805014, 0x0000013f);
    aml_set_reg(dev, 0x60805008, 0x00001100);
    aml_set_reg(dev, 0x60c0b004, 0x00000001);
    aml_set_reg(dev, 0x00a0b00c, 0x00000012);
    aml_set_reg(dev, 0x60c0b390, 0x00010102);
    aml_set_reg(dev, 0x00a0b1b8, 0xc0003100);
    aml_set_reg(dev, 0x60c00800, 0x00000110);
    aml_set_reg(dev, 0x60c0088c, 0x00005050);
    aml_set_reg(dev, 0x60c0b100, 0x00303030);
    aml_set_reg(dev, 0x60c06000, 0x00000000);
    aml_set_reg(dev, 0x60c06004, 0x00020000);
    aml_set_reg(dev, 0x60c06008, 0x00000012);
    aml_set_reg(dev, 0x60c0600c, 0x00000001);
    aml_set_reg(dev, 0x60c06048, 0x00010000);
    aml_set_reg(dev, 0x60c06200, 0x00000080);
    aml_set_reg(dev, 0x60c06204, 0x00000001);
    aml_set_reg(dev, 0x60c06208, 0x00000000 | tx_pwr);
    aml_set_reg(dev, 0x60c0620c, 0x00000000);
    aml_set_reg(dev, 0x60c06210, 0x00000000 | length1);
    aml_set_reg(dev, 0x60c06214, 0x00000000 | (rate << 4) | length);
    aml_set_reg(dev, 0x60c06218, 0x00000000);
    aml_set_reg(dev, 0x60c0621c, 0x00000000);
    aml_set_reg(dev, 0x60c06220, 0x00000000);
    aml_set_reg(dev, 0x60c06224, 0x00000000);
    aml_set_reg(dev, 0x60c06228, 0x00000000);
    aml_set_reg(dev, 0x60c0622c, 0x00000001);
    aml_set_reg(dev, 0x60c06230, 0x00000000);
    aml_set_reg(dev, 0x60c06234, 0x00000010);
    aml_set_reg(dev, 0x60c06238, 0x00000080);
    aml_set_reg(dev, 0x60c0623c, 0x00000000);
    aml_set_reg(dev, 0x60c06240, 0x00000000);
    aml_set_reg(dev, 0x60c06010, 0x00000000);
    aml_set_reg(dev, 0x60c06000, 0x00000317);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "11ag_tx:rate = %d, length=0x%x, tx_pwr=%d\n", rate, (tx_param1 >> 8), tx_pwr);
    return 0;
}

int aml_ht_siso_wf0_tx(struct net_device *dev, U8 bw, U8 rate, U8 tx_pwr, U8 length, U8 length1, U8 length2)
{
    aml_set_reg(dev, 0x60c00840, 0x80010001);
    aml_set_reg(dev, 0x00a0d084, 0x00020001);
    aml_set_reg(dev, 0x00a0d090, 0x4f210033);
    aml_set_reg(dev, 0x60805010, 0xb9d70242);
    aml_set_reg(dev, 0x60805014, 0x0000013f);
    if (bw == 0x0) {//bw 20M
        aml_set_reg(dev, 0x60805008, 0x00001100);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60805008, 0x00001150);
    }
    aml_set_reg(dev, 0x60c0b004, 0x00000001);
    aml_set_reg(dev, 0x00a0b00c, 0x00000011);
    aml_set_reg(dev, 0x60c0b390, 0x00010101);
    aml_set_reg(dev, 0x00a0b1b8, 0xc0003000);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60c00800, 0x00000110);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60c00800, 0x00000111);
    }
    aml_set_reg(dev, 0x60c0088c, 0x00005050);
    aml_set_reg(dev, 0x60c0b100, 0x00303030);
    aml_set_reg(dev, 0x60c06000, 0x00000000);
    aml_set_reg(dev, 0x60c06004, 0x00020000);
    aml_set_reg(dev, 0x60c06008, 0x00000012);
    aml_set_reg(dev, 0x60c0600c, 0x00000001);
    aml_set_reg(dev, 0x60c06048, 0x00010000);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60c06200, 0x00000002);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60c06200, 0x00000012);
    }
    aml_set_reg(dev, 0x60c06204, 0x00000001);
    aml_set_reg(dev, 0x60c06208, 0x00000000 | tx_pwr);
    aml_set_reg(dev, 0x60c0620c, 0x00000000);
    aml_set_reg(dev, 0x60c06210, 0x000000fc);
    aml_set_reg(dev, 0x60c06214, 0x00000001);
    aml_set_reg(dev, 0x60c06218, 0x00000000);
    aml_set_reg(dev, 0x60c0621c, 0x00000000);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60c06220, 0x00000008);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60c06220, 0x0000000c);
    }
    aml_set_reg(dev, 0x60c06224, 0x00000000);
    aml_set_reg(dev, 0x60c06228, 0x00000080 | rate);
    aml_set_reg(dev, 0x60c0622c, 0x00000000 | length1);
    aml_set_reg(dev, 0x60c06230, 0x00000000 | length);
    aml_set_reg(dev, 0x60c06234, 0x00000000);
    aml_set_reg(dev, 0x60c06238, 0x00000080);
    aml_set_reg(dev, 0x60c0623c, 0x00000000);
    aml_set_reg(dev, 0x60c06240, 0x00000000);
    aml_set_reg(dev, 0x60c06010, 0x00000000);
    aml_set_reg(dev, 0x60c06000, 0x00000317);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "2g_5g_siso_ht_tx:bw=%d, rate = %d, length=0x%x, tx_pwr=%d\n",bw, rate, (tx_param1 >> 8), tx_pwr);
    return 0;
}

//set ht 20/40m rate bw tx_pwr length
int aml_ht_mimo_tx(struct net_device *dev, U8 bw, U8 rate, U8 tx_pwr, U8 length, U8 length1, U8 length2)
{
    aml_set_reg(dev, 0x60c00840, 0x80010001);
    aml_set_reg(dev, 0x00a0d084, 0x00020001);
    aml_set_reg(dev, 0x00a0d090, 0x4f210033);
    aml_set_reg(dev, 0x60805010, 0xb9d70242);
    aml_set_reg(dev, 0x60805014, 0x0000013f);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60805008, 0x00001100);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60805008, 0x00001150);
    }
    aml_set_reg(dev, 0x60c0b004, 0x00000003);
    aml_set_reg(dev, 0x00a0b00c, 0x00000033);
    aml_set_reg(dev, 0x60c0b390, 0x00010103);
    aml_set_reg(dev, 0x00a0b1b8, 0xc0003000);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60c00800, 0x00000110);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60c00800, 0x00000111);
    }
    aml_set_reg(dev, 0x60c0088c, 0x00005050);
    aml_set_reg(dev, 0x60c0b100, 0x00484848);
    aml_set_reg(dev, 0x60c0b104, 0x00484848);
    aml_set_reg(dev, 0x60c06000, 0x00000000);
    aml_set_reg(dev, 0x60c06004, 0x00020000);
    aml_set_reg(dev, 0x60c06008, 0x00000012);
    aml_set_reg(dev, 0x60c0600c, 0x00000001);
    aml_set_reg(dev, 0x60c06048, 0x00010000);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60c06200, 0x00000002);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60c06200, 0x00000012);
    }
    aml_set_reg(dev, 0x60c06204, 0x00000001);
    aml_set_reg(dev, 0x60c06208, 0x00000000 | tx_pwr);
    aml_set_reg(dev, 0x60c0620c, 0x00000001);
    aml_set_reg(dev, 0x60c06210, 0x000000fc);
    aml_set_reg(dev, 0x60c06214, 0x00000001);
    aml_set_reg(dev, 0x60c06218, 0x00000000);
    aml_set_reg(dev, 0x60c0621c, 0x00000000);
    aml_set_reg(dev, 0x60c06220, 0x0000000c);
    aml_set_reg(dev, 0x60c06224, 0x00000000);
    aml_set_reg(dev, 0x60c06228, 0x00000080 | (rate + 8));
    aml_set_reg(dev, 0x60c0622c, 0x00000000 | length1);
    aml_set_reg(dev, 0x60c06230, 0x00000000 | length);
    aml_set_reg(dev, 0x60c06234, 0x00000000);
    aml_set_reg(dev, 0x60c06238, 0x00000080);
    aml_set_reg(dev, 0x60c0623c, 0x00000000);
    aml_set_reg(dev, 0x60c06240, 0x00000000);
    aml_set_reg(dev, 0x60c06010, 0x00000000);
    aml_set_reg(dev, 0x60c06000, 0x00000317);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "2g_5g_mimo_ht_tx:bw=%d, rate = %d, length=0x%x, tx_pwr=%d\n",bw, rate, (tx_param1 >> 8), tx_pwr);
    return 0;
}

int aml_ht_siso_wf1_tx(struct net_device *dev, U8 bw, U8 rate, U8 tx_pwr, U8 length, U8 length1, U8 length2)
{
    aml_set_reg(dev, 0x60c00840, 0x80010001);
    aml_set_reg(dev, 0x00a0d084, 0x00020001);
    aml_set_reg(dev, 0x00a0d090, 0x4f210033);
    aml_set_reg(dev, 0x60805010, 0xb9d70242);
    aml_set_reg(dev, 0x60805014, 0x0000013f);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60805008, 0x00001100);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60805008, 0x00001150);
    }
    aml_set_reg(dev, 0x60c0b004, 0x00000001);
    aml_set_reg(dev, 0x00a0b00c, 0x00000012);
    aml_set_reg(dev, 0x60c0b390, 0x00010102);
    aml_set_reg(dev, 0x00a0b1b8, 0xc0003100);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60c00800, 0x00000110);
    } else if(bw == 0x1) {
        aml_set_reg(dev, 0x60c00800, 0x00000111);
    }
    aml_set_reg(dev, 0x60c0088c, 0x00005050);
    aml_set_reg(dev, 0x60c0b100, 0x00303030);
    aml_set_reg(dev, 0x60c06000, 0x00000000);
    aml_set_reg(dev, 0x60c06004, 0x00020000);
    aml_set_reg(dev, 0x60c06008, 0x00000012);
    aml_set_reg(dev, 0x60c0600c, 0x00000001);
    aml_set_reg(dev, 0x60c06048, 0x00010000);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60c06200, 0x00000002);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60c06200, 0x00000012);
    }
    aml_set_reg(dev, 0x60c06204, 0x00000001);
    aml_set_reg(dev, 0x60c06208, 0x00000000 | tx_pwr);
    aml_set_reg(dev, 0x60c0620c, 0x00000000);
    aml_set_reg(dev, 0x60c06210, 0x000000fc);
    aml_set_reg(dev, 0x60c06214, 0x00000001);
    aml_set_reg(dev, 0x60c06218, 0x00000000);
    aml_set_reg(dev, 0x60c0621c, 0x00000000);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60c06220, 0x00000008);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60c06220, 0x0000000c);
    }
    aml_set_reg(dev, 0x60c06224, 0x00000000);
    aml_set_reg(dev, 0x60c06228, 0x00000080 | rate);
    aml_set_reg(dev, 0x60c0622c, 0x00000000 | length1);
    aml_set_reg(dev, 0x60c06230, 0x00000000 | length);
    aml_set_reg(dev, 0x60c06234, 0x00000000);
    aml_set_reg(dev, 0x60c06238, 0x00000080);
    aml_set_reg(dev, 0x60c0623c, 0x00000000);
    aml_set_reg(dev, 0x60c06240, 0x00000000);
    aml_set_reg(dev, 0x60c06010, 0x00000000);
    aml_set_reg(dev, 0x60c06000, 0x00000317);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "2g_5g_wf1_ht_tx:bw=%d, rate = %d, length=0x%x, tx_pwr=%d\n",bw, rate, (tx_param1 >> 8), tx_pwr);
    return 0;
}

int aml_vht_siso_wf0_tx(struct net_device *dev, U8 bw, U8 rate, U8 tx_pwr, U8 length, U8 length1, U8 length2)
{
    aml_set_reg(dev, 0x60c00840, 0x80010001);
    aml_set_reg(dev, 0x00a0d084, 0x00020001);
    aml_set_reg(dev, 0x00a0d090, 0x4f210033);
    aml_set_reg(dev, 0x60805010, 0xb9d70242);
    aml_set_reg(dev, 0x60805014, 0x0000013f);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60805008, 0x00001100);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60805008, 0x00001150);
    } else if (bw == 0x2) {
        aml_set_reg(dev, 0x60805008, 0x000011a0);
    }
    aml_set_reg(dev, 0x60c0b004, 0x00000001);
    aml_set_reg(dev, 0x00a0b00c, 0x00000011);
    aml_set_reg(dev, 0x60c0b390, 0x00010101);
    aml_set_reg(dev, 0x00a0b1b8, 0xc0003000);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60c00800, 0x00000110);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60c00800, 0x00000111);
    } else if (bw == 0x2) {
        aml_set_reg(dev, 0x60c00800, 0x00000112);
    }
    aml_set_reg(dev, 0x60c0b100, 0x00303030);
    aml_set_reg(dev, 0x60c06000, 0x00000000);
    aml_set_reg(dev, 0x60c06004, 0x00020000);
    aml_set_reg(dev, 0x60c06008, 0x00000012);
    aml_set_reg(dev, 0x60c0600c, 0x00000001);
    aml_set_reg(dev, 0x60c06048, 0x00010000);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60c06200, 0x00000004);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60c06200, 0x00000014);
    } else if (bw == 0x2) {
        aml_set_reg(dev, 0x60c06200, 0x00000024);
    }
    aml_set_reg(dev, 0x60c06204, 0x00000001);
    aml_set_reg(dev, 0x60c06208, 0x00000000 | tx_pwr);
    aml_set_reg(dev, 0x60c0620c, 0x00000000);
    aml_set_reg(dev, 0x60c06210, 0x00000000);
    aml_set_reg(dev, 0x60c06214, 0x00000000);
    aml_set_reg(dev, 0x60c06218, 0x00000000);
    aml_set_reg(dev, 0x60c0621c, 0x00000000);
    aml_set_reg(dev, 0x60c06220, 0x0000000c);
    aml_set_reg(dev, 0x60c06224, 0x00000000);
    aml_set_reg(dev, 0x60c06228, 0x00000000);
    aml_set_reg(dev, 0x60c0622c, 0x00000000);
    aml_set_reg(dev, 0x60c06230, 0x00000000);
    aml_set_reg(dev, 0x60c06234, 0x00000080 | rate);
    aml_set_reg(dev, 0x60c06238, 0x00000000 | length1);
    aml_set_reg(dev, 0x60c0623c, 0x00000000 | length);
    aml_set_reg(dev, 0x60c06240, 0x00000000 | length2);
    aml_set_reg(dev, 0x60c06010, 0x00000000);
    aml_set_reg(dev, 0x60c06000, 0x00000317);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "2g_5g_siso_vht_tx:bw=%d, rate = %d, length=0x%x, tx_pwr=%d\n",bw, rate, (tx_param1 >> 8), tx_pwr);
    return 0;
}

int aml_vht_mimo_tx(struct net_device *dev, U8 bw, U8 rate, U8 tx_pwr, U8 length, U8 length1, U8 length2)
{
    aml_set_reg(dev, 0x60c00840, 0x80010001);
    aml_set_reg(dev, 0x00a0d084, 0x00020001);
    aml_set_reg(dev, 0x00a0d090, 0x4f210033);
    aml_set_reg(dev, 0x60805010, 0xb9d70242);
    aml_set_reg(dev, 0x60805014, 0x0000013f);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60805008, 0x00001100);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60805008, 0x00001150);
    } else if (bw == 0x2) {
        aml_set_reg(dev, 0x60805008, 0x000011a0);
    }
    aml_set_reg(dev, 0x60c0b004, 0x00000003);
    aml_set_reg(dev, 0x00a0b00c, 0x00000033);
    aml_set_reg(dev, 0x60c0b390, 0x00010103);
    aml_set_reg(dev, 0x00a0b1b8, 0xc0003000);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60c00800, 0x00000110);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60c00800, 0x00000111);
    } else if (bw == 0x2) {
        aml_set_reg(dev, 0x60c00800, 0x00000112);
    }
    aml_set_reg(dev, 0x60c0b100, 0x00484848);
    aml_set_reg(dev, 0x60c0b104, 0x00484848);
    aml_set_reg(dev, 0x60c06000, 0x00000000);
    aml_set_reg(dev, 0x60c06004, 0x00020000);
    aml_set_reg(dev, 0x60c06008, 0x00000012);
    aml_set_reg(dev, 0x60c0600c, 0x00000001);
    aml_set_reg(dev, 0x60c06048, 0x0000780);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60c06200, 0x00000004);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60c06200, 0x00000014);
    } else if (bw == 0x2) {
        aml_set_reg(dev, 0x60c06200, 0x00000024);
    }
    aml_set_reg(dev, 0x60c06204, 0x00000001);
    aml_set_reg(dev, 0x60c06208, 0x00000000 | tx_pwr);
    aml_set_reg(dev, 0x60c0620c, 0x00000001);
    aml_set_reg(dev, 0x60c06210, 0x00000000);
    aml_set_reg(dev, 0x60c06214, 0x00000000);
    aml_set_reg(dev, 0x60c06218, 0x00000000);
    aml_set_reg(dev, 0x60c0621c, 0x00000000);
    aml_set_reg(dev, 0x60c06220, 0x00000008);
    aml_set_reg(dev, 0x60c06224, 0x00000000);
    aml_set_reg(dev, 0x60c06228, 0x00000000);
    aml_set_reg(dev, 0x60c0622c, 0x00000000);
    aml_set_reg(dev, 0x60c06230, 0x00000000);
    aml_set_reg(dev, 0x60c06234, 0x00000090 | rate);
    aml_set_reg(dev, 0x60c06238, 0x00000000 | length1);
    aml_set_reg(dev, 0x60c0623c, 0x00000000 | length);
    aml_set_reg(dev, 0x60c06240, 0x00000000 | length2);
    aml_set_reg(dev, 0x60c06010, 0x00000000);
    aml_set_reg(dev, 0x60c06000, 0x00000317);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "2g_5g_mimo_vht_tx:bw=%d, rate = %d, length=0x%x, tx_pwr=%d\n",bw, rate, (tx_param1 >> 8), tx_pwr);
    return 0;
}

int aml_vht_siso_wf1_tx(struct net_device *dev, U8 bw, U8 rate, U8 tx_pwr, U8 length, U8 length1, U8 length2)
{
    aml_set_reg(dev, 0x60c00840, 0x80010001);
    aml_set_reg(dev, 0x00a0d084, 0x00020001);
    aml_set_reg(dev, 0x00a0d090, 0x4f210033);
    aml_set_reg(dev, 0x60805010, 0xb9d70242);
    aml_set_reg(dev, 0x60805014, 0x0000013f);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60805008, 0x00001100);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60805008, 0x00001150);
    } else if (bw == 0x2) {
        aml_set_reg(dev, 0x60805008, 0x000011a0);
    }
    aml_set_reg(dev, 0x60c0b004, 0x00000001);
    aml_set_reg(dev, 0x00a0b00c, 0x00000012);
    aml_set_reg(dev, 0x60c0b390, 0x00010102);
    aml_set_reg(dev, 0x00a0b1b8, 0xc0003100);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60c00800, 0x00000110);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60c00800, 0x00000111);
    } else if (bw == 0x2) {
        aml_set_reg(dev, 0x60c00800, 0x00000112);
    }
    aml_set_reg(dev, 0x60c0b100, 0x00303030);
    aml_set_reg(dev, 0x60c06000, 0x00000000);
    aml_set_reg(dev, 0x60c06004, 0x00020000);
    aml_set_reg(dev, 0x60c06008, 0x00000012);
    aml_set_reg(dev, 0x60c0600c, 0x00000001);
    aml_set_reg(dev, 0x60c06048, 0x0000780);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60c06200, 0x00000004);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60c06200, 0x00000014);
    } else if (bw == 0x2) {
        aml_set_reg(dev, 0x60c06200, 0x00000024);
    }
    aml_set_reg(dev, 0x60c06204, 0x00000001);
    aml_set_reg(dev, 0x60c06208, 0x00000000 | tx_pwr);
    aml_set_reg(dev, 0x60c0620c, 0x00000000);
    aml_set_reg(dev, 0x60c06210, 0x00000000);
    aml_set_reg(dev, 0x60c06214, 0x00000000);
    aml_set_reg(dev, 0x60c06218, 0x00000000);
    aml_set_reg(dev, 0x60c0621c, 0x00000000);
    aml_set_reg(dev, 0x60c06220, 0x00000008);
    aml_set_reg(dev, 0x60c06224, 0x00000000);
    aml_set_reg(dev, 0x60c06228, 0x00000000);
    aml_set_reg(dev, 0x60c0622c, 0x00000000);
    aml_set_reg(dev, 0x60c06230, 0x00000000);
    aml_set_reg(dev, 0x60c06234, 0x00000080 | rate);
    aml_set_reg(dev, 0x60c06238, 0x00000000 | length1);
    aml_set_reg(dev, 0x60c0623c, 0x00000000 | length);
    aml_set_reg(dev, 0x60c06240, 0x00000000 | length2);
    aml_set_reg(dev, 0x60c06010, 0x00000000);
    aml_set_reg(dev, 0x60c06000, 0x00000317);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "2g_5g_wf1_vht_tx:bw=%d, rate = %d, length=0x%x, tx_pwr=%d\n",bw, rate, (tx_param1 >> 8), tx_pwr);
    return 0;
}

int aml_hesu_siso_wf0_tx(struct net_device *dev, U8 bw, U8 rate, U8 tx_pwr, U8 length, U8 length1, U8 length2)
{
    aml_set_reg(dev, 0x60c00840, 0x80010001);
    aml_set_reg(dev, 0x00a0d084, 0x00020001);
    aml_set_reg(dev, 0x00a0d090, 0x4f210033);
    aml_set_reg(dev, 0x60805010, 0xb9d70242);
    aml_set_reg(dev, 0x60805014, 0x0000013f);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60805008, 0x00001100);
    } else if(bw == 0x1) {
        aml_set_reg(dev, 0x60805008, 0x00001150);
    } else if(bw == 0x2) {
        aml_set_reg(dev, 0x60805008, 0x000011a0);
    }
    aml_set_reg(dev, 0x60c0b004, 0x00000001);
    aml_set_reg(dev, 0x00a0b00c, 0x00000011);
    aml_set_reg(dev, 0x60c0b390, 0x00010101);
    aml_set_reg(dev, 0x00a0b1b8, 0xc0003000);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60c00800, 0x00000110);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60c00800, 0x00000111);
    } else if (bw == 0x2) {
        aml_set_reg(dev, 0x60c00800, 0x00000112);
    }
    aml_set_reg(dev, 0x60c0088c, 0x00005050);
    aml_set_reg(dev, 0x60c0b100, 0x00303030);

    aml_set_reg(dev, 0x60c06000, 0x00000000);
    if (bw == 0x0 || bw == 0x1) {
        aml_set_reg(dev, 0x60c06004, 0x00020000);
    } else if (bw == 0x2) {
        aml_set_reg(dev, 0x60c06004, 0x00010000);
    }
    aml_set_reg(dev, 0x60c06008, 0x00000012);
    aml_set_reg(dev, 0x60c0600c, 0x00000001);
    aml_set_reg(dev, 0x60c06048, 0x00010000);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60c06200, 0x00000005);
        aml_set_reg(dev, 0x60c06204, 0x00000001);
        aml_set_reg(dev, 0x60c06208, 0x00000000 | tx_pwr);
        aml_set_reg(dev, 0x60c0620c, 0x00000000);
        aml_set_reg(dev, 0x60c06210, 0x00000000);
        aml_set_reg(dev, 0x60c06214, 0x00000000);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60c06200, 0x00000015);
        aml_set_reg(dev, 0x60c06204, 0x00000001);
        aml_set_reg(dev, 0x60c06208, 0x00000000 | tx_pwr);
        aml_set_reg(dev, 0x60c0620c, 0x00000000);
        aml_set_reg(dev, 0x60c06210, 0x000000fc);
        aml_set_reg(dev, 0x60c06214, 0x00000001);
    } else if (bw == 0x2) {
        aml_set_reg(dev, 0x60c06200, 0x00000025);
        aml_set_reg(dev, 0x60c06204, 0x00000001);
        aml_set_reg(dev, 0x60c06208, 0x00000000 | tx_pwr);
        aml_set_reg(dev, 0x60c0620c, 0x00000020);
        aml_set_reg(dev, 0x60c06210, 0x000000fc);
        aml_set_reg(dev, 0x60c06214, 0x00000001);
    }
    aml_set_reg(dev, 0x60c06218, 0x00000000);
    aml_set_reg(dev, 0x60c0621c, 0x00000000);
    aml_set_reg(dev, 0x60c06220, 0x00000008);
    aml_set_reg(dev, 0x60c06224, 0x00000012);
    aml_set_reg(dev, 0x60c06228, 0x00000000);
    aml_set_reg(dev, 0x60c0622c, 0x00000001);
    aml_set_reg(dev, 0x60c06230, 0x00000000);
    aml_set_reg(dev, 0x60c06234, 0x00000000);
    aml_set_reg(dev, 0x60c06238, 0x00000080 | rate);
    aml_set_reg(dev, 0x60c0623c, 0x00000000 | length1);
    aml_set_reg(dev, 0x60c06240, 0x00000000 | length);
    aml_set_reg(dev, 0x60c06244, 0x00000000 | length2);
    aml_set_reg(dev, 0x60c06010, 0x00000000);
    aml_set_reg(dev, 0x60c06000, 0x00000317);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "2g_5g_siso_hesu_tx:bw=%d, rate = %d, length=0x%x, tx_pwr=%d\n",bw, rate, (tx_param1 >> 8), tx_pwr);
    return 0;
}

int aml_hesu_mimo_tx(struct net_device *dev, U8 bw, U8 rate, U8 tx_pwr, U8 length, U8 length1, U8 length2)
{
    aml_set_reg(dev, 0x60c00840, 0x80010001);
    aml_set_reg(dev, 0x00a0d084, 0x00020001);
    aml_set_reg(dev, 0x00a0d090, 0x4f210033);
    aml_set_reg(dev, 0x60805010, 0xb9d70242);
    aml_set_reg(dev, 0x60805014, 0x0000013f);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60805008, 0x00001100);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60805008, 0x00001150);
    } else if (bw == 0x2) {
        aml_set_reg(dev, 0x60805008, 0x000011a0);
    }
    aml_set_reg(dev, 0x60c0b004, 0x00000003);
    aml_set_reg(dev, 0x00a0b00c, 0x00000033);
    aml_set_reg(dev, 0x60c0b390, 0x00010103);
    aml_set_reg(dev, 0x00a0b1b8, 0xc0003000);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60c00800, 0x00000110);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60c00800, 0x00000111);
    } else if (bw == 0x2) {
        aml_set_reg(dev, 0x60c00800, 0x00000112);
    }
    aml_set_reg(dev, 0x60c0088c, 0x00005050);
    aml_set_reg(dev, 0x60c0b100, 0x00484848);
    aml_set_reg(dev, 0x60c0b104, 0x00484848);
    aml_set_reg(dev, 0x60c06000, 0x00000000);
    if (bw == 0x0 || bw == 0x1) {
        aml_set_reg(dev, 0x60c06004, 0x00020000);
    } else if (bw == 0x2) {
        aml_set_reg(dev, 0x60c06004, 0x00010000);
    }
    aml_set_reg(dev, 0x60c06008, 0x00000012);
    aml_set_reg(dev, 0x60c0600c, 0x00000001);
    aml_set_reg(dev, 0x60c06048, 0x00010000);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60c06200, 0x00000005);
        aml_set_reg(dev, 0x60c06204, 0x00000003);
        aml_set_reg(dev, 0x60c06208, 0x00000000 | tx_pwr);
        aml_set_reg(dev, 0x60c0620c, 0x00000001);
        aml_set_reg(dev, 0x60c06210, 0x00000067);
        aml_set_reg(dev, 0x60c06214, 0x00000000);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60c06200, 0x00000015);
        aml_set_reg(dev, 0x60c06204, 0x00000003);
        aml_set_reg(dev, 0x60c06208, 0x00000000 | tx_pwr);
        aml_set_reg(dev, 0x60c0620c, 0x00000001);
        aml_set_reg(dev, 0x60c06210, 0x000000fc);
        aml_set_reg(dev, 0x60c06214, 0x00000001);
    } else if (bw == 0x2) {
        aml_set_reg(dev, 0x60c06200, 0x00000025);
        aml_set_reg(dev, 0x60c06204, 0x00000003);
        aml_set_reg(dev, 0x60c06208, 0x00000000 | tx_pwr);
        aml_set_reg(dev, 0x60c0620c, 0x00000001);
        aml_set_reg(dev, 0x60c06210, 0x00000000);
        aml_set_reg(dev, 0x60c06214, 0x00000000);
    }
    aml_set_reg(dev, 0x60c06218, 0x00000000);
    aml_set_reg(dev, 0x60c0621c, 0x00000000);
    aml_set_reg(dev, 0x60c06220, 0x00000008);
    aml_set_reg(dev, 0x60c06224, 0x00000012);
    aml_set_reg(dev, 0x60c06228, 0x00000000);
    aml_set_reg(dev, 0x60c0622c, 0x00000001);
    aml_set_reg(dev, 0x60c06230, 0x00000000);
    aml_set_reg(dev, 0x60c06234, 0x00000000);
    aml_set_reg(dev, 0x60c06238, 0x00000090 | rate);
    aml_set_reg(dev, 0x60c0623c, 0x00000000 | length1);
    aml_set_reg(dev, 0x60c06240, 0x00000000 | length);
    aml_set_reg(dev, 0x60c06244, 0x00000000 | length2);
    aml_set_reg(dev, 0x60c06010, 0x00000000);
    aml_set_reg(dev, 0x60c06000, 0x00000317);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "2g_5g_mimo_hesu_tx:bw=%d, rate = %d, length=0x%x, tx_pwr=%d\n", bw, rate, (tx_param1 >> 8), tx_pwr);
    return 0;
}

int aml_hesu_siso_wf1_tx(struct net_device *dev, U8 bw, U8 rate, U8 tx_pwr, U8 length, U8 length1, U8 length2)
{
    aml_set_reg(dev, 0x60c00840, 0x80010001);
    aml_set_reg(dev, 0x00a0d084, 0x00020001);
    aml_set_reg(dev, 0x00a0d090, 0x4f210033);
    aml_set_reg(dev, 0x60805010, 0xb9d70242);
    aml_set_reg(dev, 0x60805014, 0x0000013f);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60805008, 0x00001100);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60805008, 0x00001150);
    } else if (bw == 0x2) {
        aml_set_reg(dev, 0x60805008, 0x000011a0);
    }
    aml_set_reg(dev, 0x60c0b004, 0x00000001);
    aml_set_reg(dev, 0x00a0b00c, 0x00000012);
    aml_set_reg(dev, 0x60c0b390, 0x00010102);
    aml_set_reg(dev, 0x00a0b1b8, 0xc0003100);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60c00800, 0x00000110);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60c00800, 0x00000111);
    } else if (bw == 0x2) {
        aml_set_reg(dev, 0x60c00800, 0x00000112);
    }
    aml_set_reg(dev, 0x60c0088c, 0x00005050);
    aml_set_reg(dev, 0x60c0b100, 0x00303030);
    aml_set_reg(dev, 0x60c06000, 0x00000000);
    if (bw == 0x0 || bw == 0x1) {
        aml_set_reg(dev, 0x60c06004, 0x00020000);
    } else if (bw == 0x2) {
        aml_set_reg(dev, 0x60c06004, 0x00010000);
    }
    aml_set_reg(dev, 0x60c06008, 0x00000012);
    aml_set_reg(dev, 0x60c0600c, 0x00000001);
    aml_set_reg(dev, 0x60c06048, 0x00010000);
    if (bw == 0x0) {
        aml_set_reg(dev, 0x60c06200, 0x00000005);
        aml_set_reg(dev, 0x60c06204, 0x00000001);
        aml_set_reg(dev, 0x60c06208, 0x00000000 | tx_pwr);
        aml_set_reg(dev, 0x60c0620c, 0x00000000);
        aml_set_reg(dev, 0x60c06210, 0x00000000);
        aml_set_reg(dev, 0x60c06214, 0x00000000);
    } else if (bw == 0x1) {
        aml_set_reg(dev, 0x60c06200, 0x00000015);
        aml_set_reg(dev, 0x60c06204, 0x00000001);
        aml_set_reg(dev, 0x60c06208, 0x00000000 | tx_pwr);
        aml_set_reg(dev, 0x60c0620c, 0x00000000);
        aml_set_reg(dev, 0x60c06210, 0x000000fc);
        aml_set_reg(dev, 0x60c06214, 0x00000001);
    } else if (bw == 0x2) {
        aml_set_reg(dev, 0x60c06200, 0x00000025);
        aml_set_reg(dev, 0x60c06204, 0x00000001);
        aml_set_reg(dev, 0x60c06208, 0x00000000 | tx_pwr);
        aml_set_reg(dev, 0x60c0620c, 0x00000020);
        aml_set_reg(dev, 0x60c06210, 0x000000fc);
        aml_set_reg(dev, 0x60c06214, 0x00000001);
    }
    aml_set_reg(dev, 0x60c06218, 0x00000000);
    aml_set_reg(dev, 0x60c0621c, 0x00000000);
    aml_set_reg(dev, 0x60c06220, 0x00000008);
    aml_set_reg(dev, 0x60c06224, 0x00000012);
    aml_set_reg(dev, 0x60c06228, 0x00000000);
    aml_set_reg(dev, 0x60c0622c, 0x00000001);
    aml_set_reg(dev, 0x60c06230, 0x00000000);
    aml_set_reg(dev, 0x60c06234, 0x00000000);
    aml_set_reg(dev, 0x60c06238, 0x00000080 | rate);
    aml_set_reg(dev, 0x60c0623c, 0x00000000 | length1);
    aml_set_reg(dev, 0x60c06240, 0x00000000 | length);
    aml_set_reg(dev, 0x60c06244, 0x00000000 | length2);
    aml_set_reg(dev, 0x60c06010, 0x00000000);
    aml_set_reg(dev, 0x60c06000, 0x00000317);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "2g_5g_wf1_hesu_tx:bw=%d, rate = %d, length=0x%x, tx_pwr=%d\n",bw, rate, (tx_param1 >> 8), tx_pwr);
    return 0;
}

int aml_set_tx_prot(struct net_device *dev, int tx_pam, int tx_pam1)
{
    int model = (tx_pam & 0x00f00000) >> 20; //0x1 wf0 0x2 wf1 0x3 mimo
    int prot = (tx_pam & 0x000f0000) >> 16; //11b/11ag/ht/vht/hesu
    U8 bw = (tx_pam & 0x0000ff00) >> 8;
    U8 rate = tx_pam & 0x000000ff;
    U8 length2 = (tx_pam1 & 0x0f000000) >> 24;
    U8 length = (tx_pam1 & 0x00ff0000) >> 16;
    U8 length1 = (tx_pam1 & 0x0000ff00) >> 8;
    U8 tx_pwr = tx_pam1 & 0x000000ff;
    U8 channel = (tx_pam & 0xff000000) >>24;

    if (prot == 0x0 && model == 0x1) {
        aml_11b_siso_wf0_tx(dev, rate, tx_pwr, length, length1, length2);
    } else if (prot == 0x0 && model == 0x2) {
        aml_11b_siso_wf1_tx(dev, rate, tx_pwr, length, length1, length2);
    } else if (prot == 0x1 && model == 0x1) {
        aml_11ag_siso_wf0_tx(dev, rate, tx_pwr, length, length1, length2);
    } else if (prot == 0x1 && model == 0x2) {
        aml_11ag_siso_wf1_tx(dev, rate, tx_pwr, length, length1, length2);
    } else if (prot == 0x2 && model == 0x1) {
        aml_ht_siso_wf0_tx(dev,bw, rate, tx_pwr, length, length1, length2);
    } else if (prot == 0x2 && model == 0x2) {
        aml_ht_siso_wf1_tx(dev,bw, rate, tx_pwr, length, length1, length2);
    } else if (prot == 0x2 && model == 0x3) {
        aml_ht_mimo_tx(dev,bw, rate, tx_pwr, length, length1, length2);
    } else if (prot == 0x3 && model == 0x1) {
        aml_vht_siso_wf0_tx(dev,bw, rate, tx_pwr, length, length1, length2);
    } else if (prot == 0x3 && model == 0x2) {
        aml_vht_siso_wf1_tx(dev,bw, rate, tx_pwr, length, length1, length2);
    } else if (prot == 0x3 && model == 0x3) {
        aml_vht_mimo_tx(dev,bw, rate, tx_pwr, length, length1, length2);
    } else if (prot == 0x4 && model == 0x1) {
        aml_hesu_siso_wf0_tx(dev,bw, rate, tx_pwr, length, length1, length2);
    } else if (prot == 0x4 && model == 0x2) {
        aml_hesu_siso_wf1_tx(dev,bw, rate, tx_pwr, length, length1, length2);
    } else if (prot == 0x4 && model == 0x3) {
        aml_hesu_mimo_tx(dev,bw, rate, tx_pwr, length, length1, length2);
    } else {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "tx param error\n");
    }
    if (channel > 14)
    {
        if (model == 3)
        {
            aml_set_reg(dev,0x60c0b100, 0x00505050);
            aml_set_reg(dev,0x60c0b104, 0x00505050);
        }
        else
        {
            aml_set_reg(dev,0x60c0b100, 0x00383838);
            aml_set_reg(dev,0x60c0b104, 0x00383838);
        }
    }
    else
    {
        if (model == 3)
        {
            aml_set_reg(dev,0x60c0b100, 0x00404040);
            aml_set_reg(dev,0x60c0b104, 0x00404040);
        }
        else
        {
            aml_set_reg(dev,0x60c0b100, 0x002e2e2e);
            aml_set_reg(dev,0x60c0b104, 0x002e2e2e);
        }
    }
    return 0;
}

extern unsigned char g_fw_recovery_flag;
int aml_set_tx_end(struct net_device *dev, union iwreq_data *wrqu, char *extra)
{
    unsigned char err_msg[] = " FW-error";
    struct aml_vif *aml_vif = netdev_priv(dev);

    tx_start = 0;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set_tx_end\n");
    //aml_set_reg(dev, 0x60c06000, 0x00000000);  //tx end
    aml_set_reg(dev, 0x60805018, 0x00000001);  //phy reset
    aml_set_reg(dev, 0x60805018, 0x00000000);
    aml_set_reg(dev, 0x60c0b390, 0x00011103);

    if (g_fw_recovery_flag || (!_aml_get_efuse(aml_vif, 0))) {
        wrqu->data.length = scnprintf(extra, IW_PRIV_SIZE_MASK, "%s", err_msg);
        wrqu->data.length++;
        g_fw_recovery_flag = 0;
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "%s, %d: recovery flag found!\n", __func__, __LINE__);
    }

    return 0;
}

int aml_set_tx_start(struct net_device *dev, union iwreq_data *wrqu, char *extra)
{
    unsigned char err_msg[] = " FW-error";
    struct aml_vif *aml_vif = netdev_priv(dev);

    tx_start = 1;
    aml_set_tx_prot(dev, tx_param, tx_param1);

    if (g_fw_recovery_flag || (!_aml_get_efuse(aml_vif, 0))) {
        wrqu->data.length = scnprintf(extra, IW_PRIV_SIZE_MASK, "%s", err_msg);
        wrqu->data.length++;
        g_fw_recovery_flag = 0;
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "%s, %d: recovery flag found!\n", __func__, __LINE__);
    }

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set_tx_start\n");
    return 0;
}
int aml_set_offset_power_vld(struct net_device *dev)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    unsigned int reg_val = 0;
    unsigned int reg_val_second = 0;
    unsigned int xosc_vld = 0;
    unsigned int xosc_vld_second = 0;

    xosc_vld = _aml_get_efuse(aml_vif, EFUSE_BASE_0B);
    xosc_vld = xosc_vld & BIT20; //0x1:xosc first times enable, 0x0 xosc first times disable
    xosc_vld_second = _aml_get_efuse(aml_vif, EFUSE_BASE_07);
    xosc_vld_second = xosc_vld_second & BIT31; //0x1:xosc second times enable, 0x0 xosc second times disable
    reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_09);
    reg_val_second = _aml_get_efuse(aml_vif, EFUSE_BASE_18);
    //xosc first times enable,offset power first times vld disable
    if (((reg_val & 0x06180000) == 0x0) && (xosc_vld == BIT20) && (xosc_vld_second == 0x0)) {
        offset_times = 1;
        reg_val = reg_val | 0x06180000;
        _aml_set_efuse(aml_vif, EFUSE_BASE_09, reg_val);
    } else if (((reg_val_second & 0xc0000000) == 0x0) && (xosc_vld_second == BIT31)) {
        //xosc second times enable,offset power first times vld enable,offset power second times vld disable
        offset_times = 2;
        reg_val_second = reg_val_second | 0xc0000000;
        _aml_set_efuse(aml_vif, EFUSE_BASE_18, reg_val_second);
    } else {
        offset_times = 3;
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "efuse vld set fail, vld has been written\n");
    }
    return 0;
}

int aml_set_second_offset_power_vld(struct net_device *dev)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    unsigned int reg_val = 0;

    reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_18);
    reg_val = reg_val | 0xc0000000;
    _aml_set_efuse(aml_vif, EFUSE_BASE_18, reg_val);

}

int aml_recy_ctrl(struct net_device *dev, int recy_id)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
#ifdef CONFIG_AML_RECOVERY
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    struct aml_cmd_mgr *cmd_mgr = &aml_hw->cmd_mgr;
#endif

    switch (recy_id) {
#ifdef CONFIG_AML_RECOVERY
        case 0:
            AML_INFO("disable recovery detection");
            aml_recy_disable();
            break;
        case 1:
            AML_INFO("enable recovery detection");
            aml_recy_enable();
            break;
        case 2:
            AML_INFO("do simulate cmd queue crashed");
            spin_lock_bh(&cmd_mgr->lock);
            cmd_mgr->state = AML_CMD_MGR_STATE_CRASHED;
            spin_unlock_bh(&cmd_mgr->lock);
            break;
        case 3:
            AML_INFO("do recovery straightforward");
            aml_recy_doit(aml_hw);
            break;
        case 5:
            AML_INFO("do simulate link loss recovery");
            aml_recy_link_loss_test();
            break;
#endif
        case 4:
            AML_INFO("do firmware soft reset");
            aml_fw_reset(aml_vif);
            break;

        default:
            AML_INFO("unknow recovery operation");
            break;
    }
    return 0;
}

int aml_set_tx_path(struct net_device *dev, int path, int channel)
{
    unsigned int reg_val = 0;

    tx_path = path;//mode:wf0 0x1 wf1 0x2 mimo 0x3

    if (tx_path > 0x3) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "set_tx_path error:%d\n",tx_path);
        return -1;
    }
    tx_channel = (channel & 0x000000ff) << 24;
    tx_path = (tx_path & 0x0000000f) << 20;
    tx_param = tx_param & 0x000fffff;
    tx_param = tx_param | tx_path | tx_channel;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "aml_set_tx_path:%d\n", path);
    if ((channel <= 14) && (path == 3))
    {
        aml_set_reg(dev, 0x60c0b500, 0x00041000);
    }

    switch (path) {
        case 1:
            if (channel <= 14) {
                reg_val = aml_rf_reg_read(dev, 0x80001818);
                aml_rf_reg_write(dev, 0x80001818, reg_val & 0xFE3FFFFF);
                reg_val = aml_rf_reg_read(dev, 0x80001818);
                aml_rf_reg_write(dev, 0x80001818, reg_val | 0x80000000);
            } else {
                reg_val = aml_rf_reg_read(dev, 0x80001808);
                aml_rf_reg_write(dev, 0x80001808, reg_val & 0xFFE3FFFF);
                reg_val = aml_rf_reg_read(dev, 0x80001808);
                aml_rf_reg_write(dev, 0x80001808, reg_val | 0x02000000);
            }
            break;
        case 2:
            if (channel <= 14) {
                reg_val = aml_rf_reg_read(dev, 0x80000818);
                aml_rf_reg_write(dev, 0x80000818, reg_val & 0xFE3FFFFF);
                reg_val = aml_rf_reg_read(dev, 0x80000818);
                aml_rf_reg_write(dev, 0x80000818, reg_val | 0x80000000);
            } else {
                reg_val = aml_rf_reg_read(dev, 0x80000808);
                aml_rf_reg_write(dev, 0x80000808, reg_val & 0xFFE3FFFF);
                reg_val = aml_rf_reg_read(dev, 0x80000808);
                aml_rf_reg_write(dev, 0x80000808, reg_val | 0x02000000);
            }
            break;

        default:
            AML_PRINT(AML_DBG_MODULES_IWPRIV, "set antenna error :%x\n", path);
            break;
    }

    return 0;
}

int aml_fix_tx_power(struct net_device *dev, int pwr)
{
    struct aml_vif *aml_vif = netdev_priv(dev);

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "aml_fix_tx_power: 0x%08x\n", pwr);

    _aml_fix_txpwr(aml_vif, pwr);

    return 0;
}

int aml_emb_la_capture(struct net_device *dev, int bus1, int bus2)
{
    struct aml_vif *aml_vif = netdev_priv(dev);

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "bus1: 0x%x bus2:0x%x\n", bus1, bus2);

    _aml_set_la_capture(aml_vif, bus1, bus2);

    return 0;

}

int aml_emb_la_enable(struct net_device *dev)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    int enable  = 1;

    if (aml_hw->la_enable) {
        AML_INFO("la had enable!");
        return 0;
    }

    if (!(aml_hw->rx_buf_state & BUFFER_TX_USED_FLAG)) {
        aml_send_set_buf_state_req(aml_hw, BUFFER_RX_FORCE_REDUCE);
    }

    while (!(aml_hw->rx_buf_state & BUFFER_TX_USED_FLAG)) {
        usleep_range(2,3);
    }

    if (aml_hw->rx_buf_state & BUFFER_TX_USED_FLAG) {
        _aml_set_la_enable(aml_hw,enable);
        aml_hw->la_enable = 1;
    }
    return 0;
}

extern struct aml_dyn_snr_cfg g_dyn_snr;
static int aml_dyn_snr_cfg(struct net_device *dev, int enable, int mcs_ration)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    u32 snr_cfg = 0;

    if (enable)
    {
        g_dyn_snr.enable = enable;
        g_dyn_snr.snr_mcs_ration = mcs_ration;
    }
    else
    {
        g_dyn_snr.enable = enable;

        snr_cfg = AML_REG_READ(aml_hw->plat, AML_ADDR_SYSTEM, 0xc00828);
        snr_cfg &= ~ (BIT(29)|BIT(30));
        AML_REG_WRITE(snr_cfg, aml_hw->plat, AML_ADDR_SYSTEM, 0xc00828);
    }
    return 0;
}

int aml_set_tx_mode(struct net_device *dev, int mode)
{
    tx_mode = mode;

    if (tx_mode > 0x4) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "set_tx_mode error:%d\n", tx_mode);
        return -1;
    }
    tx_mode = (tx_mode & 0x0000000f) << 16;
    tx_param = tx_param & 0xfff0ffff;
    tx_param = tx_param | tx_mode;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set_tx_mode:%d\n", mode);
    return 0;
}

int aml_set_tx_bw(struct net_device *dev, int bw)
{
    tx_bw = bw;

    tx_bw = (tx_bw & 0x000000ff) << 8;
    tx_param = tx_param & 0xffff00ff;
    tx_param = tx_param | tx_bw;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set_tx_bw:%d\n", bw);
    return 0;
}

int aml_set_tx_rate(struct net_device *dev, int rate)
{
    tx_rate = rate;
    tx_rate = tx_rate & 0x000000ff;
    tx_param = tx_param & 0xffffff00;
    tx_param = tx_param | tx_rate;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set_tx_rate:%d\n", rate);
    return 0;
}

int aml_set_tx_len(struct net_device *dev, int len)
{
    if (len > 0xfffff) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "set_tx_len error:%d\n", len);
        return -1;
    }
    tx_len= (len & 0x000000ff) << 8;
    tx_len1 = (len & 0x0000ff00) << 8;
    tx_len2 = (len & 0x000f0000) << 8;
    tx_param1 = tx_param1 & 0xf00000ff;
    tx_param1 = tx_param1 | tx_len | tx_len1 | tx_len2;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "aml_set_tx_len:0x%x\n", len);
    return 0;
}

int aml_set_tx_pwr(struct net_device *dev, int pwr)
{
    tx_pwr = pwr;

    if (tx_pwr > 0xff) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "set_tx_pwr error :%d\n", tx_pwr);
        return -1;
    }
    tx_pwr = tx_pwr & 0x000000ff;
    tx_param1 = tx_param1 & 0xffffff00;
    tx_param1 = tx_param1 | tx_pwr;
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "set_tx_pwr:%d\n", pwr);
    return 0;
}

int aml_set_olpc_pwr(struct net_device *dev,int tx_param1)
{
    int tx_pwr = tx_param1 & 0x000000ff;
    aml_set_reg(dev, MACBYP_TXV_ADDR + MACBYP_TXV_08, tx_pwr); //bit[7:0] : txv1_txpwr_level_idx
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "aml_set_olpc_pwr tx_pwr=0x%x\n", (tx_param1 & 0x000000ff));
    return 0;
}

int aml_set_xosc_ctune(struct net_device *dev,union iwreq_data *wrqu, char *extra, int xosc_param)
{
    unsigned int xosc = aml_get_reg_2(dev, XOSC_CTUNE_BASE, wrqu, extra);
    xosc = xosc & 0xfffff00f;
    if (xosc_param > 0x3f) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "aml_xosc_ctune error=0x%x\n", xosc_param);
        return -1;
    }

    xosc_param = (xosc_param & 0x000000ff) << 4;
    xosc = xosc | xosc_param;
    aml_set_reg(dev, XOSC_CTUNE_BASE, xosc);

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "aml_xosc_ctune=0x%x\n", (xosc & 0x00000ff0) >> 4);
    return 0;
}

int aml_set_power_offset(struct net_device *dev,union iwreq_data *wrqu, char *extra, int pwr_offset)
{
    unsigned int offset = pwr_offset & 0x0000001f;
    unsigned int reference_pw;
    unsigned int ret;
    if ((pwr_offset & 0xffffefff) > 0x1f) {//bit[12]:0 wf0;1 wf1
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "aml_set_power_offset error=0x%x\n", pwr_offset);
        return -1;
    }

    if (offset > 0xf) {
        reference_pw = (offset - 32) * 32 + 1024;
    } else {
        reference_pw = offset * 32;
    }

    if ((pwr_offset & 0x1000) == 0) {//wf0
        ret = aml_get_reg_2(dev, POWER_OFFSET_BASE_WF0, wrqu, extra);
        ret = ret & 0xfffffc00;
        reference_pw = reference_pw & 0x000003ff;
        ret = ret | reference_pw;
        aml_set_reg(dev, POWER_OFFSET_BASE_WF0, ret);
    } else {//wf1
        ret = aml_get_reg_2(dev, POWER_OFFSET_BASE_WF1, wrqu, extra);
        ret = ret & 0xfffffc00;
        reference_pw = reference_pw & 0x000003ff;
        ret = ret | reference_pw;
        aml_set_reg(dev, POWER_OFFSET_BASE_WF1, ret);
    }

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "aml_set_power_offset=0x%x\n", reference_pw);
    return 0;
}

int aml_set_ram_efuse(struct net_device *dev , unsigned int ram_efuse)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    unsigned int reg_val = 0;
    if ((ram_efuse & 0x0fffffff) > 0x1f) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "aml_set_ram_efuse error=0x%x\n", ram_efuse);
        return -1;
    }

    ram_efuse = ram_efuse & 0xf000001f;
    // first write
    if (offset_times == 1) {
        if (ram_efuse >> 28 == 0x0) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1A);
            reg_val = reg_val & 0xffe0ffff;
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 16);
            _aml_set_efuse(aml_vif, EFUSE_BASE_1A, reg_val);
        } else if (ram_efuse >> 28 == 0x1) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1A);
            reg_val = reg_val & 0xe0ffffff;
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 24);
            _aml_set_efuse(aml_vif, EFUSE_BASE_1A, reg_val);
        } else if (ram_efuse >> 28 == 0x2) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1B);
            reg_val = reg_val & 0xffffffe0;
            reg_val = reg_val | (ram_efuse & 0x0000001f);
            _aml_set_efuse(aml_vif, EFUSE_BASE_1B, reg_val);
        } else if (ram_efuse >> 28 == 0x3) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1B);
            reg_val = reg_val & 0xffffe0ff;
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 8);
            _aml_set_efuse(aml_vif, EFUSE_BASE_1B, reg_val);
        } else if (ram_efuse >> 28 == 0x4) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1B);
            reg_val = reg_val & 0xffe0ffff;
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 16);
            _aml_set_efuse(aml_vif, EFUSE_BASE_1B, reg_val);
        } else if (ram_efuse >> 28 == 0x5) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1B);
            reg_val = reg_val & 0xe0ffffff;
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 24);
            _aml_set_efuse(aml_vif, EFUSE_BASE_1B, reg_val);
        } else if (ram_efuse >> 28 == 0x6) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1C);
            reg_val = reg_val & 0xffffffe0;
            reg_val = reg_val | (ram_efuse & 0x0000001f);
            _aml_set_efuse(aml_vif, EFUSE_BASE_1C, reg_val);
        } else if (ram_efuse >> 28 == 0x7) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1C);
            reg_val = reg_val & 0xffffe0ff;
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 8);
            _aml_set_efuse(aml_vif, EFUSE_BASE_1C, reg_val);
        } else if (ram_efuse >> 28 == 0x8) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1E);
            reg_val = reg_val & 0xffffffe0;
            reg_val = reg_val | (ram_efuse & 0x0000001f);
            _aml_set_efuse(aml_vif, EFUSE_BASE_1E, reg_val);
        } else if (ram_efuse >> 28 == 0x9) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1E);
            reg_val = reg_val & 0xffffe0ff;
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 8);
            _aml_set_efuse(aml_vif, EFUSE_BASE_1E, reg_val);
        } else if (ram_efuse >> 28 == 0xa) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1E);
            reg_val = reg_val & 0xffe0ffff;
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 16);
            _aml_set_efuse(aml_vif, EFUSE_BASE_1E, reg_val);
        } else if (ram_efuse >> 28 == 0xb) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1E);
            reg_val = reg_val & 0xe0ffffff;
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 24);
            _aml_set_efuse(aml_vif, EFUSE_BASE_1E, reg_val);
        } else if (ram_efuse >> 28 == 0xc) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1F);
            reg_val = reg_val & 0xffffffe0;
            reg_val = reg_val | (ram_efuse & 0x0000001f);
            _aml_set_efuse(aml_vif, EFUSE_BASE_1F, reg_val);
        }  else if (ram_efuse >> 28 == 0xd) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1F);
            reg_val = reg_val & 0xffffe0ff;
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 8);
            _aml_set_efuse(aml_vif, EFUSE_BASE_1F, reg_val);
        } else if ((ram_efuse >> 28) == 0xe) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1F);
            reg_val = reg_val & 0xffe0ffff;
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 16);
            _aml_set_efuse(aml_vif, EFUSE_BASE_1F, reg_val);
        } else if (ram_efuse >> 28 == 0xf) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_1F);
            reg_val = reg_val & 0xe0ffffff;
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 24);
            _aml_set_efuse(aml_vif, EFUSE_BASE_1F, reg_val);
        }
    } else if (offset_times == 2) {
        if (ram_efuse >> 28 == 0x0) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_15);
            reg_val = reg_val & 0xffffffe0;
            reg_val = reg_val | (ram_efuse & 0x0000001f);
            _aml_set_efuse(aml_vif, EFUSE_BASE_15, reg_val);
        } else if (ram_efuse >> 28 == 0x1) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_15);
            reg_val = reg_val & (!(0x1f << 5));
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 5);
            _aml_set_efuse(aml_vif, EFUSE_BASE_15, reg_val);
        } else if (ram_efuse >> 28 == 0x2) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_15);
            reg_val = reg_val & (!(0x1f << 10));
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 10);
            _aml_set_efuse(aml_vif, EFUSE_BASE_15, reg_val);
        } else if (ram_efuse >> 28 == 0x3) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_15);
            reg_val = reg_val & (!(0x1f << 16));
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 16);
            _aml_set_efuse(aml_vif, EFUSE_BASE_15, reg_val);
        } else if (ram_efuse >> 28 == 0x4) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_15);
            reg_val = reg_val &  (!(0x1f << 21));
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 21);
            _aml_set_efuse(aml_vif, EFUSE_BASE_15, reg_val);
        } else if (ram_efuse >> 28 == 0x5) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_15);
            reg_val = reg_val & (!(0x1f << 26));
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 26);
            _aml_set_efuse(aml_vif, EFUSE_BASE_15, reg_val);
        } else if (ram_efuse >> 28 == 0x6) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_16);
            reg_val = reg_val & 0xffffffe0;
            reg_val = reg_val | (ram_efuse & 0x0000001f);
            _aml_set_efuse(aml_vif, EFUSE_BASE_16, reg_val);
        } else if (ram_efuse >> 28 == 0x7) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_16);
            reg_val = reg_val & (!(0x1f << 5));
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 5);
            _aml_set_efuse(aml_vif, EFUSE_BASE_16, reg_val);
        } else if (ram_efuse >> 28 == 0x8) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_16);
            reg_val = reg_val & (!(0x1f << 16));
            reg_val = reg_val | ((ram_efuse & 0x0000001f) <<16);
            _aml_set_efuse(aml_vif, EFUSE_BASE_16, reg_val);
        } else if (ram_efuse >> 28 == 0x9) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_16);
            reg_val = reg_val & (!(0x1f << 21));
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 21);
            _aml_set_efuse(aml_vif, EFUSE_BASE_16, reg_val);
        } else if (ram_efuse >> 28 == 0xa) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_16);
            reg_val = reg_val &  (!(0x1f << 26));
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 26);
            _aml_set_efuse(aml_vif, EFUSE_BASE_16, reg_val);
        } else if (ram_efuse >> 28 == 0xb) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_17);
            reg_val = reg_val & 0xffffffe0;
            reg_val = reg_val | (ram_efuse & 0x0000001f);
            _aml_set_efuse(aml_vif, EFUSE_BASE_17, reg_val);
        } else if (ram_efuse >> 28 == 0xc) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_17);
            reg_val = reg_val & (!(0x1f << 5));
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 5);
            _aml_set_efuse(aml_vif, EFUSE_BASE_17, reg_val);
        }  else if (ram_efuse >> 28 == 0xd) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_17);
            reg_val = reg_val & (!(0x1f << 10));
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 10);
            _aml_set_efuse(aml_vif, EFUSE_BASE_17, reg_val);
        } else if ((ram_efuse >> 28) == 0xe) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_17);
            reg_val = reg_val & (!(0x1f << 16));
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 16);
            _aml_set_efuse(aml_vif, EFUSE_BASE_17, reg_val);
        } else if (ram_efuse >> 28 == 0xf) {
            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_17);
             reg_val = reg_val & (!(0x1f << 21));
            reg_val = reg_val | ((ram_efuse & 0x0000001f) << 21);
            _aml_set_efuse(aml_vif, EFUSE_BASE_17, reg_val);
        }
    } else {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, " efuse has been written\n");
    }

    return 0;
}

int aml_get_xosc_efuse_times(struct net_device *dev , union iwreq_data *wrqu, char *extra)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    unsigned int reg_val = 0;
    unsigned int reg_val_second = 0;
    unsigned int times = 0;

    reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_0B);
    reg_val_second = _aml_get_efuse(aml_vif, EFUSE_BASE_07);
    //xosc first times vld disable
    if ((reg_val & BIT20) == 0) {
        times = 2;
    } else if (((reg_val & BIT20) != 0) && ((reg_val_second & BIT31) == 0)) {
    //xosc first times vld enable, second times vld disable
        times = 1;
    } else {
        times = 0;
    }

    wrqu->data.length = scnprintf(extra, IW_PRIV_SIZE_MASK, "&0x%08x", times);
    wrqu->data.length++;

    return 0;
}

int aml_get_mac_efuse_times(struct net_device *dev , union iwreq_data *wrqu, char *extra)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    unsigned int mac_val = 0;
    unsigned int mac_val_second = 0;
    unsigned int times = 0;

    mac_val = _aml_get_efuse(aml_vif, EFUSE_BASE_01);
    mac_val_second = _aml_get_efuse(aml_vif, EFUSE_BASE_10);
    if (mac_val == 0) {
        times = 2;
    } else if ((mac_val != 0) && (mac_val_second == 0)) {
        times = 1;
    } else {
        times = 0;
    }

    wrqu->data.length = scnprintf(extra, IW_PRIV_SIZE_MASK, "&0x%08x", times);
    wrqu->data.length++;
    return 0;
}


int aml_set_tx_frame_delay(struct net_device *dev, int frame_delay)
{
    aml_set_reg(dev, 0x60c06048, frame_delay);
}

int aml_set_xosc_efuse(struct net_device *dev , int xosc_efuse)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    unsigned int reg_val = 0;

    if (xosc_efuse > 0xff) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "aml_set_xosc_efuse error=0x%x\n", xosc_efuse);
        return -1;
    }

    reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_0B);
    //xosc first times vld disable
    if ((reg_val & BIT20) == 0) {
        reg_val = reg_val | BIT20;
        _aml_set_efuse(aml_vif, EFUSE_BASE_0B, reg_val);

        reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_0F);
        reg_val = reg_val | (xosc_efuse << 24);
        _aml_set_efuse(aml_vif, EFUSE_BASE_0F, reg_val);
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "aml_set_xosc_efuse=0x%x\n", xosc_efuse);
    } else {
        reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_07);
        //xosc second times vld disable
        if ((reg_val & BIT31) == 0) {
            reg_val = reg_val | BIT31;
            _aml_set_efuse(aml_vif, EFUSE_BASE_07, reg_val);

            reg_val = _aml_get_efuse(aml_vif, EFUSE_BASE_0F);
            reg_val = reg_val | (xosc_efuse << 16);
            _aml_set_efuse(aml_vif, EFUSE_BASE_0F, reg_val);
            AML_PRINT(AML_DBG_MODULES_IWPRIV, "second aml_set_xosc_efuse=0x%x\n", xosc_efuse);
        }
    }

    return 0;
}


static int aml_pcie_lp_switch(struct net_device *dev, int status)
{
    int ret = 0;
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    struct aml_plat * aml_plat = aml_hw->plat;

    if (PS_D3_STATUS == status)
    {
        pci_save_state(aml_plat->pci_dev);
        pci_enable_wake(aml_plat->pci_dev, PCI_D0, 1);
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "--------------D3---------------\n");
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "pci->dev_flags = 0x%x state 0x%x d3_delay 0x%x\n",aml_plat->pci_dev->dev_flags, aml_plat->pci_dev->current_state, aml_plat->pci_dev->D3HOT_DELAY);
        ret = pci_set_power_state(aml_plat->pci_dev, PCI_D3hot);
    }
    else if (PS_D0_STATUS == status)
    {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "%s %d pci_dev->current_state 0x%x\n", __func__,__LINE__,aml_plat->pci_dev->current_state);
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "--------------D0---------------\n");
        ret = pci_set_power_state(aml_plat->pci_dev, PCI_D0);
    }
    else
    {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "%s %d: set param err\n", __func__, __LINE__);
        return 0;
    }
    if (ret) {
        ERROR_DEBUG_OUT("pci_set_power_state error %d\n", ret);
    }
    return ret;
}

int aml_set_fwlog_cmd(struct net_device *dev, int mode)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    int ret = 0;

    if (aml_bus_type != PCIE_MODE) {
        if (mode == 0) {
            ret = aml_traceind(aml_vif->aml_hw->ipc_env->pthis, mode);
            if (ret < 0)
                return -1;
        }

#ifdef CONFIG_AML_DEBUGFS
        ret = aml_log_file_info_init(mode);
        if (ret < 0) {
            AML_PRINT(AML_DBG_MODULES_IWPRIV, "aml_log_file_info_init fail\n");
            return -1;
        }
#endif

        aml_send_fwlog_cmd(aml_vif, mode);
    }
    return 0;
}

int aml_is_valid_mac_addr(const char* mac)
{
    int i = 0;
    char sep = ':';

    if (strlen(mac) != strlen("00:00:00:00:00:00")) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "%s %d mac size error!\n", __func__, __LINE__);
        return -1;
    }

    for (i = 0; i < strlen(mac); i++) {
        if ((i % 3) == 2) {
            if (mac[i] != sep) {
                AML_PRINT(AML_DBG_MODULES_IWPRIV, "%s %d mac format error!\n", __func__,__LINE__);
                return -1;
            }
        } else {
            if ((('0' <= mac[i]) && (mac[i] <= '9')) || (('a' <= mac[i]) && (mac[i] <= 'f'))) {
                ;
            } else {
                AML_PRINT(AML_DBG_MODULES_IWPRIV, "%s %d mac invalid!\n", __func__, __LINE__);
                return -1;
            }
        }
    }
    return 0;
}
void aml_set_wifi_mac_addr(struct net_device *dev, char* arg_iw)
{
    char **mac_cmd;
    int cmd_arg;
    char sep = ':';
    unsigned int mac_val = 0;
    unsigned int mac_val_second = 0;
    unsigned int efuse_data_l = 0;
    unsigned int efuse_data_h = 0;
    struct aml_vif *aml_vif = netdev_priv(dev);

    mac_val = _aml_get_efuse(aml_vif, EFUSE_BASE_01);
    mac_val_second = _aml_get_efuse(aml_vif, EFUSE_BASE_10);

    if (mac_val == 0) {
        if (!aml_is_valid_mac_addr(arg_iw)) {
            mac_cmd = aml_cmd_char_phrase(sep, arg_iw, &cmd_arg);
            if (mac_cmd) {
                efuse_data_l = (simple_strtoul(mac_cmd[2], NULL,16) << 24) | (simple_strtoul(mac_cmd[3], NULL,16) << 16)
                               | (simple_strtoul(mac_cmd[4], NULL,16) << 8) | simple_strtoul(mac_cmd[5], NULL,16);
                efuse_data_h = (simple_strtoul(mac_cmd[0], NULL,16) << 8) | (simple_strtoul(mac_cmd[1], NULL,16));

                _aml_set_efuse(aml_vif, EFUSE_BASE_01, efuse_data_l);
                efuse_data_h = (efuse_data_h & 0xffff);
                _aml_set_efuse(aml_vif, EFUSE_BASE_02, efuse_data_h);

                AML_PRINT(AML_DBG_MODULES_IWPRIV, "iwpriv write WIFI MAC addr is:  %02x:%02x:%02x:%02x:%02x:%02x\n",
                    (efuse_data_h & 0xff00) >> 8, efuse_data_h & 0x00ff, (efuse_data_l & 0xff000000) >> 24,
                    (efuse_data_l & 0x00ff0000) >> 16, (efuse_data_l & 0xff00) >> 8, efuse_data_l & 0xff);
            }
            kfree(mac_cmd);
        }
    } else if ((mac_val != 0) && (mac_val_second == 0)) {
        if (!aml_is_valid_mac_addr(arg_iw)) {
            mac_cmd = aml_cmd_char_phrase(sep, arg_iw, &cmd_arg);
            if (mac_cmd) {
                mac_val = _aml_get_efuse(aml_vif, EFUSE_BASE_07);
                mac_val = mac_val | BIT17;
                _aml_set_efuse(aml_vif, EFUSE_BASE_07, mac_val);

                efuse_data_l = (simple_strtoul(mac_cmd[2], NULL,16) << 24) | (simple_strtoul(mac_cmd[3], NULL,16) << 16)
                               | (simple_strtoul(mac_cmd[4], NULL,16) << 8) | simple_strtoul(mac_cmd[5], NULL,16);
                efuse_data_h = (simple_strtoul(mac_cmd[0], NULL,16) << 8) | (simple_strtoul(mac_cmd[1], NULL,16));

                _aml_set_efuse(aml_vif, EFUSE_BASE_10, efuse_data_l);
                efuse_data_h = (efuse_data_h & 0xffff);
                _aml_set_efuse(aml_vif, EFUSE_BASE_11, efuse_data_h);

                AML_PRINT(AML_DBG_MODULES_IWPRIV, "iwpriv write second WIFI MAC addr is:  %02x:%02x:%02x:%02x:%02x:%02x\n",
                    (efuse_data_h & 0xff00) >> 8, efuse_data_h & 0x00ff, (efuse_data_l & 0xff000000) >> 24,
                    (efuse_data_l & 0x00ff0000) >> 16, (efuse_data_l & 0xff00) >> 8, efuse_data_l & 0xff);
            }
            kfree(mac_cmd);
        }

    } else {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "Wifi mac has been written\n");
    }
}

int aml_get_mac_addr(struct net_device *dev,union iwreq_data *wrqu, char *extra)
{
    unsigned int efuse_data_l = 0;
    unsigned int efuse_data_h = 0;
    unsigned int efuse_data = 0;
    struct aml_vif *aml_vif = netdev_priv(dev);

    efuse_data = _aml_get_efuse(aml_vif, EFUSE_BASE_07);

    if (efuse_data & BIT17) {
        efuse_data_l = _aml_get_efuse(aml_vif, EFUSE_BASE_10);
        efuse_data_h = _aml_get_efuse(aml_vif, EFUSE_BASE_11);
    } else {
        efuse_data_l = _aml_get_efuse(aml_vif, EFUSE_BASE_01);
        efuse_data_h = _aml_get_efuse(aml_vif, EFUSE_BASE_02);
    }
    if (efuse_data_l != 0 || efuse_data_h != 0) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "efuse addr:%08x,%08x, MAC addr is: %02x:%02x:%02x:%02x:%02x:%02x\n", EFUSE_BASE_01, EFUSE_BASE_02,
            (efuse_data_h & 0xff00) >> 8,efuse_data_h & 0x00ff, (efuse_data_l & 0xff000000) >> 24,
            (efuse_data_l & 0x00ff0000) >> 16,(efuse_data_l & 0xff00) >> 8,efuse_data_l & 0xff);

        wrqu->data.length = scnprintf(extra, IW_PRIV_SIZE_MASK, " %02x:%02x:%02x:%02x:%02x:%02x\n",
            (efuse_data_h & 0xff00) >> 8,efuse_data_h & 0x00ff, (efuse_data_l & 0xff000000) >> 24,
            (efuse_data_l & 0x00ff0000) >> 16,(efuse_data_l & 0xff00) >> 8,efuse_data_l & 0xff);
        wrqu->data.length++;
    } else {
        aml_get_mac_addr_from_conftxt(&efuse_data_l, &efuse_data_h);

        AML_PRINT(AML_DBG_MODULES_IWPRIV, "No mac address is written into efuse! get_mac_addr_from_conftxt!\nMAC addr is: %02x:%02x:%02x:%02x:%02x:%02x\n",
            (efuse_data_h & 0xff00) >> 8,efuse_data_h & 0x00ff, (efuse_data_l & 0xff000000) >> 24,
            (efuse_data_l & 0x00ff0000) >> 16,(efuse_data_l & 0xff00) >> 8,efuse_data_l & 0xff);

        wrqu->data.length = scnprintf(extra, IW_PRIV_SIZE_MASK, " %02x:%02x:%02x:%02x:%02x:%02x\n",
            (efuse_data_h & 0xff00) >> 8,efuse_data_h & 0x00ff, (efuse_data_l & 0xff000000) >> 24,
            (efuse_data_l & 0x00ff0000) >> 16,(efuse_data_l & 0xff00) >> 8,efuse_data_l & 0xff);
        wrqu->data.length++;
    }
    return 0;
}

void aml_set_efuse_vendor_sn(struct net_device *dev, char *arg)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    char **argv;
    int argc;
    unsigned int efuse_data = 0;
    char sep = ':';

    efuse_data = _aml_get_efuse(aml_vif, EFUSE_BASE_0F);
    if (efuse_data != 0) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "efuse vendor SN(%02x:%02x) existed\n",
                (efuse_data & 0xff00) >> 8,
                efuse_data & 0x00ff);
        return;
    }

    if (strlen(arg) != strlen("00:00")) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "set efuse vendor SN(%s) illegality\n", arg);
        return;
    }

    argv = aml_cmd_char_phrase(sep, arg, &argc);
    if (argv) {
        efuse_data = ((simple_strtoul(argv[0], NULL, 16) << 8)
                | simple_strtoul(argv[1], NULL, 16));
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "set efuse vendor SN(%02x:%02x)\n",
                (efuse_data & 0xff00) >> 8,
                efuse_data & 0x00ff);
        _aml_set_efuse(aml_vif, EFUSE_BASE_0F, efuse_data);
    }
    kfree(argv);
}

int aml_get_efuse_vendor_sn(struct net_device *dev,
        union iwreq_data *wrqu, char *extra)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    unsigned int efuse_data = 0;

    efuse_data = _aml_get_efuse(aml_vif, EFUSE_BASE_0F);
    AML_PRINT(AML_DBG_MODULES_IWPRIV, "get efuse vendor SN(%02x:%02x)\n",
            (efuse_data & 0xff00) >> 8,
            efuse_data & 0x00ff);
    wrqu->data.length = scnprintf(extra, IW_PRIV_SIZE_MASK,
            "%02x:%02x\n", (efuse_data & 0xff00) >> 8,
            efuse_data & 0x00ff);
    wrqu->data.length++;

    return 0;
}

void aml_set_bt_mac_addr(struct net_device *dev, char* arg_iw)
{
    char **mac_cmd;
    int cmd_arg;
    char sep = ':';
    unsigned int mac_val = 0;
    unsigned int mac_val_second = 0;
    unsigned int efuse_data_l = 0;
    unsigned int efuse_data_h = 0;
    struct aml_vif *aml_vif = netdev_priv(dev);

    mac_val = _aml_get_efuse(aml_vif, EFUSE_BASE_01);
    mac_val_second = _aml_get_efuse(aml_vif, EFUSE_BASE_10);

    if (mac_val != 0 && mac_val_second != 0) {
        if (!aml_is_valid_mac_addr(arg_iw)) {
            mac_cmd = aml_cmd_char_phrase(sep, arg_iw, &cmd_arg);
            if (mac_cmd) {
                efuse_data_h = (simple_strtoul(mac_cmd[0], NULL,16) << 24) | (simple_strtoul(mac_cmd[1], NULL,16) << 16)
                               | (simple_strtoul(mac_cmd[2], NULL,16) << 8) | simple_strtoul(mac_cmd[3], NULL,16);
                efuse_data_l = (simple_strtoul(mac_cmd[4], NULL,16) << 24) | (simple_strtoul(mac_cmd[5], NULL,16) << 16);

                _aml_set_efuse(aml_vif, EFUSE_BASE_03, efuse_data_h);
                efuse_data_l = (efuse_data_l & 0xffff0000);
                _aml_set_efuse(aml_vif, EFUSE_BASE_02, efuse_data_l);

                AML_PRINT(AML_DBG_MODULES_IWPRIV, "iwpriv write BT MAC addr is:  %02x:%02x:%02x:%02x:%02x:%02x\n",
                    (efuse_data_h & 0xff000000) >> 24,(efuse_data_h & 0x00ff0000) >> 16,
                    (efuse_data_h & 0xff00) >> 8, efuse_data_h & 0xff,
                    (efuse_data_l & 0xff000000) >> 24, (efuse_data_l & 0x00ff0000) >> 16);
            }
            kfree(mac_cmd);
        }
    } else if ((mac_val != 0) && (mac_val_second == 0)) {
        if (!aml_is_valid_mac_addr(arg_iw)) {
            mac_cmd = aml_cmd_char_phrase(sep, arg_iw, &cmd_arg);
            if (mac_cmd) {
                mac_val = _aml_get_efuse(aml_vif, EFUSE_BASE_07);
                mac_val = mac_val | BIT16;
                _aml_set_efuse(aml_vif, EFUSE_BASE_07, mac_val);

                efuse_data_h = (simple_strtoul(mac_cmd[0], NULL,16) << 24) | (simple_strtoul(mac_cmd[1], NULL,16) << 16)
                               | (simple_strtoul(mac_cmd[2], NULL,16) << 8) | simple_strtoul(mac_cmd[3], NULL,16);
                efuse_data_l = (simple_strtoul(mac_cmd[4], NULL,16) << 24) | (simple_strtoul(mac_cmd[5], NULL,16) << 16);

                _aml_set_efuse(aml_vif, EFUSE_BASE_12, efuse_data_h);
                efuse_data_l = (efuse_data_l & 0xffff0000);
                _aml_set_efuse(aml_vif, EFUSE_BASE_11, efuse_data_l);

                AML_PRINT(AML_DBG_MODULES_IWPRIV, "iwpriv write second BT MAC addr is:  %02x:%02x:%02x:%02x:%02x:%02x\n",
                    (efuse_data_h & 0xff000000) >> 24,(efuse_data_h & 0x00ff0000) >> 16,
                    (efuse_data_h & 0xff00) >> 8, efuse_data_h & 0xff,
                    (efuse_data_l & 0xff000000) >> 24, (efuse_data_l & 0x00ff0000) >> 16);
            }
            kfree(mac_cmd);
        }
    } else {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "BT mac has been written\n");
    }
}

int aml_get_bt_mac_addr(struct net_device *dev,union iwreq_data *wrqu, char *extra)
{
    unsigned int efuse_data_l = 0;
    unsigned int efuse_data_h = 0;
    struct aml_vif *aml_vif = netdev_priv(dev);
    unsigned int efuse_data = 0;

    efuse_data_l = _aml_get_efuse(aml_vif, EFUSE_BASE_02);
    efuse_data_h = _aml_get_efuse(aml_vif, EFUSE_BASE_03);
    if ((((efuse_data_l >> 16) & 0xffff) == 0) && (efuse_data_h == 0)) {
        efuse_data_l = _aml_get_efuse(aml_vif, EFUSE_BASE_11);
        efuse_data_h = _aml_get_efuse(aml_vif, EFUSE_BASE_12);
    }
    if (efuse_data_l != 0 || efuse_data_h != 0) {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "BT MAC addr is: %02x:%02x:%02x:%02x:%02x:%02x\n",
            (efuse_data_h & 0xff000000) >> 24,(efuse_data_h & 0x00ff0000) >> 16,
            (efuse_data_h & 0xff00) >> 8, efuse_data_h & 0xff,
            (efuse_data_l & 0xff000000) >> 24, (efuse_data_l & 0x00ff0000) >> 16);

        wrqu->data.length = scnprintf(extra, IW_PRIV_SIZE_MASK, " %02x:%02x:%02x:%02x:%02x:%02x\n",
            (efuse_data_h & 0xff000000) >> 24,(efuse_data_h & 0x00ff0000) >> 16,
            (efuse_data_h & 0xff00) >> 8, efuse_data_h & 0xff,
            (efuse_data_l & 0xff000000) >> 24, (efuse_data_l & 0x00ff0000) >> 16);
        wrqu->data.length++;
    } else {
        AML_PRINT(AML_DBG_MODULES_IWPRIV, "No bt mac address is written into efuse!");
    }
    return 0;
}

#ifdef SDIO_SPEED_DEBUG
#define TEST_SDIO_ONLY
//#define SDIO_NORMAL_TX
#define SDIO_TX
//#define SDIO_SCATTER
#define DATA_LEN (1024*128)
static unsigned long payload_total = 0;
static unsigned char start_flag = 0;
static unsigned long in_time;
int g_test_times = 3;

void aml_datarate_monitor(void)
{
    static unsigned int sdio_speed = 0;

    if (time_after(jiffies, in_time + HZ)) {
        sdio_speed = payload_total >> 17;
        sdio_speed = (sdio_speed * HZ) / (jiffies - in_time);
        start_flag = 0;
        payload_total = 0;
        g_test_times--;
        AML_PRINT(AML_DBG_MODULES_IWPRIV, ">>>sdio_speed :%d mbps, time:%d\n", sdio_speed, (jiffies - in_time));
    }
}

int aml_sdio_max_speed_test_cmd(struct net_device *dev, int enable)
{
    struct aml_vif *aml_vif = netdev_priv(dev);
    struct aml_hw *aml_hw = aml_vif->aml_hw;
    struct amlw_hif_scatter_req * scat_req = NULL;
    unsigned char *data = kmalloc(DATA_LEN, GFP_ATOMIC);
    if (!data) {
        ASSERT_ERR(0);
        return -1;
    }
    memset(data, 0x66, DATA_LEN);

    scat_req = aml_hw->plat->hif_sdio_ops->hi_get_scatreq(&g_hwif_sdio);
    if (scat_req != NULL) {
        scat_req->req = HIF_WRITE | HIF_ASYNCHRONOUS;
        scat_req->addr = 0x0;
    }

    g_test_times = 3;
    while (enable && g_test_times) {
        if (!start_flag) {
            start_flag = 1;
            in_time = jiffies;
        }
#ifndef TEST_SDIO_ONLY
#ifdef SDIO_NORMAL_TX
        unsigned int i = 0;
        for (i = 0; i < DATA_LEN / 10240; ++i) {
            scat_req->scat_list[i].packet = data + 10240 * i;
            scat_req->scat_list[i].page_num = 10;
            scat_req->scat_list[i].len = 10240;
            scat_req->scat_count++;
            scat_req->len += scat_req->scat_list[i].len;
        }

        //AML_PRINT(AML_DBG_MODULES_IWPRIV, "count:%d, len:%d\n", scat_req->scat_count, scat_req->len);
        if (DATA_LEN % 10240) {
            scat_req->scat_list[i].packet = data + 10240 * i;
            scat_req->scat_list[i].page_num = ((DATA_LEN % 10240) % 1024) > 0 ? (DATA_LEN % 10240) / 1024 + 1: (DATA_LEN % 10240) / 1024;
            scat_req->scat_list[i].len = (DATA_LEN % 10240);
            scat_req->scat_count++;
            scat_req->len += scat_req->scat_list[i].len;
        }
        //AML_PRINT(AML_DBG_MODULES_IWPRIV, "count:%d, len:%d\n", scat_req->scat_count, scat_req->len);
        aml_hw->plat->hif_sdio_ops->hi_send_frame(scat_req);

        //ack irq
        aml_hw->plat->hif_sdio_ops->hi_tx_buffer_read((unsigned char *)data, (unsigned char *)0x60038000, 8);
        //read cfm
        aml_hw->plat->hif_sdio_ops->hi_tx_buffer_read((unsigned char *)data, (unsigned char *)0x60038000, 1028);
#else
        //rx
        aml_hw->plat->hif_sdio_ops->hi_rx_buffer_read((unsigned char *)aml_hw->host_buf,
            (unsigned char *)(unsigned long)RXBUF_START_ADDR, DATA_LEN, 0);
        aml_hw->plat->hif_sdio_ops->hi_random_word_write((unsigned int)(SYS_TYPE)(0x60038000), (unsigned int)1);
        //ack irq
        aml_hw->plat->hif_sdio_ops->hi_tx_buffer_read((unsigned char *)data, (unsigned char *)0x60038000, 8);
#endif

#else
#ifdef SDIO_TX
#ifndef SDIO_SCATTER
        //need delete base addr set procedure
        aml_hw->plat->hif_sdio_ops->hi_random_ram_write((unsigned char *)data, (unsigned char *)0x60038000, DATA_LEN);
        //aml_hw->plat->hif_sdio_ops->hi_random_word_write(0x00a070a0, 0x5678);// for revb irq trigger

#else
        unsigned int i = 0;
        for (i = 0; i < DATA_LEN / 10240; ++i) {
            scat_req->scat_list[i].packet = data + 10240 * i;
            scat_req->scat_list[i].page_num = 10;
            scat_req->scat_list[i].len = 10240;
            scat_req->scat_count++;
            scat_req->len += scat_req->scat_list[i].len;
        }

        if (DATA_LEN % 10240) {
            scat_req->scat_list[i].packet = data + 10240 * i;
            scat_req->scat_list[i].page_num = ((DATA_LEN % 10240) % 1024) > 0 ? (DATA_LEN % 10240) / 1024 + 1: (DATA_LEN % 10240) / 1024;
            scat_req->scat_list[i].len = (DATA_LEN % 10240);
            scat_req->scat_count++;
            scat_req->len += scat_req->scat_list[i].len;
        }
        aml_hw->plat->hif_sdio_ops->hi_send_frame(scat_req);
        //aml_hw->plat->hif_sdio_ops->hi_random_word_write(0x00a070a0, 0x5678);

#endif
#else
        aml_hw->plat->hif_sdio_ops->hi_tx_buffer_read((unsigned char *)data, (unsigned char *)0x60038000, DATA_LEN);
#endif
#endif
        payload_total += DATA_LEN;
        aml_datarate_monitor();
    }

    if (data) {
        FREE(data, "test data.");
        data = NULL;
    }
    return 0;
}
#endif

#if defined(CONFIG_WEXT_PRIV)
static int aml_iwpriv_send_para1(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
    int *param = (int *)extra;
    int sub_cmd = param[0];
    int set1 = param[1];

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "%s cmd:%d set1:%d\n", __func__, sub_cmd, set1);

    switch (sub_cmd) {
        case AML_IWP_SET_RATE_LEGACY_OFDM:
            aml_set_legacy_rate(dev, set1, 0);
            break;
        case AML_IWP_SET_SCAN_HANG:
            aml_iwpriv_set_scan_hang(dev, set1);
            break;
        case AML_IWP_SET_SCAN_TIME:
            aml_set_scan_time(dev, set1);
            break;
        case AML_IWP_SET_PS_MODE:
            aml_set_ps_mode(dev, set1);
            break;
        case AML_IWP_SET_AMSDU_MAX:
             aml_set_amsdu_max(dev, set1);
             break;
        case AML_IWP_SET_AMSDU_TX:
             aml_set_amsdu_tx(dev, set1);
             break;
        case AML_IWP_SET_LDPC:
             aml_set_ldpc(dev, set1);
             break;
        case AML_IWP_SET_P2P_OPPPS:
             aml_set_p2p_oppps(dev, set1);
             break;
        case AML_IWP_SET_TX_LFT:
             aml_set_tx_lft(dev, set1);
             break;
        case AML_IWP_SET_STBC:
            aml_set_stbc(dev, set1);
            break;
        case AML_IWP_SET_PT_CALIBRATION:
            aml_set_pt_calibration(dev, set1);
            break;
        case AML_PCIE_STATUS:
            aml_pcie_lp_switch(dev, set1);
            break;
        case AML_IWP_ENABLE_WF:
            aml_enable_wf(dev, set1);
            break;
#ifdef SDIO_SPEED_DEBUG
        case AML_IWP_ENABLE_SDIO_CAL_SPEED:
            aml_sdio_max_speed_test_cmd(dev, set1);
            break;
#endif
        case AML_IWP_SEND_TWT_TEARDOWN:
            aml_send_twt_teardown(dev, set1);
            break;
        case AML_IWP_SET_TX_MODE:
            aml_set_tx_mode(dev,set1);
            break;
        case AML_IWP_SET_TX_BW:
            aml_set_tx_bw(dev,set1);
            break;
        case AML_IWP_SET_TX_RATE:
            aml_set_tx_rate(dev,set1);
            break;
        case AML_IWP_SET_TX_LEN:
            aml_set_tx_len(dev,set1);
            break;
        case AML_IWP_SET_TX_PWR:
            aml_set_tx_pwr(dev,set1);
            break;
        case AML_IWP_SET_OLPC_PWR:
            aml_set_olpc_pwr(dev, set1);
            break;
        case AML_IWP_SET_XOSC_CTUNE:
            aml_set_xosc_ctune(dev, wrqu, extra, set1);
            break;
        case AML_IWP_SET_POWER_OFFSET:
            aml_set_power_offset(dev, wrqu, extra, set1);
            break;
        case AML_IWP_SET_RAM_EFUSE:
            aml_set_ram_efuse(dev, set1);
            break;
        case AML_IWP_SET_XOSC_EFUSE:
            aml_set_xosc_efuse(dev,set1);
            break;
        case AML_IWP_SET_TXPAGE_ONCE:
            aml_set_txpage_once(dev,set1);
            break;
        case AML_IWP_SET_TXCFM_TRI_TX:
            aml_set_txcfm_tri_tx(dev,set1);
            break;
#ifdef CONFIG_SDIO_TX_ENH
        case AML_IWP_SET_TXCFM_READ_THR:
            aml_set_txcfm_read_thresh(dev,set1);
            break;
        case AML_IWP_SET_DYN_TXCFM:
            aml_set_dyn_txcfm(dev,set1);
            break;
        case AML_IWP_SET_IRQLESS_FLAG:
            aml_set_irqless_flag(dev,set1);
            break;
#endif
        case AML_IWP_RECY_CTRL:
            aml_recy_ctrl(dev, set1);
            break;
        case AML_IWP_SET_LIMIT_POWER:
            aml_set_limit_power_status(dev, set1);
            break;
        case AML_IWP_SET_TSQ:
            aml_set_tsq(dev, set1);
            break;
        case AML_IWP_SET_MAX_DROP_NUM:
            aml_set_max_drop_num(dev, set1);
            break;
        case AML_IWP_SET_MAX_TIMEOUT:
            aml_set_max_timeout(dev, set1);
            break;
        case AML_IWP_SET_BUF_STATUS:
            aml_set_buf_state(dev, set1);
            break;
#ifdef CONFIG_AML_NAPI
        case AML_IWP_SET_NAPI:
            aml_set_napi_enable(dev, set1);
            break;
        case AML_IWP_SET_GRO:
            aml_set_gro_enable(dev, set1);
             break;
        case AML_IWP_SET_NAPI_NUM:
            aml_set_napi_num(dev, set1);
             break;
#endif
        case AML_IWP_SET_BUS_TIMEOUT_TEST:
            aml_set_bus_timeout_test(dev, set1);
            break;
        case AML_IWP_SET_TX_THS:
            aml_set_txdesc_trigger_ths(dev, set1);
            break;
        case AML_IWP_FIX_TX_PWR:
            aml_fix_tx_power(dev, set1);
            break;
        case AML_IWP_GET_FW_LOG:
            aml_set_fwlog_cmd(dev, set1);
            break;
        default:
            AML_PRINT(AML_DBG_MODULES_IWPRIV, "%s %d: param err\n", __func__, __LINE__);
            break;
    }

    return 0;
}

static int aml_iwpriv_send_para2(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
    int *param = (int *)extra;
    int sub_cmd = param[0];
    int set1 = param[1];
    int set2 = param[2];

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "%s cmd:%d set1:%d set2:%d\n", __func__, sub_cmd, set1, set2);

    switch (sub_cmd) {
        case AML_IWP_SET_RATE_LEGACY_CCK:
            aml_set_legacy_rate(dev, set1, set2);
            break;
        case AML_IWP_SET_RF_REG:
            aml_rf_reg_write(dev, set1, set2);
            break;
        case AML_IWP_SET_REG:
            aml_set_reg(dev, set1, set2);
            break;
        case AML_IWP_SET_EFUSE:
            aml_set_efuse(dev, set1, set2);
            break;
        case AML_MEM_DUMP:
            aml_dump_reg(dev, set1, set2);
            break;
        case AML_IWP_SET_RX:
            aml_set_rx(dev, set1, set2);
            break;
        case AML_IWP_SET_TX_PROT:
            aml_set_tx_prot(dev, set1, set2);
            break;
        case AML_IWP_SET_TX_PATH:
            aml_set_tx_path(dev, set1, set2);
            break;
        case AML_IWP_START_CAP:
            aml_emb_la_capture(dev, set1, set2);
            break;
        case AML_IWP_SNR_CFG:
            aml_dyn_snr_cfg(dev, set1, set2);
            break;
        case AML_IWP_SET_DC_TONE:
            aml_set_tone(dev, set1, set2);
            break;
        case AML_IWP_SET_TCP_DELAY_ACK:
            aml_set_tcp_delay_ack(dev, set1,set2);
            break;
        default:
            break;
    }

    return 0;
}

static int aml_iwpriv_send_para3(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
    int *param = (int *)extra;
    int sub_cmd = param[0];
    int set1 = param[1];
    int set2 = param[2];
    int set3 = param[3];

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "%s cmd:%d set1:%d set2:%d set3:%d \n", __func__, sub_cmd, set1, set2, set3);

    switch (sub_cmd) {
        case AML_IWP_SET_RATE_HT:
        case AML_IWP_SET_RATE_VHT:
        case AML_IWP_SET_RATE_HE:
            aml_set_mcs_fixed_rate(dev, (enum aml_iwpriv_subcmd)sub_cmd, set1, set2, set3);
            break;
        case AML_IWP_SET_P2P_NOA:
            aml_set_p2p_noa(dev, set1, set2, set3);
            break;
#ifdef TEST_MODE
        case AML_IWP_PCIE_TEST:
            aml_pcie_prssr_test(dev, set1, set2, set3);
            break;
#endif
        case AML_COEX_CMD:
            aml_coex_cmd(dev, set1, set2, set3);
            break;
        default:
            break;
    }

    return 0;
}

static int aml_iwpriv_send_para4(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
    int *param = (int *)extra;
    int sub_cmd = param[0];
    int set1 = param[1];
    int set2 = param[2];
    int set3 = param[3];
    int set4 = param[4];

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "%s cmd:%d set1:%d set2:%d set3:%d set4:%d\n", __func__, sub_cmd, set1, set2, set3, set4);

    switch (sub_cmd) {
        case AML_IWP_SET_MACBYPASS:
            aml_set_macbypass(dev, set1, set2, set3, set4);
            break;
        default:
            break;
    }

    return 0;
}

static int aml_iwpriv_get(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
    int *param = (int *)extra;
    int sub_cmd = param[0];
    /*if we need feed back the value to user space, we need these 2 lines code, this is a sample*/
    //wrqu->data.length = sizeof(int);
    //*param = 110;

    switch (sub_cmd) {
        case AML_IWP_PRINT_VERSION:
            aml_get_version();
            break;
        case AML_IWP_SET_RATE_AUTO:
            aml_set_fixed_rate(dev, RC_AUTO_RATE_INDEX);
            break;
        case AML_IWP_GET_RATE_INFO:
            aml_get_rate_info(dev);
            break;
        case AML_IWP_GET_STATS:
            aml_get_tx_stats(dev);
            break;
        case AML_IWP_GET_ACS_INFO:
            aml_get_acs_info(dev);
            break;
        case AML_IWP_GET_AMSDU_MAX:
            aml_get_amsdu_max(dev);
            break;
        case AML_IWP_GET_AMSDU_TX:
            aml_get_amsdu_tx(dev);
            break;
        case AML_IWP_GET_LDPC:
            aml_get_ldpc(dev);
            break;
        case AML_IWP_GET_TXQ:
            aml_get_txq(dev);
            aml_txq_unexpection(dev);
            break;
        case AML_IWP_GET_TX_LFT:
            aml_get_tx_lft(dev);
            break;
        case AML_IWP_GET_LAST_RX:
            aml_get_last_rx(dev);
            break;
        case AML_IWP_CLEAR_LAST_RX:
            aml_clear_last_rx(dev);
            break;
        case AML_IWP_SET_STOP_MACBYPASS:
            aml_set_stop_macbypass(dev);
            break;
        case AML_IWP_GET_STBC:
            aml_get_stbc(dev);
            break;
         case AML_LA_DUMP:
            aml_emb_la_dump(dev);
            break;
         case AML_IWP_CCA_CHECK:
            aml_cca_check(dev);
            break;
         case AML_IWP_GET_CHAN_LIST:
            aml_get_chan_list_info(dev);
            break;
        case AML_IWP_GET_CLK:
           aml_get_clock(dev);
           break;
        if (aml_bus_type != PCIE_MODE) {
            case AML_IWP_BUS_START_TEST:
                aml_sdio_start_test(dev);
                break;
        }
        case AML_IWP_GET_MSGIND:
            aml_get_proc_msg(dev);
            break;
        case AML_IWP_GET_RXIND:
            aml_get_proc_rxbuff(dev);
            break;
        case AML_IWP_SET_RX_START:
            aml_set_rx_start(dev);
            break;
        case AML_IWP_SET_OFFSET_POWER_VLD:
            aml_set_offset_power_vld(dev);
            break;
        case AML_IWP_GET_BUF_STATE:
            aml_get_buf_state(dev);
            break;
        case AML_IWP_GET_TCP_DELAY_ACK_INFO:
            aml_get_tcp_ack_info(dev);
            break;
        case AML_IWP_LA_ENABLE:
            aml_emb_la_enable(dev);
            break;
        case AML_IWP_STOP_DC_TONE:
            aml_stop_dc_tone(dev);
            break;
#ifdef CONFIG_SDIO_TX_ENH
#ifdef SDIO_TX_ENH_DBG
        case AML_IWP_GET_SDIO_TX_ENH_STATS:
            aml_get_sdio_tx_enh_stats(dev);
            break;
        case AML_IWP_RESET_SDIO_TX_ENH_STATS:
            aml_reset_sdio_tx_enh_stats(dev);
            break;
#endif
#endif

        default:
            AML_PRINT(AML_DBG_MODULES_IWPRIV, "%s %d param err\n", __func__, __LINE__);
            break;
    }

    return 0;
}


static int aml_iwpriv_get_char(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
    int sub_cmd = wrqu->data.flags;

    char set[MAX_CHAR_SIZE];

    if (wrqu->data.length > 0) {
        if (copy_from_user(set,
            wrqu->data.pointer, wrqu->data.length)) {
            return -EFAULT;
        }
        set[wrqu->data.length] = '\0';
    }

    switch (sub_cmd) {
        case AML_IWP_GET_REG:
            aml_get_reg(dev, set, wrqu, extra);
            break;
        case AML_IWP_GET_RF_REG:
            aml_get_rf_reg(dev, set, wrqu, extra);
            break;
        case AML_IWP_SET_DEBUG:
            aml_set_debug(dev, set);
            break;
        case AML_IWP_SEND_TWT_SETUP_REQ:
            aml_send_twt_req(dev, set, wrqu, extra);
            break;
        case AML_IWP_GET_EFUSE:
            aml_get_efuse(dev, set, wrqu, extra);
            break;
        case AML_IWP_SET_WIFI_MAC_EFUSE:
            aml_set_wifi_mac_addr(dev, set);
            break;
        case AML_IWP_SET_RX_END:
            aml_set_rx_end(dev,wrqu, extra);
            break;
        case AML_IWP_GET_WIFI_MAC_FROM_EFUSE:
            aml_get_mac_addr(dev,wrqu, extra);
            break;
        case AML_IWP_SET_BT_MAC_EFUSE:
            aml_set_bt_mac_addr(dev, set);
            break;
        case AML_IWP_GET_BT_MAC_FROM_EFUSE:
            aml_get_bt_mac_addr(dev,wrqu, extra);
            break;
        case AML_IWP_GET_XOSC_OFFSET:
            aml_get_xosc_offset(dev,wrqu, extra);
            break;
        case AML_IWP_GET_XOSC_EFUSE_TIMES:
            aml_get_xosc_efuse_times(dev, wrqu, extra);
            break;
        case AML_IWP_GET_MAC_TIMES:
            aml_get_mac_efuse_times(dev, wrqu, extra);
            break;
        case AML_IWP_GET_ALL_EFUSE:
            aml_get_all_efuse(dev,wrqu, extra);
            break;
        case AML_IWP_SET_TX_END:
            aml_set_tx_end(dev, wrqu, extra);
            break;
        case AML_IWP_SET_TX_START:
            aml_set_tx_start(dev, wrqu, extra);
            break;
         case AML_IWP_SET_CSI:
            aml_set_csi(dev, set);
            break;
        case AML_IWP_SET_EFUSE_VENDOR_SN:
            aml_set_efuse_vendor_sn(dev, set);
            break;
        case AML_IWP_GET_EFUSE_VENDOR_SN:
            aml_get_efuse_vendor_sn(dev, wrqu, extra);
            break;
        case AML_IWP_SET_EARLY_BEACON:
            aml_set_early_bcn_mode(dev, set, wrqu, extra);
            break;
        default:
            break;
    }

    return 0;
}

static int aml_iwpriv_get_int(struct net_device *dev,
    struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
    int sub_cmd = wrqu->data.flags;

    switch (sub_cmd) {
        case AML_IWP_GET_CSI_STATUS_COM:
            aml_get_csi_status_com(dev, wrqu);
            break;
        case AML_IWP_GET_CSI_STATUS_SP:
            aml_get_csi_status_sp(dev, wrqu, extra);
            break;
        default:
            break;
    }

    return 0;
}

int iw_standard_set_mode(struct net_device *dev, struct iw_request_info *info,
    union iwreq_data *wrqu, char *extra)
{
    struct aml_vif *vif = netdev_priv(dev);
    struct aml_hw *aml_hw = vif->aml_hw;

    AML_PRINT(AML_DBG_MODULES_IWPRIV, "%s:%d param:%d", __func__, __LINE__, wrqu->param.value);
    aml_cfg80211_change_iface(aml_hw->wiphy, dev,
            (enum nl80211_iftype)wrqu->param.value, NULL);

    return 0;
}

static const iw_handler standard_handler[] = {
    IW_HANDLER(SIOCSIWMODE,    (iw_handler)iw_standard_set_mode),
    NULL,
};

static iw_handler aml_iwpriv_private_handler[] = {
    /*if we need feed back the value to user space, we need jump command for large buffer*/
    aml_iwpriv_get,
    aml_iwpriv_send_para1,
    aml_iwpriv_send_para2,
    aml_iwpriv_send_para3,
    NULL,
    aml_iwpriv_get_char,
    aml_iwpriv_get_int,
    aml_iwpriv_send_para4,
    NULL,
};

static const struct iw_priv_args aml_iwpriv_private_args[] = {
    {
        /*if we need feed back the value to user space, we need jump command for large buffer*/
        SIOCIWFIRSTPRIV,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, ""},
    {
        AML_IWP_PRINT_VERSION,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_drv_ver"},
    {
        AML_IWP_SET_RATE_AUTO,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "set_rate_auto"},
    {
        AML_IWP_RECY_CTRL,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "recy_ctrl"},
    {
        AML_IWP_GET_RATE_INFO,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_rate_info"},
    {
        AML_IWP_GET_STATS,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_stats"},
    {
        AML_IWP_GET_ACS_INFO,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_acs"},
    {
        AML_IWP_GET_AMSDU_MAX,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_amsdu_max"},
    {
        AML_IWP_GET_AMSDU_TX,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_amsdu_tx"},
    {
        AML_IWP_GET_LDPC,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_ldpc"},
    {
        AML_IWP_GET_TXQ,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_txq"},
    {
        AML_IWP_GET_TX_LFT,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_tx_lft"},
    {
        AML_IWP_GET_LAST_RX,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_last_rx"},
    {
        AML_IWP_CLEAR_LAST_RX,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "clear_last_rx"},
    {
        AML_IWP_SET_STOP_MACBYPASS,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "stop_macbypass"},
    {
        AML_IWP_GET_STBC,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_stbc"},
    {
        AML_LA_DUMP,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "la_dump"},
    {
        AML_IWP_CCA_CHECK,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "cca_check"},
    {
        AML_IWP_BUS_START_TEST,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "usb_start_test"},
    {
        AML_IWP_BUS_START_TEST,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "sdio_start_test"},
    {
        AML_IWP_GET_MSGIND,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_msgind"},
    {
        AML_IWP_GET_RXIND,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_rxind"},
    {
        AML_IWP_SET_RX_START,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "pt_rx_start"},
    {
        AML_IWP_SET_OFFSET_POWER_VLD,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "offset_pow_vld"},
    {
        AML_IWP_GET_BUF_STATE,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_buf_state"},
    {
        AML_IWP_GET_TCP_DELAY_ACK_INFO,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_tcp_info"},
    {
        AML_IWP_LA_ENABLE,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "la_enable"},
    {
        AML_IWP_STOP_DC_TONE,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "stop_dc_tone"},
    {
        SIOCIWFIRSTPRIV + 1,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, ""},
    {
        AML_IWP_SET_RATE_LEGACY_OFDM,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_rate_ofdm"},
    {
        AML_IWP_SET_SCAN_HANG,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_sc_hang"},
    {
        AML_IWP_SET_SCAN_TIME,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_sc_time"},
    {
        AML_IWP_SET_PS_MODE,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_ps_mode"},
    {
        AML_IWP_SET_AMSDU_MAX,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_amsdu_max"},
    {
        AML_IWP_SET_AMSDU_TX,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_amsdu_tx"},
    {
        AML_IWP_SET_LDPC,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_ldpc"},
    {
        AML_IWP_SET_P2P_OPPPS,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_p2p_oppps"},
    {
        AML_IWP_SET_TX_LFT,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_tx_lft"},
    {
        AML_IWP_SET_STBC,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_stbc"},
    {
        AML_IWP_SET_PT_CALIBRATION,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_pt_cali"},
    {
        AML_PCIE_STATUS,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_pcie_status"},
    {
        AML_IWP_ENABLE_WF,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "enable_wf"},
#ifdef SDIO_SPEED_DEBUG
    {
        AML_IWP_ENABLE_SDIO_CAL_SPEED,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "sdio_speed"},
#endif
    {
        AML_IWP_SEND_TWT_TEARDOWN,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "send_twt_td"},
    {
        AML_IWP_SET_TX_MODE,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_tx_mode"},
    {
        AML_IWP_SET_TX_BW,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_tx_bw"},
    {
        AML_IWP_SET_TX_RATE,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_tx_rate"},
    {
        AML_IWP_SET_TX_LEN,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_tx_len"},
    {
        AML_IWP_SET_TX_PWR,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_tx_pwr"},
    {
        AML_IWP_SET_OLPC_PWR,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_olpc_pwr"},
    {
        AML_IWP_SET_XOSC_CTUNE,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_xosc_ctune"},
    {
        AML_IWP_SET_POWER_OFFSET,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_pwr_ofset"},
    {
        AML_IWP_SET_RAM_EFUSE,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_ram_efuse"},
    {
        AML_IWP_SET_XOSC_EFUSE,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_xosc_efuse"},
    {
        AML_IWP_SET_TX_FRAME_DELAY,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_frame_delay"},
    {
        AML_IWP_SET_TXPAGE_ONCE,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_txpage_once"},
    {
        AML_IWP_SET_TXCFM_TRI_TX,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_tri_tx_thr"},
    {
        AML_IWP_SET_LIMIT_POWER,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_limit_power"},
    {
        AML_IWP_SET_TSQ,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_tsq"},
    {
        AML_IWP_SET_MAX_DROP_NUM,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_drop_num"},
    {
        AML_IWP_SET_MAX_TIMEOUT,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_ack_to"},
    {
        AML_IWP_SET_BUF_STATUS,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_buf_state"},
#ifdef CONFIG_AML_NAPI
    {
        AML_IWP_SET_NAPI,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_napi"},
    {
        AML_IWP_SET_GRO,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_gro"},
    {
        AML_IWP_SET_NAPI_NUM,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_napi_num"},
#endif
    {
        AML_IWP_SET_BUS_TIMEOUT_TEST,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_bus_timeout"},
    {
        AML_IWP_SET_TX_THS,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_tx_ths"},
    {
        AML_IWP_FIX_TX_PWR,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,  "fix_txpwr"},
    {
        AML_IWP_GET_FW_LOG,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "get_fw_log"},
    {
        SIOCIWFIRSTPRIV + 2,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, ""},
    {
        AML_IWP_SET_RATE_LEGACY_CCK,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "set_rate_cck"},
    {
        AML_IWP_SET_RF_REG,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "set_rf_reg"},
    {
        AML_IWP_SET_REG,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "set_reg"},
    {
        AML_IWP_SET_EFUSE,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "set_efuse"},
    {
        AML_MEM_DUMP,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "mem_dump"},
    {
        AML_IWP_SET_RX,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "pt_set_rx"},
    {
        AML_IWP_SET_TX_PROT,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "set_tx_prot"},
    {
        AML_IWP_SET_TX_PATH,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "set_tx_path"},
    {
        AML_IWP_START_CAP,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "la_capture"},
    {
        AML_IWP_SNR_CFG,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "dyn_snr_cfg"},
    {
        AML_IWP_SET_DC_TONE,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "send_dc_tone"},
    {
        AML_IWP_SET_TCP_DELAY_ACK,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "set_delay_ack"},
    {
        SIOCIWFIRSTPRIV + 3,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3, 0, ""},
    {
        AML_IWP_SET_RATE_HT,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3, 0, "set_rate_ht"},
    {
        AML_IWP_SET_RATE_VHT,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3, 0, "set_rate_vht"},
    {
        AML_IWP_SET_RATE_HE,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3, 0, "set_rate_he"},
    {
        AML_IWP_SET_P2P_NOA,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3, 0, "set_p2p_noa"},
#ifdef TEST_MODE
    {
        AML_IWP_PCIE_TEST,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3, 0, "pcie_test"},
#endif
    {
        AML_COEX_CMD,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3, 0, "coex_cmd"},
    {
        SIOCIWFIRSTPRIV + 5,
        IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, ""},
    {
        AML_IWP_GET_REG,
        IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "get_reg"},
    {
        AML_IWP_GET_RF_REG,
        IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "get_rf_reg"},
    {
        AML_IWP_SET_DEBUG,
        IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "set_debug"},
    {
        AML_IWP_SEND_TWT_SETUP_REQ,
        IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "send_twt_req"},
    {
        AML_IWP_SET_EARLY_BEACON,
        IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "set_early_bcn"},
    {
        AML_IWP_GET_EFUSE,
        IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "get_efuse"},
    {
        AML_IWP_SET_WIFI_MAC_EFUSE,
        IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "set_wifi_mac"},
    {
        AML_IWP_GET_WIFI_MAC_FROM_EFUSE,
        IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "get_wifi_mac"},
    {
        AML_IWP_SET_BT_MAC_EFUSE,
        IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "set_bt_mac"},
    {
        AML_IWP_GET_BT_MAC_FROM_EFUSE,
        IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "get_bt_mac"},
    {
        AML_IWP_SET_RX_END,
        IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "pt_rx_end"},
    {
        AML_IWP_GET_XOSC_OFFSET,
        IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "get_xosc_offset"},
    {
        AML_IWP_GET_ALL_EFUSE,
        IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "get_all_efuse"},
    {
        AML_IWP_GET_XOSC_EFUSE_TIMES,
        IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "get_xosc_times"},
    {
        AML_IWP_GET_MAC_TIMES,
        IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "get_mac_times"},
    {
        AML_IWP_SET_TX_START,
        IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "pt_tx_start"},
    {
        AML_IWP_SET_TX_END,
        IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "pt_tx_end"},
    {
        AML_IWP_SET_CSI,
        IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "set_csi"},
    {
        SIOCIWFIRSTPRIV + 6,
        IW_PRIV_TYPE_INT | IW_PRIV_INT_SIZE_MASK, IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_MASK, ""},
    {
        AML_IWP_GET_CSI_STATUS_COM,
        IW_PRIV_TYPE_INT | IW_PRIV_INT_SIZE_MASK, IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_MASK, "get_csi_com"},
    {
        AML_IWP_GET_CSI_STATUS_SP,
        IW_PRIV_TYPE_INT | IW_PRIV_INT_SIZE_MASK, IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_MASK, "get_csi_sp"},
    {
        AML_IWP_GET_CSI,
        IW_PRIV_TYPE_INT | IW_PRIV_INT_SIZE_MASK, IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_MASK, "get_csi"},
    {
        SIOCIWFIRSTPRIV + 7,
        IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | 4, 0, ""},
    {
        AML_IWP_SET_MACBYPASS,
        IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 4, 0, "set_macbypass"},
    {
        AML_IWP_GET_CHAN_LIST,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_chan_list"},
    {
        AML_IWP_GET_CLK,
        0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "clk_msr"},
#ifdef CONFIG_SDIO_TX_ENH
#ifdef SDIO_TX_ENH_DBG
        {
            AML_IWP_GET_SDIO_TX_ENH_STATS,
            0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_txenh_log"},
        {
            AML_IWP_RESET_SDIO_TX_ENH_STATS,
            0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "reset_txenh_log"},
#endif
        {
            AML_IWP_SET_TXCFM_READ_THR,
            IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_tcrd_thr"},
        {
            AML_IWP_SET_DYN_TXCFM,
            IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_dyn_txcfm"},
        {
            AML_IWP_SET_IRQLESS_FLAG,
            IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "set_irqless"},
#endif
        {
            AML_IWP_SET_EFUSE_VENDOR_SN,
            IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK,
            "set_vendor_sn"},
        {
            AML_IWP_GET_EFUSE_VENDOR_SN,
            IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK,
            "get_vendor_sn"},
};
#endif


struct iw_handler_def iw_handle = {
#if defined(CONFIG_WEXT_PRIV)
    .num_standard = sizeof(standard_handler) / sizeof(standard_handler[0]),
    .num_private = ARRAY_SIZE(aml_iwpriv_private_handler),
    .num_private_args = ARRAY_SIZE(aml_iwpriv_private_args),
    .standard = (iw_handler *)standard_handler,
    .private = aml_iwpriv_private_handler,
    .private_args = aml_iwpriv_private_args,
#endif
};

