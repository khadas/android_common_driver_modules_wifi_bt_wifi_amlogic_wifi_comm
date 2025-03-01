#ifndef _SHARE_MEM_MAP_H_
#define _SHARE_MEM_MAP_H_

#define _SSHRAM (0x60000000)
#define IPC_SHARED_ENV (0x60000000)
#define DEBUG_INFO (0x60000b64)
#define BCN_DWNLD_DESC (0x60001188)
#define TX_HW_DESC (0x60001198)
#define RADAR_EVENT_DESC_ARRAY (0x60006418)
#define RXL_HW_BUFFER1 (0x60006530)
#define RXL_HW_BUFFER2 (0x60006534)
#define SCAN_PROBE_REQ_IE (0x60006538)
#define TX_AGG_DESC_ARRAY5 (0x60006678)
#define TX_AGG_DESC_ARRAY4 (0x60006be8)
#define TX_AGG_DESC_ARRAY3 (0x60006ffc)
#define TX_AGG_DESC_ARRAY2 (0x60007adc)
#define TX_AGG_DESC_ARRAY1 (0x600085bc)
#define TX_AGG_DESC_ARRAY0 (0x6000909c)
#define TXL_BUFFER_CONTROL_DESC_BCMC (0x60009b7c)
#define TXL_BUFFER_CONTROL_DESC (0x60009c7c)
#define TXL_BUFFER_DMA_LATE_LOC (0x6000a07c)
#define TXL_BUFFER_HW_DESC (0x6000a080)
#define TXL_BUFFER_POOL (0x6000a0e4)
#define TXL_SPEC_FRAME_BUFFER_CONTROL (0x6000a0f8)
#define TXL_BUFFER_CONTROL_NDP (0x6000a138)
#define TXL_BUFFER_CONTROL_NDPA_BRP (0x6000a178)
#define TXL_BUFFER_CONTROL_5G (0x6000a1b8)
#define TXL_BUFFER_CONTROL_24G (0x6000a1f8)
#define TXL_FRAME_HWDESC_POOL (0x6000a238)
#define TXL_P2P_NOA_IE_POOL (0x6000a3c8)
#define TXL_P2P_NOA_DESC (0x6000a460)
#define TXL_BCN_BUF_CTRL (0x6000a4b0)
#define TXL_BCN_END_DESC (0x6000a5b0)
#define TXL_BCN_HWDESC_POOL (0x6000a600)
#define TXL_BCN_POOL (0x6000a740)
#define TXL_TIM_DESC (0x6000b330)
#define TXL_TIM_BITMAP_POOL (0x6000b3d0)
#define TXL_TIM_IE_POOL (0x6000b7c0)
#define TXL_FRAME_POOL (0x6000b7e0)
#define HE_TB_DESC (0x6000c44c)
#define HSU_BUF (0x6000c914)
#define MIC_AAD (0x6000cb14)
#define MFP_AAD (0x6000cb24)
#define SCANU_ADD_IE (0x6000cb38)
#define E2A_MSG_EXT_BUF (0x6000cc00)
#define TRACE_COMPO_LEVEL (0x6000d00c)
#define _SHAREDRAM_TXRX (0x6000d07c)
#define _ESHRAM (0x60080000)

#endif//_SHARE_MEM_MAP_H_
