/*	$OpenBSD: src/sys/dev/ic/ar9380.c,v 1.6 2010/10/18 16:05:28 damien Exp $	*/

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
 * Driver for Atheros 802.11a/g/n chipsets.
 * Routines for AR9380 and AR9381 chipsets.
 */

#include "bpfilter.h"

#include <sys/param.h>
#include <sys/sockio.h>
#include <sys/mbuf.h>
#include <sys/kernel.h>
#include <sys/socket.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/queue.h>
#include <sys/conf.h>
#include <sys/device.h>

#include <machine/bus.h>
#include <machine/endian.h>

#if NBPFILTER > 0
#include <net/bpf.h>
#endif
#include <net/if.h>
#include <net/if_arp.h>
#include <net/if_dl.h>
#include <net/if_media.h>
#include <net/if_types.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/if_ether.h>

#include <net80211/ieee80211_var.h>
#include <net80211/ieee80211_amrr.h>
#include <net80211/ieee80211_radiotap.h>

#include <dev/ic/athnreg.h>
#include <dev/ic/athnvar.h>

#include <dev/ic/ar9003reg.h>
#include <dev/ic/ar9380reg.h>

int	ar9380_attach(struct athn_softc *);
void	ar9380_setup(struct athn_softc *);
void	ar9380_swap_rom(struct athn_softc *);
int	ar9380_set_synth(struct athn_softc *, struct ieee80211_channel *,
	    struct ieee80211_channel *);
void	ar9380_get_paprd_masks(struct athn_softc *, struct ieee80211_channel *,
	    uint32_t *, uint32_t *);
void	ar9380_init_from_rom(struct athn_softc *, struct ieee80211_channel *,
	    struct ieee80211_channel *);
void	ar9380_spur_mitigate_cck(struct athn_softc *,
	    struct ieee80211_channel *, struct ieee80211_channel *);
void	ar9380_spur_mitigate_ofdm(struct athn_softc *,
	    struct ieee80211_channel *, struct ieee80211_channel *);
void	ar9380_spur_mitigate(struct athn_softc *, struct ieee80211_channel *,
	    struct ieee80211_channel *);
void	ar9380_set_txpower(struct athn_softc *, struct ieee80211_channel *,
	    struct ieee80211_channel *);
void	ar9380_get_correction(struct athn_softc *, struct ieee80211_channel *,
	    int, int *, int *);
void	ar9380_set_correction(struct athn_softc *, struct ieee80211_channel *);

/* Extern functions. */
int	athn_interpolate(int, int, int, int, int);
uint8_t	athn_chan2fbin(struct ieee80211_channel *);
void	athn_get_pier_ival(uint8_t, const uint8_t *, int, int *, int *);
int	ar9003_attach(struct athn_softc *);
void	ar9003_write_txpower(struct athn_softc *, int16_t power[]);
void	ar9003_get_lg_tpow(struct athn_softc *, struct ieee80211_channel *,
	    uint8_t, const uint8_t *, const struct ar_cal_target_power_leg *,
	    int, uint8_t[]);
void	ar9003_get_ht_tpow(struct athn_softc *, struct ieee80211_channel *,
	    uint8_t, const uint8_t *, const struct ar_cal_target_power_ht *,
	    int, uint8_t[]);


int
ar9380_attach(struct athn_softc *sc)
{
	sc->eep_base = AR9380_EEP_START_LOC;
	sc->eep_size = sizeof(struct ar9380_eeprom);
	sc->eep_def = ar9380_def_rom;
	sc->ngpiopins = 17;
	sc->ops.setup = ar9380_setup;
	sc->ops.swap_rom = ar9380_swap_rom;
	sc->ops.init_from_rom = ar9380_init_from_rom;
	sc->ops.set_txpower = ar9380_set_txpower;
	sc->ops.set_synth = ar9380_set_synth;
	sc->ops.spur_mitigate = ar9380_spur_mitigate;
	sc->ops.get_paprd_masks = ar9380_get_paprd_masks;
	sc->cca_min_2g = AR9380_PHY_CCA_MIN_GOOD_VAL_2GHZ;
	sc->cca_max_2g = AR9380_PHY_CCA_MAX_GOOD_VAL_2GHZ;
	sc->cca_min_5g = AR9380_PHY_CCA_MIN_GOOD_VAL_5GHZ;
	sc->cca_max_5g = AR9380_PHY_CCA_MAX_GOOD_VAL_5GHZ;
	sc->ini = &ar9380_2_2_ini;

	return (ar9003_attach(sc));
}

