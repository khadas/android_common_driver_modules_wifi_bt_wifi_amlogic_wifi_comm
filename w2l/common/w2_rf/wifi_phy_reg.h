#ifdef WIFI_PHY_REG
#else
#define WIFI_PHY_REG

#define DF_PHY_REG_A0                             (0x0)
// Bit 31  :0      reg_clock_gate_mask            U     RW        default = 'hffffffff
typedef union DF_PHY_REG_A0_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_clock_gate_mask : 32;
  } b;
} DF_PHY_REG_A0_FIELD_T;

#define DF_PHY_REG_A2                             (0x8)
// Bit 15  :0      reg_wifi_phy_soft_rst_ctrl     U     RW        default = 'h0
typedef union DF_PHY_REG_A2_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_wifi_phy_soft_rst_ctrl : 16;
    unsigned int rsvd_0 : 16;
  } b;
} DF_PHY_REG_A2_FIELD_T;

#define DF_PHY_REG_A3                             (0xc)
// Bit 0           reg_adda0_enb                  U     RW        default = 'h1
// Bit 1           reg_adda1_enb                  U     RW        default = 'h1
// Bit 4           reg_frontend0_clk_enable       U     RW        default = 'h1
// Bit 5           reg_frontend1_clk_enable       U     RW        default = 'h1
typedef union DF_PHY_REG_A3_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_adda0_enb : 1;
    unsigned int reg_adda1_enb : 1;
    unsigned int rsvd_0 : 2;
    unsigned int reg_frontend0_clk_enable : 1;
    unsigned int reg_frontend1_clk_enable : 1;
    unsigned int rsvd_1 : 26;
  } b;
} DF_PHY_REG_A3_FIELD_T;

#define DF_PHY_REG_A4                             (0x10)
// Bit 31  :0      reg_fpga_control               U     RW        default = 'h0
typedef union DF_PHY_REG_A4_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_fpga_control : 32;
  } b;
} DF_PHY_REG_A4_FIELD_T;

#define DF_PHY_REG_A5                             (0x14)
// Bit 0           reg_adda_mimo_mode             U     RW        default = 'h0
// Bit 8           reg_adda2rf_mode_mux           U     RW        default = 'h0
typedef union DF_PHY_REG_A5_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_adda_mimo_mode : 1;
    unsigned int rsvd_0 : 7;
    unsigned int reg_adda2rf_mode_mux : 1;
    unsigned int rsvd_1 : 23;
  } b;
} DF_PHY_REG_A5_FIELD_T;

#define DF_PHY_REG_A64                            (0x100)
// Bit 0           reg_test_en                    U     RW        default = 'h1
typedef union DF_PHY_REG_A64_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_test_en : 1;
    unsigned int rsvd_0 : 31;
  } b;
} DF_PHY_REG_A64_FIELD_T;

#define DF_PHY_REG_A109                           (0x1b4)
// Bit 31  :0      reg_adda0_cfg                  U     RW        default = 'hc0000000
typedef union DF_PHY_REG_A109_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_adda0_cfg : 32;
  } b;
} DF_PHY_REG_A109_FIELD_T;

#define DF_PHY_REG_A110                           (0x1b8)
// Bit 31  :0      reg_adda1_cfg                  U     RW        default = 'hc0000000
typedef union DF_PHY_REG_A110_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_adda1_cfg : 32;
  } b;
} DF_PHY_REG_A110_FIELD_T;

#define DF_PHY_REG_A112                           (0x1c0)
// Bit 0           reg_wf_sx_ldo_fc               U     RW        default = 'h0
// Bit 12  :4      reg_wf_sx_ldo_fc_bnd           U     RW        default = 'hf0
typedef union DF_PHY_REG_A112_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_wf_sx_ldo_fc : 1;
    unsigned int rsvd_0 : 3;
    unsigned int reg_wf_sx_ldo_fc_bnd : 9;
    unsigned int rsvd_1 : 19;
  } b;
} DF_PHY_REG_A112_FIELD_T;

#define DF_PHY_REG_A113                           (0x1c4)
// Bit 30  :0      ro_wf_rf_status                U     RW        default = 'h0
typedef union DF_PHY_REG_A113_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int ro_wf_rf_status : 31;
    unsigned int rsvd_0 : 1;
  } b;
} DF_PHY_REG_A113_FIELD_T;

#define DF_PHY_REG_A137                           (0x224)
// Bit 8           reg_rf0_5g_2g                  U     RW        default = 'h1
// Bit 25  :24     reg_rf0_adc_fs                 U     RW        default = 'h3
// Bit 29  :28     reg_rf0_dac_fs                 U     RW        default = 'h3
typedef union DF_PHY_REG_A137_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rsvd_0 : 8;
    unsigned int reg_rf0_5g_2g : 1;
    unsigned int rsvd_1 : 15;
    unsigned int reg_rf0_adc_fs : 2;
    unsigned int rsvd_2 : 2;
    unsigned int reg_rf0_dac_fs : 2;
    unsigned int rsvd_3 : 2;
  } b;
} DF_PHY_REG_A137_FIELD_T;

#define DF_PHY_REG_A138                           (0x228)
// Bit 8           reg_rf1_5g_2g                  U     RW        default = 'h1
// Bit 25  :24     reg_rf1_adc_fs                 U     RW        default = 'h3
// Bit 29  :28     reg_rf1_dac_fs                 U     RW        default = 'h3
typedef union DF_PHY_REG_A138_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int rsvd_0 : 8;
    unsigned int reg_rf1_5g_2g : 1;
    unsigned int rsvd_1 : 15;
    unsigned int reg_rf1_adc_fs : 2;
    unsigned int rsvd_2 : 2;
    unsigned int reg_rf1_dac_fs : 2;
    unsigned int rsvd_3 : 2;
  } b;
} DF_PHY_REG_A138_FIELD_T;

#define DF_PHY_REG_A141                           (0x234)
// Bit 0           reg_tx0_lpf_bypass             U     RW        default = 'h0
// Bit 4           reg_tx1_lpf_bypass             U     RW        default = 'h0
typedef union DF_PHY_REG_A141_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_tx0_lpf_bypass : 1;
    unsigned int rsvd_0 : 3;
    unsigned int reg_tx1_lpf_bypass : 1;
    unsigned int rsvd_1 : 27;
  } b;
} DF_PHY_REG_A141_FIELD_T;

#define DF_PHY_REG_A142                           (0x238)
// Bit 0           reg_wf0_wf1_cali_sel           U     RW        default = 'h0
typedef union DF_PHY_REG_A142_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_wf0_wf1_cali_sel : 1;
    unsigned int rsvd_0 : 31;
  } b;
} DF_PHY_REG_A142_FIELD_T;

#define DF_PHY_REG_A143                           (0x23c)
// Bit 7   :0      reg_wf_tx_pa_en_delay_bnd      U     RW        default = 'h50
// Bit 9   :8      reg_wf2g_tx_mode               U     RW        default = 'h2
// Bit 11  :10     reg_wf5g_tx_mode               U     RW        default = 'h2
typedef union DF_PHY_REG_A143_FIELD
{
  unsigned int data;
  struct
  {
    unsigned int reg_wf_tx_pa_en_delay_bnd : 8;
    unsigned int reg_wf2g_tx_mode : 2;
    unsigned int reg_wf5g_tx_mode : 2;
    unsigned int rsvd_0 : 20;
  } b;
} DF_PHY_REG_A143_FIELD_T;
#endif
