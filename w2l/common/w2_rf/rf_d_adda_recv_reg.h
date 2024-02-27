#ifdef RF_D_ADDA_RECV_REG
#else
#define RF_D_ADDA_RECV_REG

#define RG_RECV_A0                                (0x0)
// Bit 31  :0      rg_recv_cfg0                   U     RW        default = 'hf
typedef union RG_RECV_A0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_recv_cfg0 : 32;
  } b;
} RG_RECV_A0_FIELD_T;

#define RG_RECV_A1                                (0x4)
// Bit 31  :0      rg_recv_cfg1                   U     RW        default = 'h0
typedef union RG_RECV_A1_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_recv_cfg1 : 32;
  } b;
} RG_RECV_A1_FIELD_T;

#define RG_RECV_A2                                (0x8)
// Bit 0           rg_rxneg_msb                   U     RW        default = 'h0
// Bit 1           rg_rxiq_swap                   U     RW        default = 'h0
// Bit 20          rg_dcloop_bypass               U     RW        default = 'h1
// Bit 24          rg_rx_adc_fs                   U     RW        default = 'h1
// Bit 28          rg_to_agc_en                   U     RW        default = 'h1
typedef union RG_RECV_A2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxneg_msb : 1;
    unsigned int rg_rxiq_swap : 1;
    unsigned int rsvd_0 : 18;
    unsigned int rg_dcloop_bypass : 1;
    unsigned int rsvd_1 : 3;
    unsigned int rg_rx_adc_fs : 1;
    unsigned int rsvd_2 : 3;
    unsigned int rg_to_agc_en : 1;
    unsigned int rsvd_3 : 3;
  } b;
} RG_RECV_A2_FIELD_T;

#define RG_RECV_A11                               (0x2c)
// Bit 4           rg_rxirr_bypass                U     RW        default = 'h1
// Bit 11  :8      rg_rx_async_wgap               U     RW        default = 'ha
typedef union RG_RECV_A11_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rsvd_0 : 4;
    unsigned int rg_rxirr_bypass : 1;
    unsigned int rsvd_1 : 3;
    unsigned int rg_rx_async_wgap : 4;
    unsigned int rsvd_2 : 20;
  } b;
} RG_RECV_A11_FIELD_T;

#define RG_RECV_A12                               (0x30)
// Bit 24  :0      rg_rxddc_angle                 U     RW        default = 'h0
// Bit 28          rg_rxddc_phs_reset             U     RW        default = 'h0
// Bit 31          rg_rxddc_bypass                U     RW        default = 'h1
typedef union RG_RECV_A12_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxddc_angle : 25;
    unsigned int rsvd_0 : 3;
    unsigned int rg_rxddc_phs_reset : 1;
    unsigned int rsvd_1 : 2;
    unsigned int rg_rxddc_bypass : 1;
  } b;
} RG_RECV_A12_FIELD_T;

#define RG_RECV_A13                               (0x34)
// Bit 9   :0      rg_dc_sub_thr_pow              U     RW        default = 'h74
typedef union RG_RECV_A13_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_dc_sub_thr_pow : 10;
    unsigned int rsvd_0 : 22;
  } b;
} RG_RECV_A13_FIELD_T;

#define RG_RECV_A14                               (0x38)
// Bit 9   :0      rg_hdmi_sub_thr_pow1           U     RW        default = 'h70
// Bit 21  :12     rg_hdmi_sub_thr_pow2           U     RW        default = 'h68
typedef union RG_RECV_A14_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_hdmi_sub_thr_pow1 : 10;
    unsigned int rsvd_0 : 2;
    unsigned int rg_hdmi_sub_thr_pow2 : 10;
    unsigned int rsvd_1 : 10;
  } b;
} RG_RECV_A14_FIELD_T;

#define RG_RECV_A15                               (0x3c)
// Bit 9   :0      rg_cw_sub_thr_pow1             U     RW        default = 'h78
// Bit 21  :12     rg_cw_sub_thr_pow2             U     RW        default = 'h66
typedef union RG_RECV_A15_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_cw_sub_thr_pow1 : 10;
    unsigned int rsvd_0 : 2;
    unsigned int rg_cw_sub_thr_pow2 : 10;
    unsigned int rsvd_1 : 10;
  } b;
} RG_RECV_A15_FIELD_T;