void
ar9380_setup(struct athn_softc *sc)
{
	struct ieee80211com *ic = &sc->sc_ic;
	struct ar9380_eeprom *eep = sc->eep;
	struct ar9380_base_eep_hdr *base = &eep->baseEepHeader;
	uint8_t type;

	if (base->opFlags & AR_OPFLAGS_11A)
		sc->flags |= ATHN_FLAG_11A;
	if (base->opFlags & AR_OPFLAGS_11G)
		sc->flags |= ATHN_FLAG_11G;
	if (base->opFlags & AR_OPFLAGS_11N)
		sc->flags |= ATHN_FLAG_11N;

	IEEE80211_ADDR_COPY(ic->ic_myaddr, eep->macAddr);
	sc->led_pin = base->wlanLedGpio;

	/* Check if we have a hardware radio switch. */
	if (base->rfSilent & AR_EEP_RFSILENT_ENABLED) {
		sc->flags |= ATHN_FLAG_RFSILENT;
		/* Get GPIO pin used by hardware radio switch. */
		sc->rfsilent_pin = base->wlanDisableGpio;
	}

	/* Set the number of HW key cache entries. */
	sc->kc_entries = AR_KEYTABLE_SIZE;

	sc->txchainmask = MS(base->txrxMask, AR_EEP_TX_MASK);
	sc->rxchainmask = MS(base->txrxMask, AR_EEP_RX_MASK);

	/* Fast PLL clock is always supported. */
	sc->flags |= ATHN_FLAG_FAST_PLL_CLOCK;

	/* Enable PA predistortion if supported. */
	if (base->featureEnable & AR_EEP_PAPRD)
		sc->flags |= ATHN_FLAG_PAPRD;

	/* Select initialization values based on ROM. */
	type = MS(eep->baseEepHeader.txrxgain, AR_EEP_RX_GAIN);
	if (type == AR_EEP_RX_GAIN_WO_XLNA)
		sc->rx_gain = &ar9380_2_2_rx_gain_wo_xlna;
	else
		sc->rx_gain = &ar9380_2_2_rx_gain;

	/* Select initialization values based on ROM. */
	type = MS(eep->baseEepHeader.txrxgain, AR_EEP_TX_GAIN);
	if (type == AR_EEP_TX_GAIN_HIGH_OB_DB)
		sc->tx_gain = &ar9380_2_2_tx_gain_high_ob_db;
	else if (type == AR_EEP_TX_GAIN_LOW_OB_DB)
		sc->tx_gain = &ar9380_2_2_tx_gain_low_ob_db;
	else
		sc->tx_gain = &ar9380_2_2_tx_gain;
}

void
ar9380_swap_rom(struct athn_softc *sc)
{
#if BYTE_ORDER == BIG_ENDIAN
	struct ar9380_eeprom *eep = sc->eep;
	struct ar9380_base_eep_hdr *base = &eep->baseEepHeader;
	struct ar9380_modal_eep_header *modal;
	int i;

	base->regDmn[0] = swap16(base->regDmn[0]);
	base->regDmn[1] = swap16(base->regDmn[1]);
	base->swreg = swap32(base->swreg);

	modal = &eep->modalHeader2G;
	modal->antCtrlCommon = swap32(modal->antCtrlCommon);
	modal->antCtrlCommon2 = swap32(modal->antCtrlCommon2);
	modal->papdRateMaskHt20 = swap32(modal->papdRateMaskHt20);
	modal->papdRateMaskHt40 = swap32(modal->papdRateMaskHt40);
	for (i = 0; i < AR9380_MAX_CHAINS; i++)
		modal->antCtrlChain[i] = swap16(modal->antCtrlChain[i]);

	modal = &eep->modalHeader5G;
	modal->antCtrlCommon = swap32(modal->antCtrlCommon);
	modal->antCtrlCommon2 = swap32(modal->antCtrlCommon2);
	modal->papdRateMaskHt20 = swap32(modal->papdRateMaskHt20);
	modal->papdRateMaskHt40 = swap32(modal->papdRateMaskHt40);
	for (i = 0; i < AR9380_MAX_CHAINS; i++)
		modal->antCtrlChain[i] = swap16(modal->antCtrlChain[i]);
#endif
}

