/*	$OpenBSD: src/sys/dev/ic/ar9003reg.h,v 1.1 2010/05/10 17:44:21 damien Exp $	*/

/*-
 * Copyright (c) 2010 Damien Bergamini <damien.bergamini@free.fr>
 * Copyright (c) 2010 Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * MAC registers.
 */
#define AR_ISR_S2_S			0x00d0
#define AR_ISR_S3_S			0x00d4
#define AR_ISR_S4_S			0x00d8
#define AR_ISR_S5_S			0x00dc
#define AR_GPIO_IN_OUT			0x4048
#define AR_GPIO_OE_OUT			0x4050
#define AR_GPIO_INTR_POL		0x4058
#define AR_GPIO_INPUT_EN_VAL		0x405c
#define AR_GPIO_INPUT_MUX1		0x4060
#define AR_GPIO_INPUT_MUX2		0x4064
#define AR_GPIO_OUTPUT_MUX(i)		(0x4068 + (i) * 4)
#define AR_INPUT_STATE			0x4074
#define AR_EEPROM_STATUS_DATA		0x4084
#define AR_OBS				0x4088
#define AR_GPIO_PDPU			0x4090
#define AR_PCIE_MSI			0x40a4

/*
 * PHY registers.
 */
#define AR_PHY_TIMING1			0x09800
#define AR_PHY_TIMING2			0x09804
#define AR_PHY_TIMING3			0x09808
#define AR_PHY_TIMING4			0x0980c
#define AR_PHY_TIMING5			0x09810
#define AR_PHY_TIMING6			0x09814
#define AR_PHY_TIMING11			0x09818
#define AR_PHY_SPUR_REG			0x0981c
#define AR_PHY_FIND_SIG_LOW		0x09820
#define AR_PHY_SFCORR			0x09824
#define AR_PHY_SFCORR_LOW		0x09828
#define AR_PHY_SFCORR_EXT		0x0982c
#define AR_PHY_EXT_CCA(i)		(0x09830 + (i) * 0x1000)
#define AR_PHY_RADAR_0			0x09834
#define AR_PHY_RADAR_1			0x09838
#define AR_PHY_RADAR_EXT		0x0983c
#define AR_PHY_MULTICHAIN_CTRL		0x09880
#define AR_PHY_PERCHAIN_CSD		0x09884
#define AR_PHY_TX_CRC			0x098a0
#define AR_PHY_TST_DAC_CONST		0x098a4
#define AR_PHY_SPUR_REPORT_0		0x098a8
#define AR_PHY_TX_IQCAL_CONTROL_3	0x098b0
#define AR_PHY_IQ_ADC_MEAS_0_B(i)	(0x098c0 + (i) * 0x1000)
#define AR_PHY_IQ_ADC_MEAS_1_B(i)	(0x098c4 + (i) * 0x1000)
#define AR_PHY_IQ_ADC_MEAS_2_B(i)	(0x098c8 + (i) * 0x1000)
#define AR_PHY_IQ_ADC_MEAS_3_B(i)	(0x098cc + (i) * 0x1000)
#define AR_PHY_TX_PHASE_RAMP_0		0x098d0
#define AR_PHY_ADC_DC_GAIN_CORR(i)	(0x098d4 + (i) * 0x1000)
#define AR_PHY_RX_IQCAL_CORR_B(i)	(0x098dc + (i) * 0x1000)
#define AR_PHY_CHAN_INFO_TAB(i, j)	(0x09b00 + (i) * 0x1000 + (j) * 4)
#define AR_PHY_TIMING_3A		0x09c00
#define AR_PHY_LDPC_CNTL1		0x09c04
#define AR_PHY_LDPC_CNTL2		0x09c08
#define AR_PHY_PILOT_SPUR_MASK		0x09c0c
#define AR_PHY_CHAN_SPUR_MASK		0x09c10
#define AR_PHY_SGI_DELTA		0x09c14
#define AR_PHY_ML_CNTL_1		0x09c18
#define AR_PHY_ML_CNTL_2		0x09c1c
#define AR_PHY_TST_ADC			0x09c20
#define AR_PHY_SETTLING			0x09e00
#define AR_PHY_RXGAIN(i)		(0x09e04 + (i) * 0x1000)
#define AR_PHY_GAINS_MINOFF0		0x09e08
#define AR_PHY_DESIRED_SZ		0x09e0c
#define AR_PHY_FIND_SIG			0x09e10
#define AR_PHY_AGC			0x09e14
#define AR_PHY_EXT_ATTEN_CTL(i)		(0x09e18 + (i) * 0x1000)
#define AR_PHY_CCA(i)			(0x09e1c + (i) * 0x1000)
#define AR_PHY_CCA_CTRL(i)		(0x09e20 + (i) * 0x1000)
#define AR_PHY_RESTART			0x09e24
#define AR_PHY_MC_GAIN_CTRL		0x09e28
#define AR_PHY_EXTCHN_PWRTHR1		0x09e2c
#define AR_PHY_EXT_CHN_WIN		0x09e30
#define AR_PHY_20_40_DET_THR		0x09e34
#define AR_PHY_RIFS_SRCH		0x09e38
#define AR_PHY_PEAK_DET_CTRL_1		0x09e3c
#define AR_PHY_PEAK_DET_CTRL_2		0x09e40
#define AR_PHY_RX_GAIN_BOUNDS_1		0x09e44
#define AR_PHY_RX_GAIN_BOUNDS_2		0x09e48
#define AR_PHY_RSSI(i)			(0x09f80 + (i) * 0x1000)
#define AR_PHY_SPUR_CCK_REP0		0x09f84
#define AR_PHY_CCK_DETECT		0x09fc0
#define AR_PHY_DAG_CTRLCCK		0x09fc4
#define AR_PHY_IQCORR_CTRL_CCK		0x09fc8
#define AR_PHY_CCK_SPUR_MIT		0x09fcc
#define AR_PHY_RX_OCGAIN		0x0a000
#define AR_PHY_D2_CHIP_ID		0x0a200
#define AR_PHY_GEN_CTRL			0x0a204
#define AR_PHY_MODE			0x0a208
#define AR_PHY_ACTIVE			0x0a20c
#define AR_PHY_SPUR_MASK_A		0x0a220
#define AR_PHY_SPUR_MASK_B		0x0a224
#define AR_PHY_SPECTRAL_SCAN		0x0a228
#define AR_PHY_RADAR_BW_FILTER		0x0a22c
#define AR_PHY_SEARCH_START_DELAY	0x0a230
#define AR_PHY_MAX_RX_LEN		0x0a234
#define AR_PHY_FRAME_CTL		0x0a238
#define AR_PHY_RFBUS_REQ		0x0a23c
#define AR_PHY_RFBUS_GRANT		0x0a240
#define AR_PHY_RIFS			0x0a244
#define AR_PHY_RX_CLR_DELAY		0x0a250
#define AR_PHY_RX_DELAY			0x0a254
#define AR_PHY_XPA_TIMING_CTL		0x0a264
#define AR_PHY_MISC_PA_CTL		0x0a280
#define AR_PHY_SWITCH_CHAIN(i)		(0x0a284 + (i) * 0x1000)
#define AR_PHY_SWITCH_COM		0x0a288
#define AR_PHY_SWITCH_COM_2		0x0a28c
#define AR_PHY_RX_CHAINMASK		0x0a2a0
#define AR_PHY_CAL_CHAINMASK		0x0a2c0
#define AR_PHY_AGC_CONTROL		0x0a2c4
#define AR_PHY_CALMODE			0x0a2c8
#define AR_PHY_FCAL_1			0x0a2cc
#define AR_PHY_FCAL_2_0			0x0a2d0
#define AR_PHY_DFT_TONE_CTL_0		0x0a2d4
#define AR_PHY_CL_CAL_CTL		0x0a2d8
#define AR_PHY_CL_TAB_0			0x0a300
#define AR_PHY_SYNTH_CONTROL		0x0a340
#define AR_PHY_ADDAC_CLK_SEL		0x0a344
#define AR_PHY_PLL_CTL			0x0a348
#define AR_PHY_ANALOG_SWAP		0x0a34c
#define AR_PHY_ADDAC_PARA_CTL		0x0a350
#define AR_PHY_XPA_CFG			0x0a358
#define AR_PHY_TEST			0x0a360
#define AR_PHY_TEST_CTL_STATUS		0x0a364
#define AR_PHY_TSTDAC			0x0a368
#define AR_PHY_CHAN_STATUS		0x0a36c
#define AR_PHY_CHAN_INFO_MEMORY		0x0a370
#define AR_PHY_CHNINFO_NOISEPWR		0x0a374
#define AR_PHY_CHNINFO_GAINDIFF		0x0a378
#define AR_PHY_CHNINFO_FINETIM		0x0a37c
#define AR_PHY_CHAN_INFO_GAIN_0		0x0a380
#define AR_PHY_SCRAMBLER_SEED		0x0a390
#define AR_PHY_CCK_TX_CTRL		0x0a394
#define AR_PHY_HEAVYCLIP_CTL		0x0a3a4
#define AR_PHY_HEAVYCLIP_20		0x0a3a8
#define AR_PHY_HEAVYCLIP_40		0x0a3ac
#define AR_PHY_ILLEGAL_TXRATE		0x0a3b0
#define AR_PHY_PWRTX_RATE1		0x0a3c0
#define AR_PHY_PWRTX_RATE2		0x0a3c4
#define AR_PHY_PWRTX_RATE3		0x0a3c8
#define AR_PHY_PWRTX_RATE4		0x0a3cc
#define AR_PHY_PWRTX_RATE5		0x0a3d0
#define AR_PHY_PWRTX_RATE6		0x0a3d4
#define AR_PHY_PWRTX_RATE7		0x0a3d8
#define AR_PHY_PWRTX_RATE8		0x0a3dc
#define AR_PHY_PWRTX_RATE10		0x0a3e4
#define AR_PHY_PWRTX_RATE11		0x0a3e8
#define AR_PHY_PWRTX_RATE12		0x0a3ec
#define AR_PHY_PWRTX_MAX		0x0a3f0
#define AR_PHY_POWER_TX_SUB		0x0a3f4
#define AR_PHY_TPC_4_B0			0x0a404
#define AR_PHY_TPCRG5(i)		(0x0a408 + (i) * 0x1000)
#define AR_PHY_TPC_6_B(i)		(0x0a40c + (i) * 0x1000)
#define AR_PHY_TPC_11_B(i)		(0x0a420 + (i) * 0x1000)
#define AR_PHY_TPC_18			0x0a43c
#define AR_PHY_TPC_19			0x0a440
#define AR_PHY_BB_THERM_ADC_1		0x0a448
#define AR_PHY_TX_FORCED_GAIN		0x0a458
#define AR_PHY_PDADC_TAB(i)		(0x0a480 + (i) * 0x1000)
#define AR_PHY_TX_IQCAL_CONTROL_1	0x0a648
#define AR_PHY_TX_IQCAL_START		0x0a640
#define AR_PHY_TX_IQCAL_CORR_COEFF_01_B(i)	\
					(0x0a650 + (i) * 0x1000)