#define RG_RECV_A16                               (0x40)
// Bit 0           rg_cw_remove_sub_man_bypass0     U     RW        default = 'h0
// Bit 1           rg_cw_remove_sub_bypass_man_mode0     U     RW        default = 'h0
// Bit 4           rg_cw_remove_sub_man_bypass1     U     RW        default = 'h0
// Bit 5           rg_cw_remove_sub_bypass_man_mode1     U     RW        default = 'h0
// Bit 8           rg_cw_remove_sub_man_bypass2     U     RW        default = 'h0
// Bit 9           rg_cw_remove_sub_bypass_man_mode2     U     RW        default = 'h0
// Bit 12          rg_cw_remove_sub_man_bypass3     U     RW        default = 'h0
// Bit 13          rg_cw_remove_sub_bypass_man_mode3     U     RW        default = 'h0
// Bit 16          rg_cw_remove_sub_man_bypass4     U     RW        default = 'h0
// Bit 17          rg_cw_remove_sub_bypass_man_mode4     U     RW        default = 'h0
// Bit 20          rg_dc_bypass                   U     RW        default = 'h0
// Bit 24          rg_hdmi_bypass                 U     RW        default = 'h0
// Bit 28          rg_cw_bypass                   U     RW        default = 'h0
// Bit 30          rg_cw_remove_bypass_val        U     RW        default = 'h1
// Bit 31          rg_cw_remove_bypass_man        U     RW        default = 'h1
typedef union RG_RECV_A16_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_cw_remove_sub_man_bypass0 : 1;
    unsigned int rg_cw_remove_sub_bypass_man_mode0 : 1;
    unsigned int rsvd_0 : 2;
    unsigned int rg_cw_remove_sub_man_bypass1 : 1;
    unsigned int rg_cw_remove_sub_bypass_man_mode1 : 1;
    unsigned int rsvd_1 : 2;
    unsigned int rg_cw_remove_sub_man_bypass2 : 1;
    unsigned int rg_cw_remove_sub_bypass_man_mode2 : 1;
    unsigned int rsvd_2 : 2;
    unsigned int rg_cw_remove_sub_man_bypass3 : 1;
    unsigned int rg_cw_remove_sub_bypass_man_mode3 : 1;
    unsigned int rsvd_3 : 2;
    unsigned int rg_cw_remove_sub_man_bypass4 : 1;
    unsigned int rg_cw_remove_sub_bypass_man_mode4 : 1;
    unsigned int rsvd_4 : 2;
    unsigned int rg_dc_bypass : 1;
    unsigned int rsvd_5 : 3;
    unsigned int rg_hdmi_bypass : 1;
    unsigned int rsvd_6 : 3;
    unsigned int rg_cw_bypass : 1;
    unsigned int rsvd_7 : 1;
    unsigned int rg_cw_remove_bypass_val : 1;
    unsigned int rg_cw_remove_bypass_man : 1;
  } b;
} RG_RECV_A16_FIELD_T;

#define RG_RECV_A17                               (0x44)
// Bit 24  :0      rg_cw_remove_sub_angle_base1     U     RW        default = 'h1d33333
typedef union RG_RECV_A17_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_cw_remove_sub_angle_base1 : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_RECV_A17_FIELD_T;

#define RG_RECV_A18                               (0x48)
// Bit 24  :0      rg_cw_remove_sub_angle_base2     U     RW        default = 'h800000
typedef union RG_RECV_A18_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_cw_remove_sub_angle_base2 : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_RECV_A18_FIELD_T;

#define RG_RECV_A19                               (0x4c)
// Bit 24  :0      rg_cw_remove_sub_angle_base3     U     RW        default = 'h1e00000
typedef union RG_RECV_A19_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_cw_remove_sub_angle_base3 : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_RECV_A19_FIELD_T;