void
ar9380_get_paprd_masks(struct athn_softc *sc, struct ieee80211_channel *c,
    uint32_t *ht20mask, uint32_t *ht40mask)
{
	const struct ar9380_eeprom *eep = sc->eep;
	const struct ar9380_modal_eep_header *modal;

	if (IEEE80211_IS_CHAN_2GHZ(c))
		modal = &eep->modalHeader2G;
	else
		modal = &eep->modalHeader5G;
	*ht20mask = modal->papdRateMaskHt20;
	*ht40mask = modal->papdRateMaskHt40;
}

int
ar9380_set_synth(struct athn_softc *sc, struct ieee80211_channel *c,
    struct ieee80211_channel *extc)
{
	uint32_t freq = c->ic_freq;
	uint32_t chansel, phy;

	if (IEEE80211_IS_CHAN_2GHZ(c)) {
		chansel = (freq << 16) / 15;
		AR_WRITE(sc, AR_PHY_SYNTH_CONTROL, AR9380_BMODE);
	} else {
		chansel = (freq << 15) / 15;
		chansel >>= 1;
		AR_WRITE(sc, AR_PHY_SYNTH_CONTROL, 0);
	}

	/* Enable Long Shift Select for synthesizer. */
	AR_SETBITS(sc, AR_PHY_65NM_CH0_SYNTH4,
	    AR_PHY_SYNTH4_LONG_SHIFT_SELECT);

	/* Program synthesizer. */
	phy = (chansel << 2) | AR9380_FRACMODE;
	DPRINTFN(4, ("AR_PHY_65NM_CH0_SYNTH7=0x%08x\n", phy));
	AR_WRITE(sc, AR_PHY_65NM_CH0_SYNTH7, phy);
	/* Toggle Load Synth Channel bit. */
	AR_WRITE(sc, AR_PHY_65NM_CH0_SYNTH7, phy | AR9380_LOAD_SYNTH);
	return (0);
}

void
ar9380_init_from_rom(struct athn_softc *sc, struct ieee80211_channel *c,
    struct ieee80211_channel *extc)
{
	const struct ar9380_eeprom *eep = sc->eep;
	const struct ar9380_modal_eep_header *modal;
	uint32_t reg;
	int i;

	if (IEEE80211_IS_CHAN_2GHZ(c))
		modal = &eep->modalHeader2G;
	else
		modal = &eep->modalHeader5G;

	/* Apply XPA bias level. */
	reg = AR_READ(sc, AR_PHY_65NM_CH0_TOP);
	reg = RW(reg, AR_PHY_65NM_CH0_TOP_XPABIASLVL,
	    MS(modal->xpaBiasLvl, AR_EEP_XPABIASLVL));
	AR_WRITE(sc, AR_PHY_65NM_CH0_TOP, reg);
	reg = AR_READ(sc, AR_PHY_65NM_CH0_THERM);
	reg = RW(reg, AR_PHY_65NM_CH0_THERM_SPARE,
	    MS(modal->xpaBiasLvl, AR_EEP_THERM_SPARE));
	AR_WRITE(sc, AR_PHY_65NM_CH0_THERM, reg);

	/* Apply antenna control. */
	reg = AR_READ(sc, AR_PHY_SWITCH_COM);
	reg = RW(reg, AR_SWITCH_TABLE_COM_ALL, modal->antCtrlCommon);
	AR_WRITE(sc, AR_PHY_SWITCH_COM, reg);
	reg = AR_READ(sc, AR_PHY_SWITCH_COM_2);
	reg = RW(reg, AR_SWITCH_TABLE_COM_2_ALL, modal->antCtrlCommon2);
	AR_WRITE(sc, AR_PHY_SWITCH_COM_2, reg);

	for (i = 0; i < AR9380_MAX_CHAINS; i++) {
		reg = AR_READ(sc, AR_PHY_SWITCH_CHAIN(i));
		reg = RW(reg, AR_SWITCH_TABLE_ALL, modal->antCtrlChain[i]);
		AR_WRITE(sc, AR_PHY_SWITCH_CHAIN(i), reg);
	}