#define AR_PHY_TX_IQCAL_STATUS_B(i)	(0x0a68c + (i) * 0x1000)
#define AR_PHY_PANIC_WD_STATUS		0x0a7c0
#define AR_PHY_PANIC_WD_CTL_1		0x0a7c4
#define AR_PHY_PANIC_WD_CTL_2		0x0a7c8
#define AR_PHY_BT_CTL			0x0a7cc
#define AR_PHY_ONLY_WARMRESET		0x0a7d0
#define AR_PHY_ONLY_CTL			0x0a7d4
#define AR_PHY_ECO_CTRL			0x0a7dc

/*
 * Analog registers.
 */
#define AR_IS_ANALOG_REG(reg)		((reg) >= 0x16000 && (reg) <= 0x17000)
#define AR_PHY_65NM_CH0_SYNTH4		0x1608c
#define AR_PHY_65NM_CH0_SYNTH7		0x16098
#define AR_PHY_65NM_CH0_BIAS1		0x160c0
#define AR_PHY_65NM_CH0_BIAS2		0x160c4
#define AR_PHY_65NM_CH0_BIAS4		0x160cc
#define AR_PHY_65NM_CH0_RXTX1		0x16100
#define AR_PHY_65NM_CH0_RXTX2		0x16104
#define AR_PHY_65NM_CH0_RXTX4		0x1610c
#define AR_PHY_65NM_CH0_TOP		0x16288
#define AR_PHY_65NM_CH0_THERM		0x16290
#define AR_PHY_65NM_CH1_RXTX1		0x16500
#define AR_PHY_65NM_CH1_RXTX2		0x16504
#define AR_PHY_65NM_CH2_RXTX1		0x16900
#define AR_PHY_65NM_CH2_RXTX2		0x16904


/* Bits for AR_PHY_TIMING2. */
#define AR_PHY_TIMING2_FORCE_PPM_VAL_M	0x00000fff
#define AR_PHY_TIMING2_FORCE_PPM_VAL_S	0
#define AR_PHY_TIMING2_USE_FORCE_PPM	0x00001000

/* Bits for AR_PHY_TIMING3. */
#define AR_PHY_TIMING3_DSC_EXP_M	0x0001e000
#define AR_PHY_TIMING3_DSC_EXP_S	13
#define AR_PHY_TIMING3_DSC_MAN_M	0xfffe0000
#define AR_PHY_TIMING3_DSC_MAN_S	17

/* Bits for AR_PHY_TIMING4. */
#define AR_PHY_TIMING4_IQCAL_LOG_COUNT_MAX_M	0x0000f000
#define AR_PHY_TIMING4_IQCAL_LOG_COUNT_MAX_S	12
#define AR_PHY_TIMING4_DO_CAL			0x00010000
#define AR_PHY_TIMING4_ENABLE_PILOT_MASK	0x10000000
#define AR_PHY_TIMING4_ENABLE_CHAN_MASK		0x20000000
#define AR_PHY_TIMING4_ENABLE_SPUR_FILTER	0x40000000
#define AR_PHY_TIMING4_ENABLE_SPUR_RSSI		0x80000000