#define RG_RECV_A20                               (0x50)
// Bit 24  :0      rg_cw_remove_sub_angle_base4     U     RW        default = 'he00000
typedef union RG_RECV_A20_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_cw_remove_sub_angle_base4 : 25;
    unsigned int rsvd_0 : 7;
  } b;
} RG_RECV_A20_FIELD_T;

#define RG_RECV_A21                               (0x54)
// Bit 10  :0      rg_dc_dc_remove_cnt            U     RW        default = 'h3ff
// Bit 25  :16     rg_hdmi_dc_remove_cnt          U     RW        default = 'h3f
typedef union RG_RECV_A21_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_dc_dc_remove_cnt : 11;
    unsigned int rsvd_0 : 5;
    unsigned int rg_hdmi_dc_remove_cnt : 10;
    unsigned int rsvd_1 : 6;
  } b;
} RG_RECV_A21_FIELD_T;

#define RG_RECV_A22                               (0x58)
// Bit 9   :0      rg_cw_dc_remove_cnt            U     RW        default = 'h3f
typedef union RG_RECV_A22_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_cw_dc_remove_cnt : 10;
    unsigned int rsvd_0 : 22;
  } b;
} RG_RECV_A22_FIELD_T;

#define RG_RECV_A23                               (0x5c)
// Bit 0           rg_dpd_cw_rmv_bypass           U     RW        default = 'h1
typedef union RG_RECV_A23_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_dpd_cw_rmv_bypass : 1;
    unsigned int rsvd_0 : 31;
  } b;
} RG_RECV_A23_FIELD_T;

#define RG_RECV_A24                               (0x60)
// Bit 9   :0      rg_cw_remove_gaindb_val        U     RW        default = 'h84
// Bit 12          rg_cw_remove_gaindb_man        U     RW        default = 'h0
// Bit 25  :16     rg_cw_remove_rssipow_val       U     RW        default = 'h84
// Bit 28          rg_cw_remove_rssipow_man       U     RW        default = 'h0
// Bit 30          rg_cw_remove_diffdb_or_gaindb     U     RW        default = 'h0
typedef union RG_RECV_A24_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_cw_remove_gaindb_val : 10;
    unsigned int rsvd_0 : 2;
    unsigned int rg_cw_remove_gaindb_man : 1;
    unsigned int rsvd_1 : 3;
    unsigned int rg_cw_remove_rssipow_val : 10;
    unsigned int rsvd_2 : 2;
    unsigned int rg_cw_remove_rssipow_man : 1;
    unsigned int rsvd_3 : 1;
    unsigned int rg_cw_remove_diffdb_or_gaindb : 1;
    unsigned int rsvd_4 : 1;
  } b;
} RG_RECV_A24_FIELD_T;

#define RG_RECV_A25                               (0x64)
// Bit 0           rg_rssi_snr_est                U     RW        default = 'h0
typedef union RG_RECV_A25_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rssi_snr_est : 1;
    unsigned int rsvd_0 : 31;
  } b;
} RG_RECV_A25_FIELD_T;

#define RG_RECV_A26                               (0x68)
// Bit 7   :0      rg_rx_scale_factor             U     RW        default = 'h10
typedef union RG_RECV_A26_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rx_scale_factor : 8;
    unsigned int rsvd_0 : 24;
  } b;
} RG_RECV_A26_FIELD_T;

#define RG_RECV_A27                               (0x6c)
// Bit 0           rg_rx_bn_sel                   U     RW        default = 'h0
// Bit 4           rg_rx_bn_sel_man_mode          U     RW        default = 'h0
// Bit 8           rg_cw_remove_11b_sub_bypass1     U     RW        default = 'h1
// Bit 12          rg_cw_remove_11b_sub_bypass2     U     RW        default = 'h1
// Bit 16          rg_cw_remove_11b_sub_bypass3     U     RW        default = 'h1
// Bit 20          rg_cw_remove_11b_sub_bypass4     U     RW        default = 'h1
typedef union RG_RECV_A27_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rx_bn_sel : 1;
    unsigned int rsvd_0 : 3;
    unsigned int rg_rx_bn_sel_man_mode : 1;
    unsigned int rsvd_1 : 3;
    unsigned int rg_cw_remove_11b_sub_bypass1 : 1;
    unsigned int rsvd_2 : 3;
    unsigned int rg_cw_remove_11b_sub_bypass2 : 1;
    unsigned int rsvd_3 : 3;
    unsigned int rg_cw_remove_11b_sub_bypass3 : 1;
    unsigned int rsvd_4 : 3;
    unsigned int rg_cw_remove_11b_sub_bypass4 : 1;
    unsigned int rsvd_5 : 11;
  } b;
} RG_RECV_A27_FIELD_T;