	if (eep->baseEepHeader.miscConfiguration & AR_EEP_DRIVE_STRENGTH) {
		/* Apply drive strength. */
		reg = AR_READ(sc, AR_PHY_65NM_CH0_BIAS1);
		reg = (reg & ~0x00ffffc0) | 0x00b6db40;
		AR_WRITE(sc, AR_PHY_65NM_CH0_BIAS1, reg);

		reg = AR_READ(sc, AR_PHY_65NM_CH0_BIAS2);
		reg = (reg & ~0xffffffe0) | 0xb6db6da0;
		AR_WRITE(sc, AR_PHY_65NM_CH0_BIAS2, reg);

		reg = AR_READ(sc, AR_PHY_65NM_CH0_BIAS4);
		reg = (reg & ~0xff800000) | 0xb6800000;
		AR_WRITE(sc, AR_PHY_65NM_CH0_BIAS4, reg);
	}

	if (eep->baseEepHeader.featureEnable & AR_EEP_INTERNAL_REGULATOR) {
		/* Internal regulator is ON. */
		AR_CLRBITS(sc, AR_RTC_REG_CONTROL1,
		    AR_RTC_REG_CONTROL1_SWREG_PROGRAM);
		AR_WRITE(sc, AR_RTC_REG_CONTROL0, eep->baseEepHeader.swreg);
		AR_SETBITS(sc, AR_RTC_REG_CONTROL1,
		    AR_RTC_REG_CONTROL1_SWREG_PROGRAM);
	} else
		AR_SETBITS(sc, AR_RTC_SLEEP_CLK, AR_RTC_FORCE_SWREG_PRD);
}

void
ar9380_spur_mitigate_cck(struct athn_softc *sc, struct ieee80211_channel *c,
    struct ieee80211_channel *extc)
{
	/* NB: It is safe to call this function for 5GHz channels. */
	static const int16_t freqs[] = { 2420, 2440, 2464, 2480 };
	int i, spur, freq;
	uint32_t reg;

	for (i = 0; i < nitems(freqs); i++) {
		spur = freqs[i] - c->ic_freq;
		if (abs(spur) < 10)	/* +/- 10MHz range. */
			break;
	}
	if (i == nitems(freqs)) {
		/* Disable CCK spur mitigation. */
		reg = AR_READ(sc, AR_PHY_AGC_CONTROL);
		reg = RW(reg, AR_PHY_AGC_CONTROL_YCOK_MAX, 0x5);
		AR_WRITE(sc, AR_PHY_AGC_CONTROL, reg);
		reg = AR_READ(sc, AR_PHY_CCK_SPUR_MIT);
		reg = RW(reg, AR_PHY_CCK_SPUR_MIT_CCK_SPUR_FREQ, 0);
		reg &= ~AR_PHY_CCK_SPUR_MIT_USE_CCK_SPUR_MIT;
		AR_WRITE(sc, AR_PHY_CCK_SPUR_MIT, reg);
		return;
	}
	freq = (spur * 524288) / 11;

	reg = AR_READ(sc, AR_PHY_AGC_CONTROL);
	reg = RW(reg, AR_PHY_AGC_CONTROL_YCOK_MAX, 0x7);
	AR_WRITE(sc, AR_PHY_AGC_CONTROL, reg);

	reg = AR_READ(sc, AR_PHY_CCK_SPUR_MIT);
	reg = RW(reg, AR_PHY_CCK_SPUR_MIT_CCK_SPUR_FREQ, freq);
	reg = RW(reg, AR_PHY_CCK_SPUR_MIT_SPUR_RSSI_THR, 0x7f);
	reg = RW(reg, AR_PHY_CCK_SPUR_MIT_SPUR_FILTER_TYPE, 0x2);
	reg |= AR_PHY_CCK_SPUR_MIT_USE_CCK_SPUR_MIT;
	AR_WRITE(sc, AR_PHY_CCK_SPUR_MIT, reg);
}

void
ar9380_spur_mitigate_ofdm(struct athn_softc *sc, struct ieee80211_channel *c,
    struct ieee80211_channel *extc)
{
	const struct ar9380_eeprom *eep;
	const uint8_t *spurchans;
	uint32_t reg;
	int idx, spur_delta_phase, spur_off, range, i;
	int freq, spur, spur_freq_sd, spur_subchannel_sd;