/* Bits for AR_PHY_TIMING5. */
#define AR_PHY_TIMING5_CYCPWR_THR1_ENABLE	0x00000001
#define AR_PHY_TIMING5_CYCPWR_THR1_M		0x000000fe
#define AR_PHY_TIMING5_CYCPWR_THR1_S		1
#define AR_PHY_TIMING5_RSSI_THR1A_ENA		0x00008000
#define AR_PHY_TIMING5_CYCPWR_THR1A_M		0x007f0000
#define AR_PHY_TIMING5_CYCPWR_THR1A_S		16
#define AR_PHY_TIMING5_RSSI_THR1A_M		0x007f0000
#define AR_PHY_TIMING5_RSSI_THR1A_S		16

/* Bits for AR_PHY_TIMING11. */
#define AR_PHY_TIMING11_SPUR_DELTA_PHASE_M		0x000fffff
#define AR_PHY_TIMING11_SPUR_DELTA_PHASE_S		0
#define AR_PHY_TIMING11_SPUR_FREQ_SD_M			0x3ff00000
#define AR_PHY_TIMING11_SPUR_FREQ_SD_S			20
#define AR_PHY_TIMING11_USE_SPUR_FILTER_IN_AGC		0x40000000
#define AR_PHY_TIMING11_USE_SPUR_FILTER_IN_SELFCOR	0x80000000

/* Bits for AR_PHY_SPUR_REG. */
#define AR_PHY_SPUR_REG_SPUR_RSSI_THRESH_M	0x000000ff
#define AR_PHY_SPUR_REG_SPUR_RSSI_THRESH_S	0
#define AR_PHY_SPUR_REG_EN_VIT_SPUR_RSSI	0x00000100
#define AR_PHY_SPUR_REG_ENABLE_MASK_PPM		0x00020000
#define AR_PHY_SPUR_REG_MASK_RATE_CNTL_M	0x03fc0000
#define AR_PHY_SPUR_REG_MASK_RATE_CNTL_S	18
#define AR_PHY_SPUR_REG_ENABLE_NF_RSSI_SPUR_MIT	0x04000000

/* Bits for AR_PHY_FIND_SIG_LOW. */
#define AR_PHY_FIND_SIG_LOW_RELSTEP_M		0x0000001f
#define AR_PHY_FIND_SIG_LOW_RELSTEP_S		0
#define AR_PHY_FIND_SIG_LOW_FIRSTEP_LOW_M	0x00000fc0
#define AR_PHY_FIND_SIG_LOW_FIRSTEP_LOW_S	6
#define AR_PHY_FIND_SIG_LOW_FIRPWR_M		0x0007f000
#define AR_PHY_FIND_SIG_LOW_FIRPWR_S		12

/* Bits for AR_PHY_SFCORR. */
#define AR_PHY_SFCORR_M2COUNT_THR_M		0x0000001f
#define AR_PHY_SFCORR_M2COUNT_THR_S		0
#define AR_PHY_SFCORR_M1_THRESH_M		0x00fe0000
#define AR_PHY_SFCORR_M1_THRESH_S		17
#define AR_PHY_SFCORR_M2_THRESH_M		0x7f000000
#define AR_PHY_SFCORR_M2_THRESH_S		24

/* Bits for AR_PHY_SFCORR_LOW. */
#define AR_PHY_SFCORR_LOW_USE_SELF_CORR_LOW	0x00000001
#define AR_PHY_SFCORR_LOW_M2COUNT_THR_LOW_M	0x00003f00
#define AR_PHY_SFCORR_LOW_M2COUNT_THR_LOW_S	8
#define AR_PHY_SFCORR_LOW_M1_THRESH_LOW_M	0x001fc000
#define AR_PHY_SFCORR_LOW_M1_THRESH_LOW_S	14
#define AR_PHY_SFCORR_LOW_M2_THRESH_LOW_M	0x0fe00000
#define AR_PHY_SFCORR_LOW_M2_THRESH_LOW_S	21

/* Bits for AR_PHY_SFCORR_EXT. */
#define AR_PHY_SFCORR_EXT_M1_THRESH_M		0x0000007f
#define AR_PHY_SFCORR_EXT_M1_THRESH_S		0
#define AR_PHY_SFCORR_EXT_M2_THRESH_M		0x00003f80
#define AR_PHY_SFCORR_EXT_M2_THRESH_S		7
#define AR_PHY_SFCORR_EXT_M1_THRESH_LOW_M	0x001fc000
#define AR_PHY_SFCORR_EXT_M1_THRESH_LOW_S	14
#define AR_PHY_SFCORR_EXT_M2_THRESH_LOW_M	0x0fe00000
#define AR_PHY_SFCORR_EXT_M2_THRESH_LOW_S	21
#define AR_PHY_SFCORR_EXT_SPUR_SUBCHANNEL_SD	0x10000000

/* Bits for AR_PHY_RADAR_0. */
#define AR_PHY_RADAR_0_ENA		0x00000001
#define AR_PHY_RADAR_0_INBAND_M		0x0000003e
#define AR_PHY_RADAR_0_INBAND_S		1
#define AR_PHY_RADAR_0_PRSSI_M		0x00000fc0
#define AR_PHY_RADAR_0_PRSSI_S		6
#define AR_PHY_RADAR_0_HEIGHT_M		0x0003f000
#define AR_PHY_RADAR_0_HEIGHT_S		12
#define AR_PHY_RADAR_0_RRSSI_M		0x00fc0000
#define AR_PHY_RADAR_0_RRSSI_S		18
#define AR_PHY_RADAR_0_FIRPWR_M		0x7f000000
#define AR_PHY_RADAR_0_FIRPWR_S		24
#define AR_PHY_RADAR_0_FFT_ENA		0x80000000

/* Bits for AR_PHY_RADAR_1. */
#define AR_PHY_RADAR_1_MAXLEN_M		0x000000ff
#define AR_PHY_RADAR_1_MAXLEN_S		0
#define AR_PHY_RADAR_1_RELSTEP_THRESH_M	0x00001f00
#define AR_PHY_RADAR_1_RELSTEP_THRESH_S	8
#define AR_PHY_RADAR_1_RELSTEP_CHECK	0x00002000
#define AR_PHY_RADAR_1_MAX_RRSSI	0x00004000
#define AR_PHY_RADAR_1_BLOCK_CHECK	0x00008000
#define AR_PHY_RADAR_1_RELPWR_THRESH_M	0x003f0000
#define AR_PHY_RADAR_1_RELPWR_THRESH_S	16
#define AR_PHY_RADAR_1_USE_FIR128	0x00400000
#define AR_PHY_RADAR_1_RELPWR_ENA	0x00800000

/* Bits for AR_PHY_RADAR_EXT. */
#define AR_PHY_RADAR_EXT_ENA		0x00004000
#define AR_PHY_RADAR_DC_PWR_THRESH_M	0x007f8000
#define AR_PHY_RADAR_DC_PWR_THRESH_S	15
#define AR_PHY_RADAR_LB_DC_CAP_M  	0x7f800000
#define AR_PHY_RADAR_LB_DC_CAP_S	23

/* Bits for AR_PHY_TX_IQCAL_CONTROL_3. */
#define AR_PHY_TX_IQCAL_CONTROL_3_IQCORR_EN	0x80000000