#define RG_RECV_A28                               (0x70)
// Bit 24          rg_rxirr_comp_gain_select      U     RW        default = 'h1
// Bit 28          rg_rxirr_comp_manaul           U     RW        default = 'h0
typedef union RG_RECV_A28_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rsvd_0 : 24;
    unsigned int rg_rxirr_comp_gain_select : 1;
    unsigned int rsvd_1 : 3;
    unsigned int rg_rxirr_comp_manaul : 1;
    unsigned int rsvd_2 : 3;
  } b;
} RG_RECV_A28_FIELD_T;

#define RG_RECV_A30                               (0x78)
// Bit 10  :0      rg_rxirr_comp_alpha_man        U     RW        default = 'h1
// Bit 20  :12     rg_rxirr_comp_theta_man        U     RW        default = 'h0
typedef union RG_RECV_A30_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxirr_comp_alpha_man : 11;
    unsigned int rsvd_0 : 1;
    unsigned int rg_rxirr_comp_theta_man : 9;
    unsigned int rsvd_1 : 11;
  } b;
} RG_RECV_A30_FIELD_T;

#define RG_RECV_A39                               (0x9c)
// Bit 14  :0      rg_rxirr_comp_coef0            U     RW        default = 'h0
// Bit 30  :16     rg_rxirr_comp_coef1            U     RW        default = 'h0
typedef union RG_RECV_A39_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxirr_comp_coef0 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int rg_rxirr_comp_coef1 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_RECV_A39_FIELD_T;

#define RG_RECV_A40                               (0xa0)
// Bit 14  :0      rg_rxirr_comp_coef2            U     RW        default = 'h7ffb
// Bit 30  :16     rg_rxirr_comp_coef3            U     RW        default = 'h15
typedef union RG_RECV_A40_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxirr_comp_coef2 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int rg_rxirr_comp_coef3 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_RECV_A40_FIELD_T;

#define RG_RECV_A41                               (0xa4)
// Bit 14  :0      rg_rxirr_comp_coef4            U     RW        default = 'h7ff8
// Bit 30  :16     rg_rxirr_comp_coef5            U     RW        default = 'h7ff2
typedef union RG_RECV_A41_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxirr_comp_coef4 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int rg_rxirr_comp_coef5 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_RECV_A41_FIELD_T;

#define RG_RECV_A42                               (0xa8)
// Bit 14  :0      rg_rxirr_comp_coef6            U     RW        default = 'h1e
// Bit 30  :16     rg_rxirr_comp_coef7            U     RW        default = 'h2015
typedef union RG_RECV_A42_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxirr_comp_coef6 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int rg_rxirr_comp_coef7 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_RECV_A42_FIELD_T;

#define RG_RECV_A43                               (0xac)
// Bit 14  :0      rg_rxirr_comp_coef8            U     RW        default = 'h7ff7
// Bit 30  :16     rg_rxirr_comp_coef9            U     RW        default = 'h14
typedef union RG_RECV_A43_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxirr_comp_coef8 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int rg_rxirr_comp_coef9 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_RECV_A43_FIELD_T;

#define RG_RECV_A44                               (0xb0)
// Bit 14  :0      rg_rxirr_comp_coef10           U     RW        default = 'h7fd7
// Bit 30  :16     rg_rxirr_comp_coef11           U     RW        default = 'h18
typedef union RG_RECV_A44_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxirr_comp_coef10 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int rg_rxirr_comp_coef11 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_RECV_A44_FIELD_T;