	if (IEEE80211_IS_CHAN_2GHZ(c))
		spurchans = eep->modalHeader2G.spurChans;
	else
		spurchans = eep->modalHeader5G.spurChans;
	if (spurchans[0] == 0)
		return;

	/* Disable OFDM spur mitigation. */
	AR_CLRBITS(sc, AR_PHY_TIMING4, AR_PHY_TIMING4_ENABLE_SPUR_FILTER);

	reg = AR_READ(sc, AR_PHY_TIMING11);
	reg = RW(reg, AR_PHY_TIMING11_SPUR_FREQ_SD, 0);
	reg = RW(reg, AR_PHY_TIMING11_SPUR_DELTA_PHASE, 0);
	reg &= ~AR_PHY_TIMING11_USE_SPUR_FILTER_IN_AGC;
	reg &= ~AR_PHY_TIMING11_USE_SPUR_FILTER_IN_SELFCOR;
	AR_WRITE(sc, AR_PHY_TIMING11, reg);

	AR_CLRBITS(sc, AR_PHY_SFCORR_EXT,
	    AR_PHY_SFCORR_EXT_SPUR_SUBCHANNEL_SD);

	AR_CLRBITS(sc, AR_PHY_TIMING4, AR_PHY_TIMING4_ENABLE_SPUR_RSSI);

	reg = AR_READ(sc, AR_PHY_SPUR_REG);
	reg = RW(reg, AR_PHY_SPUR_REG_MASK_RATE_CNTL, 0);
	reg &= ~AR_PHY_SPUR_REG_EN_VIT_SPUR_RSSI;
	reg &= ~AR_PHY_SPUR_REG_ENABLE_NF_RSSI_SPUR_MIT;
	reg &= ~AR_PHY_SPUR_REG_ENABLE_MASK_PPM;
	AR_WRITE(sc, AR_PHY_SPUR_REG, reg);

	freq = c->ic_freq;
#ifndef IEEE80211_NO_HT
	if (extc != NULL) {
		range = 19;	/* +/- 19MHz range. */
		if (AR_READ(sc, AR_PHY_GEN_CTRL) & AR_PHY_GC_DYN2040_PRI_CH)
			freq += 10;
		else
			freq -= 10;
	} else
#endif
		range = 10;	/* +/- 10MHz range. */
	for (i = 0; i < AR9380_EEPROM_MODAL_SPURS; i++) {
		spur = spurchans[i];
		if (spur == 0)
			return;
		/* Convert to frequency. */
		if (IEEE80211_IS_CHAN_2GHZ(c))
			spur = 2300 + spur;
		else
			spur = 4900 + (spur * 5);
		spur -= freq;
		if (abs(spur) < range)
			break;
	}
	if (i == AR9380_EEPROM_MODAL_SPURS)
		return;

	/* Enable OFDM spur mitigation. */
#ifndef IEEE80211_NO_HT
	if (extc != NULL) {
		spur_delta_phase = (spur * 131072) / 5;
		reg = AR_READ(sc, AR_PHY_GEN_CTRL);
		if (spur < 0) {
			spur_subchannel_sd =
			    (reg & AR_PHY_GC_DYN2040_PRI_CH) == 0;
			spur_off = spur + 10;
		} else {
			spur_subchannel_sd =
			    (reg & AR_PHY_GC_DYN2040_PRI_CH) != 0;
			spur_off = spur - 10;
		}
	} else
#endif
	{
		spur_delta_phase = (spur * 262144) / 5;
		spur_subchannel_sd = 0;
		spur_off = spur;
	}
	spur_freq_sd = (spur_off * 512) / 11;

	AR_SETBITS(sc, AR_PHY_TIMING4, AR_PHY_TIMING4_ENABLE_SPUR_FILTER);

	reg = AR_READ(sc, AR_PHY_TIMING11);
	reg = RW(reg, AR_PHY_TIMING11_SPUR_FREQ_SD, spur_freq_sd);
	reg = RW(reg, AR_PHY_TIMING11_SPUR_DELTA_PHASE, spur_delta_phase);
	reg |= AR_PHY_TIMING11_USE_SPUR_FILTER_IN_AGC;
	reg |= AR_PHY_TIMING11_USE_SPUR_FILTER_IN_SELFCOR;
	AR_WRITE(sc, AR_PHY_TIMING11, reg);

