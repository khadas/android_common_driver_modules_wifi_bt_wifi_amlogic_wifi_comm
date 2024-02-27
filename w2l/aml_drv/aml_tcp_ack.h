/**
****************************************************************************************
*
* @file aml_tcp_ack.h
*
* Copyright (C) Amlogic, Inc. All rights reserved (2022-2023).
*
* @brief Declaration of the tcp ack.
*
****************************************************************************************
*/
#include "aml_defs.h"

#ifndef __AML_TCP_ACK_H__
#define __AML_TCP_ACK_H__

#define AML_TCP_SESS_NUM  128
#define MAX_TCP_ACK_LEN 200
#define MAX_DROP_TCP_ACK_CNT  24
#define MAX_DROP_TCP_ACK_CNT_SDIO  24
#define MAX_DROP_TCP_ACK_CNT_USB  10
#define TCK_SESS_TIMEOUT_TIME 5000
#define MAX_TCP_ACK_TIMEOUT 30 //ms

#define MIN_WIN 96
#define SIZE_KB 1024

#define AML_U32_BEFORE(a, b) ((__s32)((__u32)a - (__u32)b) <= 0)

struct aml_tcp_ack_tx {
    struct aml_sta *sta;
    struct aml_vif *aml_vif;
    struct sk_buff *skb;
};


struct aml_pkt_info {
    u16 source;
    u16 dest;
    u32 saddr;
    u32 daddr;
    u32 seq;
    u16 win;
};

struct aml_tcp_sess_info {
    u8 index;
    u8 busy;
    /*tcp drop count*/
    u8 drop_cnt;
    u16 win_scale;
    seqlock_t seqlock;
    unsigned long last_time;
    /*tcp session ageout time*/
    unsigned long timeout;
    struct timer_list timer;
    struct sk_buff *skb;
    struct sk_buff *in_txq_skb;
    struct aml_sta *sta;
    struct aml_vif *aml_vif;
    struct aml_pkt_info pkt_info;
};

struct aml_tcp_sess_mgr {
    atomic_t enable;
    atomic_t dynamic_adjust;
    int used_num;
    int free_index;
    u32 total_drop_cnt;
    unsigned long last_time;
    /*tcp session ageout time*/
    unsigned long timeout;
    /*max tcp ack drop count*/
    atomic_t max_drop_cnt;
    /*tcp ack timeout time*/
    atomic_t max_timeout;
    spinlock_t lock;
    struct aml_hw *aml_hw;
    struct aml_tcp_sess_info tcp_info[AML_TCP_SESS_NUM];
    unsigned int ack_winsize;
    unsigned short win_scale;
};

void aml_tcp_delay_ack_deinit(struct aml_hw *aml_hw);
void aml_tcp_delay_ack_init(struct aml_hw *aml_hw);
int aml_filter_tx_tcp_ack(struct net_device *dev, struct sk_buff *skb, struct aml_sta *sta);
void aml_check_tcpack_skb(struct aml_hw *aml_hw, struct sk_buff *skb, u32 len);

#endif