/* Bits for AR_PHY_RX_IQCAL_CORR_B(0). */
#define AR_PHY_RX_IQCAL_CORR_IQCORR_Q_Q_COFF_M		0x0000007f
#define AR_PHY_RX_IQCAL_CORR_IQCORR_Q_Q_COFF_S		0
#define AR_PHY_RX_IQCAL_CORR_IQCORR_Q_I_COFF_M		0x00003f80
#define AR_PHY_RX_IQCAL_CORR_IQCORR_Q_I_COFF_S		7
#define AR_PHY_RX_IQCAL_CORR_IQCORR_ENABLE		0x00004000
#define AR_PHY_RX_IQCAL_CORR_LOOPBACK_IQCORR_Q_Q_COFF_M	0x003f8000
#define AR_PHY_RX_IQCAL_CORR_LOOPBACK_IQCORR_Q_Q_COFF_S	15
#define AR_PHY_RX_IQCAL_CORR_LOOPBACK_IQCORR_Q_I_COFF_M	0x1fc00000
#define AR_PHY_RX_IQCAL_CORR_LOOPBACK_IQCORR_Q_I_COFF_S	22
#define AR_PHY_RX_IQCAL_CORR_B0_LOOPBACK_IQCORR_EN	0x20000000

/* Bits for AR_PHY_PILOT_SPUR_MASK. */
#define AR_PHY_PILOT_SPUR_MASK_CF_PILOT_MASK_A_M	0x0000001f
#define AR_PHY_PILOT_SPUR_MASK_CF_PILOT_MASK_A_S	0
#define AR_PHY_PILOT_SPUR_MASK_CF_PILOT_MASK_IDX_A_M	0x00000fe0
#define AR_PHY_PILOT_SPUR_MASK_CF_PILOT_MASK_IDX_A_S	5

/* Bits for AR_PHY_CHAN_SPUR_MASK. */
#define AR_PHY_CHAN_SPUR_MASK_CF_CHAN_MASK_A_M		0x0000001f
#define AR_PHY_CHAN_SPUR_MASK_CF_CHAN_MASK_A_S		0
#define AR_PHY_CHAN_SPUR_MASK_CF_CHAN_MASK_IDX_A_M	0x00000fe0
#define AR_PHY_CHAN_SPUR_MASK_CF_CHAN_MASK_IDX_A_S	5

/* Bits for AR_PHY_SGI_DELTA. */
#define AR_PHY_SGI_DSC_EXP_M	0x0000000f
#define AR_PHY_SGI_DSC_EXP_S	0
#define AR_PHY_SGI_DSC_MAN_M	0x0007fff0
#define AR_PHY_SGI_DSC_MAN_S	4

/* Bits for AR_PHY_SETTLING. */
#define AR_PHY_SETTLING_SWITCH_M	0x00003f80
#define AR_PHY_SETTLING_SWITCH_S	7

/* Bits for AR_PHY_RXGAIN(i). */
#define AR_PHY_RXGAIN_TXRX_ATTEN_M	0x0003f000
#define AR_PHY_RXGAIN_TXRX_ATTEN_S	12
#define AR_PHY_RXGAIN_TXRX_RF_MAX_M	0x007c0000
#define AR_PHY_RXGAIN_TXRX_RF_MAX_S	18

/* Bits for AR_PHY_DESIRED_SZ. */
#define AR_PHY_DESIRED_SZ_ADC_M		0x000000ff
#define AR_PHY_DESIRED_SZ_ADC_S		0
#define AR_PHY_DESIRED_SZ_PGA_M		0x0000ff00
#define AR_PHY_DESIRED_SZ_PGA_S		8
#define AR_PHY_DESIRED_SZ_TOT_DES_M	0x0ff00000
#define AR_PHY_DESIRED_SZ_TOT_DES_S	20

/* Bits for AR_PHY_FIND_SIG. */
#define AR_PHY_FIND_SIG_RELSTEP_M	0x0000001f
#define AR_PHY_FIND_SIG_RELSTEP_S	0
#define AR_PHY_FIND_SIG_RELPWR_M	0x000007c0
#define AR_PHY_FIND_SIG_RELPWR_S	6
#define AR_PHY_FIND_SIG_FIRSTEP_M	0x0003f000
#define AR_PHY_FIND_SIG_FIRSTEP_S	12
#define AR_PHY_FIND_SIG_FIRPWR_M	0x03fc0000
#define AR_PHY_FIND_SIG_FIRPWR_S	18

/* Bits for AR_PHY_AGC. */
#define AR_PHY_AGC_COARSE_PWR_CONST_M	0x0000007f
#define AR_PHY_AGC_COARSE_PWR_CONST_S	0
#define AR_PHY_AGC_COARSE_LOW_M		0x00007f80
#define AR_PHY_AGC_COARSE_LOW_S		7
#define AR_PHY_AGC_COARSE_HIGH_M	0x003f8000
#define AR_PHY_AGC_COARSE_HIGH_S	15

/* Bits for AR_PHY_EXT_ATTEN_CTL(i). */
#define AR_PHY_EXT_ATTEN_CTL_BSW_ATTEN_M	0x0000001f
#define AR_PHY_EXT_ATTEN_CTL_BSW_ATTEN_S	0
#define AR_PHY_EXT_ATTEN_CTL_XATTEN1_DB_M	0x0000003f
#define AR_PHY_EXT_ATTEN_CTL_XATTEN1_DB_S	0
#define AR_PHY_EXT_ATTEN_CTL_XATTEN2_DB_M	0x00000fc0
#define AR_PHY_EXT_ATTEN_CTL_XATTEN2_DB_S	6
#define AR_PHY_EXT_ATTEN_CTL_BSW_MARGIN_M	0x00003c00
#define AR_PHY_EXT_ATTEN_CTL_BSW_MARGIN_S	10
#define AR_PHY_EXT_ATTEN_CTL_XATTEN1_MARGIN_M	0x0001f000
#define AR_PHY_EXT_ATTEN_CTL_XATTEN1_MARGIN_S	12
#define AR_PHY_EXT_ATTEN_CTL_XATTEN2_MARGIN_M	0x003e0000
#define AR_PHY_EXT_ATTEN_CTL_XATTEN2_MARGIN_S	17
#define AR_PHY_EXT_ATTEN_CTL_RXTX_MARGIN_M	0x00fc0000
#define AR_PHY_EXT_ATTEN_CTL_RXTX_MARGIN_S	18

/* Bits for AR_PHY_CCA(i). */
#define AR_PHY_MAXCCA_PWR_M	0x000001ff
#define AR_PHY_MAXCCA_PWR_S	0
#define AR_PHY_MINCCA_PWR_M	0x1ff00000
#define AR_PHY_MINCCA_PWR_S	20

/* Bits for AR_PHY_EXT_CCA(i). */
#define AR_PHY_EXT_MAXCCA_PWR_M		0x000001ff
#define AR_PHY_EXT_MAXCCA_PWR_S		0
#define AR_PHY_EXT_MINCCA_PWR_M		0x01ff0000
#define AR_PHY_EXT_MINCCA_PWR_S		16

/* Bits for AR_PHY_RESTART. */
#define AR_PHY_RESTART_ENA	0x00000001
#define AR_PHY_RESTART_DIV_GC_M	0x001c0000
#define AR_PHY_RESTART_DIV_GC_S	18