	reg = AR_READ(sc, AR_PHY_SFCORR_EXT);
	if (spur_subchannel_sd)
		reg |= AR_PHY_SFCORR_EXT_SPUR_SUBCHANNEL_SD;
	else
		reg &= ~AR_PHY_SFCORR_EXT_SPUR_SUBCHANNEL_SD;
	AR_WRITE(sc, AR_PHY_SFCORR_EXT, reg);

	AR_SETBITS(sc, AR_PHY_TIMING4, AR_PHY_TIMING4_ENABLE_SPUR_RSSI);

	reg = AR_READ(sc, AR_PHY_SPUR_REG);
	reg = RW(reg, AR_PHY_SPUR_REG_MASK_RATE_CNTL, 0xff);
	reg = RW(reg, AR_PHY_SPUR_REG_SPUR_RSSI_THRESH, 34);
	reg |= AR_PHY_SPUR_REG_EN_VIT_SPUR_RSSI;
	if (AR_READ(sc, AR_PHY_MODE) & AR_PHY_MODE_DYNAMIC)
		reg |= AR_PHY_SPUR_REG_ENABLE_NF_RSSI_SPUR_MIT;
	reg |= AR_PHY_SPUR_REG_ENABLE_MASK_PPM;
	AR_WRITE(sc, AR_PHY_SPUR_REG, reg);

	idx = (spur * 16) / 5;
	if (idx < 0)
		idx--;

	/* Write pilot mask. */
	AR_SETBITS(sc, AR_PHY_TIMING4,
	    AR_PHY_TIMING4_ENABLE_PILOT_MASK |
	    AR_PHY_TIMING4_ENABLE_CHAN_MASK);

	reg = AR_READ(sc, AR_PHY_PILOT_SPUR_MASK);
	reg = RW(reg, AR_PHY_PILOT_SPUR_MASK_CF_PILOT_MASK_IDX_A, idx);
	reg = RW(reg, AR_PHY_PILOT_SPUR_MASK_CF_PILOT_MASK_A, 0x0c);
	AR_WRITE(sc, AR_PHY_PILOT_SPUR_MASK, reg);

	reg = AR_READ(sc, AR_PHY_SPUR_MASK_A);
	reg = RW(reg, AR_PHY_SPUR_MASK_A_CF_PUNC_MASK_IDX_A, idx);
	reg = RW(reg, AR_PHY_SPUR_MASK_A_CF_PUNC_MASK_A, 0xa0);
	AR_WRITE(sc, AR_PHY_SPUR_MASK_A, reg);

	reg = AR_READ(sc, AR_PHY_CHAN_SPUR_MASK);
	reg = RW(reg, AR_PHY_CHAN_SPUR_MASK_CF_CHAN_MASK_IDX_A, idx);
	reg = RW(reg, AR_PHY_CHAN_SPUR_MASK_CF_CHAN_MASK_A, 0x0c);
	AR_WRITE(sc, AR_PHY_CHAN_SPUR_MASK, reg);
}

void
ar9380_spur_mitigate(struct athn_softc *sc, struct ieee80211_channel *c,
    struct ieee80211_channel *extc)
{
	/* NB: We call spur_mitigate_cck for 5GHz too, just to disable it. */
	ar9380_spur_mitigate_cck(sc, c, extc);
	ar9380_spur_mitigate_ofdm(sc, c, extc);
}

void
ar9380_set_txpower(struct athn_softc *sc, struct ieee80211_channel *c,
    struct ieee80211_channel *extc)
{
	const struct ar9380_eeprom *eep = sc->eep;
	uint8_t tpow_cck[4], tpow_ofdm[4];
	uint8_t tpow_ht20[14], tpow_ht40[14];
	int16_t power[ATHN_POWER_COUNT];