#define RG_RECV_A45                               (0xb4)
// Bit 14  :0      rg_rxirr_comp_coef12           U     RW        default = 'h7fd9
// Bit 30  :16     rg_rxirr_comp_coef13           U     RW        default = 'h0
typedef union RG_RECV_A45_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxirr_comp_coef12 : 15;
    unsigned int rsvd_0 : 1;
    unsigned int rg_rxirr_comp_coef13 : 15;
    unsigned int rsvd_1 : 1;
  } b;
} RG_RECV_A45_FIELD_T;

#define RG_RECV_A46                               (0xb8)
// Bit 14  :0      rg_rxirr_comp_coef14           U     RW        default = 'h0
typedef union RG_RECV_A46_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxirr_comp_coef14 : 15;
    unsigned int rsvd_0 : 17;
  } b;
} RG_RECV_A46_FIELD_T;

#define RG_RECV_A47                               (0xbc)
// Bit 11  :0      rg_rxi_dcoft                   U     RW        default = 'h0
// Bit 23  :12     rg_rxq_dcoft                   U     RW        default = 'h0
// Bit 24          rg_dcft_man_mode               U     RW        default = 'h1
typedef union RG_RECV_A47_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rxi_dcoft : 12;
    unsigned int rg_rxq_dcoft : 12;
    unsigned int rg_dcft_man_mode : 1;
    unsigned int rsvd_0 : 7;
  } b;
} RG_RECV_A47_FIELD_T;

#define RG_RECV_A48                               (0xc0)
// Bit 0           rg_fiiq_bypass                 U     RW        default = 'h1
// Bit 4           rg_fdiq_bypass                 U     RW        default = 'h1
typedef union RG_RECV_A48_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_fiiq_bypass : 1;
    unsigned int rsvd_0 : 3;
    unsigned int rg_fdiq_bypass : 1;
    unsigned int rsvd_1 : 27;
  } b;
} RG_RECV_A48_FIELD_T;

#define RG_RECV_A49                               (0xc4)
// Bit 14  :0      rg_fe_gain_lut                 U     RW        default = 'h3fe
typedef union RG_RECV_A49_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_fe_gain_lut : 15;
    unsigned int rsvd_0 : 17;
  } b;
} RG_RECV_A49_FIELD_T;

#define RG_RECV_A50                               (0xc8)
// Bit 0           rg_agc_lock_man                U     RW        default = 'h0
// Bit 4           rg_agc_lock_bypass             U     RW        default = 'h1
// Bit 8           rg_agc_lock_man_bit            U     RW        default = 'h1
// Bit 12          rg_fi_select_man               U     RW        default = 'h1
typedef union RG_RECV_A50_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_agc_lock_man : 1;
    unsigned int rsvd_0 : 3;
    unsigned int rg_agc_lock_bypass : 1;
    unsigned int rsvd_1 : 3;
    unsigned int rg_agc_lock_man_bit : 1;
    unsigned int rsvd_2 : 3;
    unsigned int rg_fi_select_man : 1;
    unsigned int rsvd_3 : 19;
  } b;
} RG_RECV_A50_FIELD_T;

#define RG_RECV_A51                               (0xcc)
// Bit 15  :0      rg_recv_soft_rst_ctrl          U     RW        default = 'h0
typedef union RG_RECV_A51_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_recv_soft_rst_ctrl : 16;
    unsigned int rsvd_0 : 16;
  } b;
} RG_RECV_A51_FIELD_T;

#define RG_RECV_A52                               (0xd0)
// Bit 10  :0      rg_rx_irr_alpha_fiiq_lg        U     RW        default = 'h0
// Bit 20  :12     rg_rx_irr_theta_fiiq_lg        U     RW        default = 'h0
typedef union RG_RECV_A52_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rx_irr_alpha_fiiq_lg : 11;
    unsigned int rsvd_0 : 1;
    unsigned int rg_rx_irr_theta_fiiq_lg : 9;
    unsigned int rsvd_1 : 11;
  } b;
} RG_RECV_A52_FIELD_T;

#define RG_RECV_A53                               (0xd4)
// Bit 10  :0      rg_rx_irr_alpha_fiiq_hg        U     RW        default = 'h0
// Bit 20  :12     rg_rx_irr_theta_fiiq_hg        U     RW        default = 'h0
typedef union RG_RECV_A53_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rx_irr_alpha_fiiq_hg : 11;
    unsigned int rsvd_0 : 1;
    unsigned int rg_rx_irr_theta_fiiq_hg : 9;
    unsigned int rsvd_1 : 11;
  } b;
} RG_RECV_A53_FIELD_T;