/* Bits for AR_PHY_CCK_DETECT. */
#define AR_PHY_CCK_DETECT_WEAK_SIG_THR_CCK_M		0x0000003f
#define AR_PHY_CCK_DETECT_WEAK_SIG_THR_CCK_S		0
#define AR_PHY_CCK_DETECT_ANT_SWITCH_TIME_M		0x00001fc0
#define AR_PHY_CCK_DETECT_ANT_SWITCH_TIME_S		6
#define AR_PHY_CCK_DETECT_BB_ENABLE_ANT_FAST_DIV	0x00002000

/* Bits for AR_PHY_DAG_CTRLCCK. */
#define AR_PHY_DAG_CTRLCCK_EN_RSSI_THR	0x00000200
#define AR_PHY_DAG_CTRLCCK_RSSI_THR_M	0x0001fc00
#define AR_PHY_DAG_CTRLCCK_RSSI_THR_S	10

/* Bits for AR_PHY_CCK_SPUR_MIT. */
#define AR_PHY_CCK_SPUR_MIT_USE_CCK_SPUR_MIT	0x00000001
#define AR_PHY_CCK_SPUR_MIT_SPUR_RSSI_THR_M	0x000001fe
#define AR_PHY_CCK_SPUR_MIT_SPUR_RSSI_THR_S	1
#define AR_PHY_CCK_SPUR_MIT_CCK_SPUR_FREQ_M	0x1ffffe00
#define AR_PHY_CCK_SPUR_MIT_CCK_SPUR_FREQ_S	9
#define AR_PHY_CCK_SPUR_MIT_SPUR_FILTER_TYPE_M	0x60000000
#define AR_PHY_CCK_SPUR_MIT_SPUR_FILTER_TYPE_S  29

/* Bits for AR_PHY_GEN_CTRL. */
#define AR_PHY_GC_TURBO_MODE		0x00000001
#define AR_PHY_GC_TURBO_SHORT		0x00000002
#define AR_PHY_GC_DYN2040_EN		0x00000004
#define AR_PHY_GC_DYN2040_PRI_ONLY	0x00000008
#define AR_PHY_GC_DYN2040_PRI_CH	0x00000010
#define AR_PHY_GC_DYN2040_EXT_CH	0x00000020
#define AR_PHY_GC_HT_EN			0x00000040
#define AR_PHY_GC_SHORT_GI_40		0x00000080
#define AR_PHY_GC_WALSH			0x00000100
#define AR_PHY_GC_SINGLE_HT_LTF1	0x00000200
#define AR_PHY_GC_GF_DETECT_EN		0x00000400
#define AR_PHY_GC_ENABLE_DAC_FIFO	0x00000800

/* Bits for AR_PHY_MODE. */
#define AR_PHY_MODE_OFDM		0x00000000
#define AR_PHY_MODE_CCK			0x00000001
#define AR_PHY_MODE_DYNAMIC		0x00000004
#define AR_PHY_MODE_HALF		0x00000020
#define AR_PHY_MODE_QUARTER		0x00000040
#define AR_PHY_MODE_DYN_CCK_DISABLE	0x00000100
#define AR_PHY_MODE_SVD_HALF		0x00000200

/* Bits for AR_PHY_ACTIVE. */
#define AR_PHY_ACTIVE_DIS	0x00000000
#define AR_PHY_ACTIVE_EN	0x00000001

/* Bits for AR_PHY_SPUR_MASK_A. */
#define AR_PHY_SPUR_MASK_A_CF_PUNC_MASK_A_M	0x000003ff
#define AR_PHY_SPUR_MASK_A_CF_PUNC_MASK_A_S	0
#define AR_PHY_SPUR_MASK_A_CF_PUNC_MASK_IDX_A_M	0x0001fc00
#define AR_PHY_SPUR_MASK_A_CF_PUNC_MASK_IDX_A_S	10

/* Bits for AR_PHY_SPECTRAL_SCAN. */
#define AR_PHY_SPECTRAL_SCAN_ENABLE		0x00000001
#define AR_PHY_SPECTRAL_SCAN_ACTIVE		0x00000002
#define AR_PHY_SPECTRAL_SCAN_FFT_PERIOD_M	0x000000f0
#define AR_PHY_SPECTRAL_SCAN_FFT_PERIOD_S	4
#define AR_PHY_SPECTRAL_SCAN_PERIOD_M		0x0000ff00
#define AR_PHY_SPECTRAL_SCAN_PERIOD_S		8
#define AR_PHY_SPECTRAL_SCAN_COUNT_M		0x00ff0000
#define AR_PHY_SPECTRAL_SCAN_COUNT_S		16
#define AR_PHY_SPECTRAL_SCAN_SHORT_REPEAT	0x01000000

/* Bits for AR_PHY_RFBUS_REQ. */
#define AR_PHY_RFBUS_REQ_EN	0x00000001

/* Bits for AR_PHY_RFBUS_GRANT. */
#define AR_PHY_RFBUS_GRANT_EN	0x00000001

/* Bits for AR_PHY_RIFS. */
#define AR_PHY_RIFS_INIT_DELAY	0x3ff0000

/* Bits for AR_PHY_RX_DELAY. */
#define AR_PHY_RX_DELAY_DELAY_M	0x00003fff
#define AR_PHY_RX_DELAY_DELAY_S	0

/* Bits for AR_PHY_XPA_TIMING_CTL. */
#define AR_PHY_XPA_TIMING_CTL_FRAME_XPAA_ON_M	0x000000ff
#define AR_PHY_XPA_TIMING_CTL_FRAME_XPAA_ON_S	0
#define AR_PHY_XPA_TIMING_CTL_FRAME_XPAB_ON_M	0x0000ff00
#define AR_PHY_XPA_TIMING_CTL_FRAME_XPAB_ON_S	8
#define AR_PHY_XPA_TIMING_CTL_TX_END_XPAA_OFF_M	0x00ff0000
#define AR_PHY_XPA_TIMING_CTL_TX_END_XPAA_OFF_S	16
#define AR_PHY_XPA_TIMING_CTL_TX_END_XPAB_OFF_M	0xff000000
#define AR_PHY_XPA_TIMING_CTL_TX_END_XPAB_OFF_S	24

/* Bits for AR_PHY_SWITCH_CHAIN. */
#define AR_SWITCH_TABLE_ALL_M	0x00000fff
#define AR_SWITCH_TABLE_ALL_S	0

/* Bits for AR_PHY_SWITCH_COM. */
#define AR_SWITCH_TABLE_COM_ALL_M	0x0000ffff
#define AR_SWITCH_TABLE_COM_ALL_S	0

/* Bits for AR_SWITCH_TABLE_COM_2. */
#define AR_SWITCH_TABLE_COM_2_ALL_M	0x00ffffff
#define AR_SWITCH_TABLE_COM_2_ALL_S	0