	if (IEEE80211_IS_CHAN_2GHZ(c)) {
		/* Get CCK target powers. */
		ar9003_get_lg_tpow(sc, c, AR_CTL_11B,
		    eep->calTargetFbinCck, eep->calTargetPowerCck,
		    AR9380_NUM_2G_CCK_TARGET_POWERS, tpow_cck);

		/* Get OFDM target powers. */
		ar9003_get_lg_tpow(sc, c, AR_CTL_11G,
		    eep->calTargetFbin2G, eep->calTargetPower2G,
		    AR9380_NUM_2G_20_TARGET_POWERS, tpow_ofdm);

		/* Get HT-20 target powers. */
		ar9003_get_ht_tpow(sc, c, AR_CTL_2GHT20,
		    eep->calTargetFbin2GHT20, eep->calTargetPower2GHT20,
		    AR9380_NUM_2G_20_TARGET_POWERS, tpow_ht20);

		if (extc != NULL) {
			/* Get HT-40 target powers. */
			ar9003_get_ht_tpow(sc, c, AR_CTL_2GHT40,
			    eep->calTargetFbin2GHT40,
			    eep->calTargetPower2GHT40,
			    AR9380_NUM_2G_40_TARGET_POWERS, tpow_ht40);
		}
	} else {
		/* Get OFDM target powers. */
		ar9003_get_lg_tpow(sc, c, AR_CTL_11A,
		    eep->calTargetFbin5G, eep->calTargetPower5G,
		    AR9380_NUM_5G_20_TARGET_POWERS, tpow_ofdm);

		/* Get HT-20 target powers. */
		ar9003_get_ht_tpow(sc, c, AR_CTL_5GHT20,
		    eep->calTargetFbin5GHT20, eep->calTargetPower5GHT20,
		    AR9380_NUM_5G_20_TARGET_POWERS, tpow_ht20);

		if (extc != NULL) {
			/* Get HT-40 target powers. */
			ar9003_get_ht_tpow(sc, c, AR_CTL_5GHT40,
			    eep->calTargetFbin5GHT40,
			    eep->calTargetPower5GHT40,
			    AR9380_NUM_5G_40_TARGET_POWERS, tpow_ht40);
		}
	}

	memset(power, 0, sizeof(power));
	/* Shuffle target powers accross transmit rates. */
	power[ATHN_POWER_OFDM6 ] =
	power[ATHN_POWER_OFDM9 ] =
	power[ATHN_POWER_OFDM12] =
	power[ATHN_POWER_OFDM18] =
	power[ATHN_POWER_OFDM24] = tpow_ofdm[0];
	power[ATHN_POWER_OFDM36] = tpow_ofdm[1];
	power[ATHN_POWER_OFDM48] = tpow_ofdm[2];
	power[ATHN_POWER_OFDM54] = tpow_ofdm[3];
	if (IEEE80211_IS_CHAN_2GHZ(c)) {
		power[ATHN_POWER_CCK1_LP ] =
		power[ATHN_POWER_CCK2_LP ] =
		power[ATHN_POWER_CCK2_SP ] =
		power[ATHN_POWER_CCK55_LP] = tpow_cck[0];
		power[ATHN_POWER_CCK55_SP] = tpow_cck[1];
		power[ATHN_POWER_CCK11_LP] = tpow_cck[2];
		power[ATHN_POWER_CCK11_SP] = tpow_cck[3];
	}
	/* Next entry covers MCS0, MCS8 and MCS16. */
	power[ATHN_POWER_HT20( 0)] = tpow_ht20[ 0];
	/* Next entry covers MCS1-3, MCS9-11 and MCS17-19. */
	power[ATHN_POWER_HT20( 1)] = tpow_ht20[ 1];
	power[ATHN_POWER_HT20( 4)] = tpow_ht20[ 2];
	power[ATHN_POWER_HT20( 5)] = tpow_ht20[ 3];
	power[ATHN_POWER_HT20( 6)] = tpow_ht20[ 4];
	power[ATHN_POWER_HT20( 7)] = tpow_ht20[ 5];
	power[ATHN_POWER_HT20(12)] = tpow_ht20[ 6];
	power[ATHN_POWER_HT20(13)] = tpow_ht20[ 7];
	power[ATHN_POWER_HT20(14)] = tpow_ht20[ 8];
	power[ATHN_POWER_HT20(15)] = tpow_ht20[ 9];
	power[ATHN_POWER_HT20(20)] = tpow_ht20[10];
	power[ATHN_POWER_HT20(21)] = tpow_ht20[11];
	power[ATHN_POWER_HT20(22)] = tpow_ht20[12];
	power[ATHN_POWER_HT20(23)] = tpow_ht20[13];
	if (extc != NULL) {
		/* Next entry covers MCS0, MCS8 and MCS16. */
		power[ATHN_POWER_HT40( 0)] = tpow_ht40[ 0];
		/* Next entry covers MCS1-3, MCS9-11 and MCS17-19. */
		power[ATHN_POWER_HT40( 1)] = tpow_ht40[ 1];
		power[ATHN_POWER_HT40( 4)] = tpow_ht40[ 2];
		power[ATHN_POWER_HT40( 5)] = tpow_ht40[ 3];
		power[ATHN_POWER_HT40( 6)] = tpow_ht40[ 4];
		power[ATHN_POWER_HT40( 7)] = tpow_ht40[ 5];
		power[ATHN_POWER_HT40(12)] = tpow_ht40[ 6];
		power[ATHN_POWER_HT40(13)] = tpow_ht40[ 7];
		power[ATHN_POWER_HT40(14)] = tpow_ht40[ 8];
		power[ATHN_POWER_HT40(15)] = tpow_ht40[ 9];
		power[ATHN_POWER_HT40(20)] = tpow_ht40[10];
		power[ATHN_POWER_HT40(21)] = tpow_ht40[11];
		power[ATHN_POWER_HT40(22)] = tpow_ht40[12];
		power[ATHN_POWER_HT40(23)] = tpow_ht40[13];
	}