#define RG_RECV_A54                               (0xd8)
// Bit 31  :0      rg_rx_src_delta_low            U     RW        default = 'h2aaaaaab
typedef union RG_RECV_A54_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rx_src_delta_low : 32;
  } b;
} RG_RECV_A54_FIELD_T;

#define RG_RECV_A55                               (0xdc)
// Bit 6   :0      rg_rx_src_delta_high           U     RW        default = 'h0
// Bit 12          rg_rx_src_bypass               U     RW        default = 'h1
// Bit 19  :16     rg_rx_src_async_wgap           U     RW        default = 'ha
typedef union RG_RECV_A55_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rx_src_delta_high : 7;
    unsigned int rsvd_0 : 5;
    unsigned int rg_rx_src_bypass : 1;
    unsigned int rsvd_1 : 3;
    unsigned int rg_rx_src_async_wgap : 4;
    unsigned int rsvd_2 : 12;
  } b;
} RG_RECV_A55_FIELD_T;

#define RG_RECV_A56                               (0xe0)
// Bit 31  :0      rg_rx_rssi_src_delta_low       U     RW        default = 'h2aaaaaab
typedef union RG_RECV_A56_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rx_rssi_src_delta_low : 32;
  } b;
} RG_RECV_A56_FIELD_T;

#define RG_RECV_A57                               (0xe4)
// Bit 6   :0      rg_rx_rssi_src_delta_high      U     RW        default = 'h0
// Bit 12          rg_rx_rssi_src_bypass          U     RW        default = 'h1
// Bit 19  :16     rg_rx_rssi_src_async_wgap      U     RW        default = 'ha
typedef union RG_RECV_A57_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rx_rssi_src_delta_high : 7;
    unsigned int rsvd_0 : 5;
    unsigned int rg_rx_rssi_src_bypass : 1;
    unsigned int rsvd_1 : 3;
    unsigned int rg_rx_rssi_src_async_wgap : 4;
    unsigned int rsvd_2 : 12;
  } b;
} RG_RECV_A57_FIELD_T;

#define RG_RECV_A58                               (0xe8)
// Bit 0           rg_rx_src_async_sel            U     RW        default = 'h1
// Bit 4           rg_rx_src_async_clr            U     RW        default = 'h0
// Bit 8           rg_rx_rssi_src_async_sel       U     RW        default = 'h1
// Bit 16          rg_rx_rssi_src_async_clr       U     RW        default = 'h0
typedef union RG_RECV_A58_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rg_rx_src_async_sel : 1;
    unsigned int rsvd_0 : 3;
    unsigned int rg_rx_src_async_clr : 1;
    unsigned int rsvd_1 : 3;
    unsigned int rg_rx_rssi_src_async_sel : 1;
    unsigned int rsvd_2 : 7;
    unsigned int rg_rx_rssi_src_async_clr : 1;
    unsigned int rsvd_3 : 15;
  } b;
} RG_RECV_A58_FIELD_T;

#define RG_RECV_A59                               (0xec)
// Bit 0           ro_rx_adc_src_wfull            U     RO        default = 'h0
// Bit 4           ro_rx_adc_src_rempty           U     RO        default = 'h0
// Bit 8           ro_rx_rssi_src_wfull           U     RO        default = 'h0
// Bit 16          ro_rx_rssi_src_rempty          U     RO        default = 'h0
typedef union RG_RECV_A59_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_rx_adc_src_wfull : 1;
    unsigned int rsvd_0 : 3;
    unsigned int ro_rx_adc_src_rempty : 1;
    unsigned int rsvd_1 : 3;
    unsigned int ro_rx_rssi_src_wfull : 1;
    unsigned int rsvd_2 : 7;
    unsigned int ro_rx_rssi_src_rempty : 1;
    unsigned int rsvd_3 : 15;
  } b;
} RG_RECV_A59_FIELD_T;

#endif