/* Bits for AR_PHY_AGC_CONTROL. */
#define AR_PHY_AGC_CONTROL_CAL			0x00000001
#define AR_PHY_AGC_CONTROL_NF			0x00000002
#define AR_PHY_AGC_CONTROL_YCOK_MAX_M		0x000003c0
#define AR_PHY_AGC_CONTROL_YCOK_MAX_S		6
#define AR_PHY_AGC_CONTROL_OFFSET_CAL		0x00000800
#define AR_PHY_AGC_CONTROL_ENABLE_NF		0x00008000
#define AR_PHY_AGC_CONTROL_FLTR_CAL		0x00010000
#define AR_PHY_AGC_CONTROL_NO_UPDATE_NF		0x00020000
#define AR_PHY_AGC_CONTROL_EXT_NF_PWR_MEAS	0x00040000
#define AR_PHY_AGC_CONTROL_CLC_SUCCESS		0x00080000

/* Bits for AR_PHY_CALMODE. */
#define AR_PHY_CALMODE_IQ		0x00000000
#define AR_PHY_CALMODE_ADC_GAIN		0x00000001
#define AR_PHY_CALMODE_ADC_DC_PER	0x00000002
#define AR_PHY_CALMODE_ADC_DC_INIT	0x00000003

/* Bits for AR_PHY_FCAL_2_0. */
#define AR_PHY_FCAL20_CAP_STATUS_0_M	0x01f00000
#define AR_PHY_FCAL20_CAP_STATUS_0_S	20

/* Bits for AR_PHY_SYNTH_CONTROL. */
#define AR9380_BMODE	0x20000000

/* Bits for AR_PHY_ANALOG_SWAP. */
#define AR_PHY_SWAP_ALT_CHAIN	0x00000040

/* Bits for AR_PHY_ADDAC_PARA_CTL. */
#define AR_PHY_ADDAC_PARACTL_OFF_PWDADC	0x00008000

/* Bits for AR_PHY_TEST. */
#define AR_PHY_TEST_RFSILENT_BB		0x00002000
#define AR_PHY_TEST_BBB_OBS_SEL_M	0x00780000
#define AR_PHY_TEST_BBB_OBS_SEL_S	19
#define AR_PHY_TEST_RX_OBS_SEL_BIT5	0x00800000
#define AR_PHY_TEST_CHAIN_SEL_M		0xc0000000
#define AR_PHY_TEST_CHAIN_SEL_S		30

/* Bits for AR_PHY_TEST_CTL_STATUS. */
#define AR_PHY_TEST_CTL_TSTDAC_EN		0x00000001
#define AR_PHY_TEST_CTL_TX_OBS_SEL_M		0x0000001c
#define AR_PHY_TEST_CTL_TX_OBS_SEL_S		2
#define AR_PHY_TEST_CTL_TX_OBS_MUX_SEL_M	0x00000060
#define AR_PHY_TEST_CTL_TX_OBS_MUX_SEL_S	5
#define AR_PHY_TEST_CTL_TSTADC_EN		0x00000100
#define AR_PHY_TEST_CTL_RX_OBS_SEL_M		0x00003c00
#define AR_PHY_TEST_CTL_RX_OBS_SEL_S		10

/* Bits for AR_PHY_CHAN_INFO_MEMORY. */
#define AR_PHY_CHAN_INFO_MEMORY_CAPTURE_MASK	0x00000001
#define AR_PHY_CHAN_INFO_TAB_S2_READ		0x00000008

/* Bits for AR_PHY_CHAN_INFO_GAIN_0. */
#define AR_PHY_CHAN_INFO_GAIN_DIFF_PPM_MASK	0x00000fff
#define AR_PHY_CHAN_INFO_GAIN_DIFF_UPPER_LIMIT	320

/* Bits for AR_PHY_CCK_TX_CTRL. */
#define AR_PHY_CCK_TX_CTRL_JAPAN	0x00000010

/* Bits for AR_PHY_PWRTX_MAX. */
#define AR_PHY_POWER_TX_RATE_MAX_TPC_ENABLE	0x00000040

/* Bits for AR_PHY_TPCRG5(i). */
#define AR_PHY_TPCRG5_PD_GAIN_OVERLAP_M		0x0000000f
#define AR_PHY_TPCRG5_PD_GAIN_OVERLAP_S		0
#define AR_PHY_TPCRG5_PD_GAIN_BOUNDARY_1_M	0x000003f0
#define AR_PHY_TPCRG5_PD_GAIN_BOUNDARY_1_S	4
#define AR_PHY_TPCRG5_PD_GAIN_BOUNDARY_2_M	0x0000fc00
#define AR_PHY_TPCRG5_PD_GAIN_BOUNDARY_2_S	10
#define AR_PHY_TPCRG5_PD_GAIN_BOUNDARY_3_M	0x003f0000
#define AR_PHY_TPCRG5_PD_GAIN_BOUNDARY_3_S	16
#define AR_PHY_TPCRG5_PD_GAIN_BOUNDARY_4_M	0x0fc00000
#define AR_PHY_TPCRG5_PD_GAIN_BOUNDARY_4_S	22

/* Bits for AR_PHY_TPC_6_B(i). */
#define AR_PHY_TPC_6_ERROR_EST_MODE_M	0x03000000
#define AR_PHY_TPC_6_ERROR_EST_MODE_S	24

/* Bits for AR_PHY_TPC_11_B(i). */
#define AR_PHY_TPC_OLPC_GAIN_DELTA_M		0x00ff0000
#define AR_PHY_TPC_OLPC_GAIN_DELTA_S		16
#define AR_PHY_TPC_OLPC_GAIN_DELTA_PAL_ON_M	0xff000000
#define AR_PHY_TPC_OLPC_GAIN_DELTA_PAL_ON_S	24

/* Bits for AR_PHY_TPC_18. */
#define AR_PHY_TPC_18_THERM_CAL_VALUE_M		0x000000ff
#define AR_PHY_TPC_18_THERM_CAL_VALUE_S 	0

/* Bits for AR_PHY_TPC_19. */
#define AR_PHY_TPC_19_ALPHA_THERM_M		0x000000ff
#define AR_PHY_TPC_19_ALPHA_THERM_S		0

/* Bits for AR_PHY_BB_THERM_ADC_1. */
#define AR_PHY_BB_THERM_ADC_1_INIT_THERM_M	0x000000ff
#define AR_PHY_BB_THERM_ADC_1_INIT_THERM_S	0

/* Bits for AR_PHY_TX_FORCED_GAIN. */
#define AR_PHY_TXGAIN_FORCE			0x00000001
#define AR_PHY_TXGAIN_FORCED_TXBB1DBGAIN_M	0x0000000e
#define AR_PHY_TXGAIN_FORCED_TXBB1DBGAIN_S	1
#define AR_PHY_TXGAIN_FORCED_TXMXRGAIN_M	0x000003c0
#define AR_PHY_TXGAIN_FORCED_TXMXRGAIN_S	6
#define AR_PHY_TXGAIN_FORCED_PADVGNRA_M		0x00003c00
#define AR_PHY_TXGAIN_FORCED_PADVGNRA_S		10
#define AR_PHY_TXGAIN_FORCED_PADVGNRB_M		0x0003c000
#define AR_PHY_TXGAIN_FORCED_PADVGNRB_S		14
#define AR_PHY_TXGAIN_FORCED_PADVGNRD_M		0x00c00000
#define AR_PHY_TXGAIN_FORCED_PADVGNRD_S		22