	/* Write transmit power values to hardware. */
	ar9003_write_txpower(sc, power);

	/* Apply transmit power correction. */
	ar9380_set_correction(sc, c);
}

void
ar9380_get_correction(struct athn_softc *sc, struct ieee80211_channel *c,
    int chain, int *corr, int *temp)
{
	const struct ar9380_eeprom *eep = sc->eep;
	const struct ar9380_cal_data_per_freq_op_loop *pierdata;
	const uint8_t *pierfreq;
	uint8_t fbin;
	int lo, hi, npiers;

	if (IEEE80211_IS_CHAN_2GHZ(c)) {
		pierfreq = eep->calFreqPier2G;
		pierdata = eep->calPierData2G[chain];
		npiers = AR9380_NUM_2G_CAL_PIERS;
	} else {
		pierfreq = eep->calFreqPier5G;
		pierdata = eep->calPierData5G[chain];
		npiers = AR9380_NUM_5G_CAL_PIERS;
	}
	/* Find channel in ROM pier table. */
	fbin = athn_chan2fbin(c);
	athn_get_pier_ival(fbin, pierfreq, npiers, &lo, &hi);

	*corr = athn_interpolate(fbin,
	    pierfreq[lo], pierdata[lo].refPower,
	    pierfreq[hi], pierdata[hi].refPower);
	*temp = athn_interpolate(fbin,
	    pierfreq[lo], pierdata[lo].tempMeas,
	    pierfreq[hi], pierdata[hi].tempMeas);
}

void
ar9380_set_correction(struct athn_softc *sc, struct ieee80211_channel *c)
{
	const struct ar9380_eeprom *eep = sc->eep;
	uint32_t reg;
	int8_t slope;
	int i, corr, temp, temp0;

	for (i = 0; i < AR9380_MAX_CHAINS; i++) {
		ar9380_get_correction(sc, c, i, &corr, &temp);
		if (i == 0)
			temp0 = temp;

		reg = AR_READ(sc, AR_PHY_TPC_11_B(i));
		reg = RW(reg, AR_PHY_TPC_11_OLPC_GAIN_DELTA, corr);
		AR_WRITE(sc, AR_PHY_TPC_11_B(i), reg);

		/* Enable open loop power control. */
		reg = AR_READ(sc, AR_PHY_TPC_6_B(i));
		reg = RW(reg, AR_PHY_TPC_6_ERROR_EST_MODE, 3);
		AR_WRITE(sc, AR_PHY_TPC_6_B(i), reg);
	}

	/* Enable temperature compensation. */
	if (IEEE80211_IS_CHAN_2GHZ(c))
		slope = eep->modalHeader2G.tempSlope;
	else
		slope = eep->modalHeader5G.tempSlope;

	reg = AR_READ(sc, AR_PHY_TPC_19);
	reg = RW(reg, AR_PHY_TPC_19_ALPHA_THERM, slope);
	AR_WRITE(sc, AR_PHY_TPC_19, reg);

	reg = AR_READ(sc, AR_PHY_TPC_18);
	reg = RW(reg, AR_PHY_TPC_18_THERM_CAL, temp0);
	AR_WRITE(sc, AR_PHY_TPC_18, reg);
}