/* Bits for AR_PHY_TX_IQCAL_CONTROL_1. */
#define AR_PHY_TX_IQCAQL_CONTROL_1_IQCORR_I_Q_COFF_DELPT_M	0x01fc0000
#define AR_PHY_TX_IQCAQL_CONTROL_1_IQCORR_I_Q_COFF_DELPT_S	18

/* Bits for AR_PHY_TX_IQCAL_START. */
#define AR_PHY_TX_IQCAL_START_DO_CAL	0x00000001

/* Bits for AR_PHY_TX_IQCAL_CORR_COEFF_01_B(i). */
#define AR_PHY_TX_IQCAL_CORR_COEFF_01_COEFF_TABLE_M	0x00003fff
#define AR_PHY_TX_IQCAL_CORR_COEFF_01_COEFF_TABLE_S	0

/* Bits for AR_PHY_TX_IQCAL_STATUS_B(i). */
#define AR_PHY_TX_IQCAL_STATUS_FAILED	0x00000001

/* Bits for AR_PHY_65NM_CH0_SYNTH4. */
#define AR_PHY_SYNTH4_LONG_SHIFT_SELECT	0x00000002

/* Bits for AR_PHY_65NM_CH0_SYNTH7. */
#define AR9380_FRACMODE		0x40000000
#define AR9380_LOAD_SYNTH	0x80000000

/* Bits for AR_PHY_65NM_CH0_RXTX4. */
#define AR_PHY_65NM_CH0_RXTX4_THERM_ON  0x10000000

/* Bits for AR_PHY_65NM_CH0_TOP. */
#define AR_PHY_65NM_CH0_TOP_XPABIASLVL_M	0x00000003
#define AR_PHY_65NM_CH0_TOP_XPABIASLVL_S	0	/* XXX Linux says 8 */

/* Bits for AR_PHY_65NM_CH0_THERM. */
#define AR_PHY_65NM_CH0_THERM_SPARE_M		0x0000003f
#define AR_PHY_65NM_CH0_THERM_SPARE_S		0
#define AR_PHY_65NM_CH0_THERM_SAR_ADC_OUT_M	0x0000ff00
#define AR_PHY_65NM_CH0_THERM_SAR_ADC_OUT_S	8
#define AR_PHY_65NM_CH0_THERM_START		0x20000000
#define AR_PHY_65NM_CH0_THERM_LOCAL		0x80000000


#define AR9003_RX_LP_QDEPTH	128
#define AR9003_RX_HP_QDEPTH	16

#define AR9003_NTXSTATUS	64

/* Maximum number of DMA segments per Tx descriptor. */
#define AR9003_MAX_SCATTER	4

/*
 * Tx DMA descriptor.
 */
struct ar_tx_desc {
	uint32_t	ds_info;
	uint32_t	ds_link;
	struct {
		uint32_t	ds_data;
		uint32_t	ds_ctl;
	} __packed	ds_segs[AR9003_MAX_SCATTER];
	uint32_t	ds_ctl10;
	uint32_t	ds_ctl11;
	uint32_t	ds_ctl12;
	uint32_t	ds_ctl13;
	uint32_t	ds_ctl14;
	uint32_t	ds_ctl15;
	uint32_t	ds_ctl16;
	uint32_t	ds_ctl17;
	uint32_t	ds_ctl18;
	uint32_t	ds_ctl19;
	uint32_t	ds_ctl20;
	uint32_t	ds_ctl21;
	uint32_t	ds_ctl22;
	/*
	 * Padding to make Tx descriptors 128 bytes such that they will
	 * not cross a 4KB boundary.
	 */
	uint32_t	pad[9];
} __packed;

/* Bits for ds_info. */
#define AR_TXI_DESC_NDWORDS_M		0x000000ff
#define AR_TXI_DESC_NDWORDS_S		0
#define AR_TXI_QCU_NUM_M		0x00000f00
#define AR_TXI_QCU_NUM_S		8
#define AR_TXI_CTRL_STAT		0x00004000
#define AR_TXI_DESC_TX			0x00008000
#define AR_TXI_DESC_ID_M		0xffff0000
#define AR_TXI_DESC_ID_S		16
#define AR_VENDOR_ATHEROS		0x168c	/* NB: PCI_VENDOR_ATHEROS */

/* Bits for ds_ctl. */
#define AR_TXC_BUF_LEN_M		0x0fff0000
#define AR_TXC_BUF_LEN_S		16

/* Bits for ds_ctl10. */
#define AR_TXC10_PTR_CHK_SUM_M		0x0000ffff
#define AR_TXC10_PTR_CHK_SUM_S		0

/* Bits for ds_ctl11. */
#define AR_TXC11_FRAME_LEN_M		0x00000fff
#define AR_TXC11_FRAME_LEN_S		0
#define AR_TXC11_XMIT_POWER_M		0x003f0000
#define AR_TXC11_XMIT_POWER_S		16
#define AR_TXC11_RTS_ENABLE		0x00400000
#define AR_TXC11_CLR_DEST_MASK		0x01000000
#define AR_TXC11_DEST_IDX_VALID		0x40000000
#define AR_TXC11_CTS_ENABLE		0x80000000

/* Bits for ds_ctl12. */
#define AR_TXC12_DEST_IDX_M		0x000fe000
#define AR_TXC12_DEST_IDX_S		13
#define AR_TXC12_FRAME_TYPE_M		0x00f00000
#define AR_TXC12_FRAME_TYPE_S		20
#define AR_FRAME_TYPE_NORMAL		0
#define AR_FRAME_TYPE_ATIM		1
#define AR_FRAME_TYPE_PSPOLL		2
#define AR_FRAME_TYPE_BEACON		3
#define AR_FRAME_TYPE_PROBE_RESP	4
#define AR_TXC12_NO_ACK			0x01000000

/* Bits for ds_ctl13. */
#define AR_TXC13_BURST_DUR_M		0x00007fff
#define AR_TXC13_BURST_DUR_S		0
#define AR_TXC13_DUR_UPDATE_ENA		0x00008000
#define AR_TXC13_XMIT_DATA_TRIES0_M	0x000f0000
#define AR_TXC13_XMIT_DATA_TRIES0_S	16
#define AR_TXC13_XMIT_DATA_TRIES1_M	0x00f00000
#define AR_TXC13_XMIT_DATA_TRIES1_S	20
#define AR_TXC13_XMIT_DATA_TRIES2_M	0x0f000000
#define AR_TXC13_XMIT_DATA_TRIES2_S	24
#define AR_TXC13_XMIT_DATA_TRIES3_M	0xf0000000
#define AR_TXC13_XMIT_DATA_TRIES3_S	28

/* Bits for ds_ctl14. */
#define AR_TXC14_XMIT_RATE0_M		0x000000ff
#define AR_TXC14_XMIT_RATE0_S		0
#define AR_TXC14_XMIT_RATE1_M		0x0000ff00
#define AR_TXC14_XMIT_RATE1_S		8
#define AR_TXC14_XMIT_RATE2_M		0x00ff0000
#define AR_TXC14_XMIT_RATE2_S		16
#define AR_TXC14_XMIT_RATE3_M		0xff000000
#define AR_TXC14_XMIT_RATE3_S		24

/* Bits for ds_ctl15. */
#define AR_TXC15_PACKET_DUR0_M		0x00007fff
#define AR_TXC15_PACKET_DUR0_S		0
#define AR_TXC15_RTSCTS_QUAL0		0x00008000
#define AR_TXC15_PACKET_DUR1_M		0x7fff0000
#define AR_TXC15_PACKET_DUR1_S		16
#define AR_TXC15_RTSCTS_QUAL1		0x80000000
/* Shortcut. */
#define AR_TXC15_RTSCTS_QUAL01	\
	(AR_TXC15_RTSCTS_QUAL0 | AR_TXC15_RTSCTS_QUAL1)

/* Bits for ds_ctl16. */
#define AR_TXC16_PACKET_DUR2_M		0x00007fff
#define AR_TXC16_PACKET_DUR2_S		0
#define AR_TXC16_RTSCTS_QUAL2		0x00008000
#define AR_TXC16_PACKET_DUR3_M		0x7fff0000
#define AR_TXC16_PACKET_DUR3_S		16
#define AR_TXC16_RTSCTS_QUAL3		0x80000000
/* Shortcut. */
#define AR_TXC16_RTSCTS_QUAL23	\
	(AR_TXC16_RTSCTS_QUAL2 | AR_TXC16_RTSCTS_QUAL3)

/* Bits for ds_ctl17. */
#define AR_TXC17_ENCR_TYPE_M		0x0c000000
#define AR_TXC17_ENCR_TYPE_S		26
#define AR_ENCR_TYPE_CLEAR		0
#define AR_ENCR_TYPE_WEP		1
#define AR_ENCR_TYPE_AES		2
#define AR_ENCR_TYPE_TKIP		3

/* Bits for ds_ctl18. */
#define AR_TXC18_2040_0			0x00000001
#define AR_TXC18_GI0			0x00000002
#define AR_TXC18_CHAIN_SEL0_M		0x0000001c
#define AR_TXC18_CHAIN_SEL0_S		2
#define AR_TXC18_2040_1			0x00000020
#define AR_TXC18_GI1			0x00000040
#define AR_TXC18_CHAIN_SEL1_M		0x00000380
#define AR_TXC18_CHAIN_SEL1_S		7
#define AR_TXC18_2040_2			0x00000400
#define AR_TXC18_GI2			0x00000800
#define AR_TXC18_CHAIN_SEL2_M		0x00007000
#define AR_TXC18_CHAIN_SEL2_S		12
#define AR_TXC18_2040_3			0x00008000
#define AR_TXC18_GI3			0x00010000
#define AR_TXC18_CHAIN_SEL3_M		0x000e0000
#define AR_TXC18_CHAIN_SEL3_S		17
#define AR_TXC18_RTSCTS_RATE_M		0x0ff00000
#define AR_TXC18_RTSCTS_RATE_S		20
/* Shortcuts. */
#define AR_TXC18_2040_0123	\
	(AR_TXC18_2040_0 | AR_TXC18_2040_1 | AR_TXC18_2040_2 | AR_TXC18_2040_3)
#define AR_TXC18_GI0123		\
	(AR_TXC18_GI0 | AR_TXC18_GI1 | AR_TXC18_GI2 | AR_TXC18_GI3)

/* Bits for ds_ctl19. */
#define AR_TXC19_NOT_SOUNDING		0x20000000


/*
 * Tx status DMA descriptor.
 */
struct ar_tx_status {
	uint32_t	ds_info;
	uint32_t	ds_status1;
	uint32_t	ds_status2;
	uint32_t	ds_status3;
	uint32_t	ds_status4;
	uint32_t	ds_status5;
	uint32_t	ds_status6;
	uint32_t	ds_status7;
	uint32_t	ds_status8;
} __packed;

/* Bits for ds_status3. */
#define AR_TXS3_EXCESSIVE_RETRIES	0x00000002
#define AR_TXS3_FIFO_UNDERRUN		0x00000004
#define AR_TXS3_RTS_FAIL_CNT_M		0x000000f0
#define AR_TXS3_RTS_FAIL_CNT_S		4
#define AR_TXS3_DATA_FAIL_CNT_M		0x00000f00
#define AR_TXS3_DATA_FAIL_CNT_S		8
#define AR_TXS3_TX_DELIM_UNDERRUN	0x00010000
#define AR_TXS3_TX_DATA_UNDERRUN	0x00020000
/* Shortcut. */
#define AR_TXS3_UNDERRUN		\
	(AR_TXS3_FIFO_UNDERRUN |	\
	 AR_TXS3_TX_DELIM_UNDERRUN |	\
	 AR_TXS3_TX_DATA_UNDERRUN)

/* Bits for ds_status8. */
#define AR_TXS8_DONE			0x00000001
#define AR_TXS8_FINAL_IDX_M		0x00600000
#define AR_TXS8_FINAL_IDX_S		21

/*
 * Rx status DMA descriptor.
 */
struct ar_rx_status {
	uint32_t	ds_info;
	uint32_t	ds_status1;
	uint32_t	ds_status2;
	uint32_t	ds_status3;
	uint32_t	ds_status4;
	uint32_t	ds_status5;
	uint32_t	ds_status6;
	uint32_t	ds_status7;
	uint32_t	ds_status8;
	uint32_t	ds_status9;
	uint32_t	ds_status10;
	uint32_t	ds_status11;
} __packed;

/* Bits for ds_info. */
#define AR_RXI_CTRL_STAT		0x00004000
#define AR_RXI_DESC_TX			0x00008000
#define AR_RXI_DESC_ID_M		0xffff0000
#define AR_RXI_DESC_ID_S		16

/* Bits for ds_status1. */
#define AR_RXS1_DONE			0x00000001
#define AR_RXS1_RATE_M			0x000003fc
#define AR_RXS1_RATE_S			2

/* Bits for ds_status2. */
#define AR_RXS2_DATA_LEN_M		0x00000fff
#define AR_RXS2_DATA_LEN_S		0

/* Bits for ds_status4. */
#define AR_RXS4_GI			0x00000001
#define AR_RXS4_ANTENNA_M		0xffffff00
#define AR_RXS4_ANTENNA_S		8

/* Bits for ds_status5. */
#define AR_RXS5_RSSI_COMBINED_M		0xff000000
#define AR_RXS5_RSSI_COMBINED_S		24

/* Bits for ds_status11. */
#define AR_RXS11_FRAME_OK		0x00000002
#define AR_RXS11_CRC_ERR		0x00000004
#define AR_RXS11_DECRYPT_CRC_ERR	0x00000008
#define AR_RXS11_PHY_ERR		0x00000010
#define AR_RXS11_PHY_ERR_CODE_M		0x0000ff00
#define AR_RXS11_PHY_ERR_CODE_S		8
#define AR_RXS11_MICHAEL_ERR		0x00000020

/*
 * AR9003 family common ROM structures.
 */
#define AR_EEP_COMPRESS_NONE	0
#define AR_EEP_COMPRESS_LZMA	1
#define AR_EEP_COMPRESS_PAIRS	2
#define AR_EEP_COMPRESS_BLOCK	3

struct ar_cal_target_power_leg {
	uint8_t	tPow2x[4];
} __packed;

struct ar_cal_target_power_ht {
	uint8_t	tPow2x[14];
} __packed;
