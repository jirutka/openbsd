/*	$OpenBSD: src/sys/dev/ic/anvar.h,v 1.12 2003/07/14 22:48:50 mickey Exp $	*/

/*
 * Copyright (c) 1997, 1998, 1999
 *	Bill Paul <wpaul@ctr.columbia.edu>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Bill Paul.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Bill Paul AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Bill Paul OR THE VOICES IN HIS HEAD
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD: src/sys/dev/an/if_aironet_ieee.h,v 1.1 2000/01/14 20:40:55 wpaul Exp $
 */

#ifndef _IF_AIRONET_IEEE_H
#define _IF_AIRONET_IEEE_H

#pragma pack(1)

/*
 * This header defines a simple command interface to the FreeBSD
 * Aironet driver (an) driver, which is used to set certain
 * device-specific parameters which can't be easily managed through
 * ifconfig(8). No, sysctl(2) is not the answer. I said a _simple_
 * interface, didn't I.
 */

#ifndef SIOCSAIRONET
#ifdef __FreeBSD__
#define SIOCSAIRONET	SIOCSIFGENERIC
#else   /* !__FreeBSD__ */
#define SIOCSAIRONET	SIOCSIFASYNCMAP
#endif
#endif

#ifndef SIOCGAIRONET
#ifdef __FreeBSD__
#define SIOCGAIRONET	SIOCGIFGENERIC
#else   /* !__FreeBSD__ */
#define SIOCGAIRONET	SIOCGIFASYNCMAP
#endif
#endif

/*
 * This is a make-predend RID value used only by the driver
 * to allow the user to set the speed.
 */
#define AN_RID_TX_SPEED		0x1234

/*
 * Technically I don't think there's a limit to a record
 * length. The largest record is the one that contains the CIS
 * data, which is 240 words long, so 256 should be a safe
 * value.
 */
#define AN_MAX_DATALEN	512

struct an_req {
	u_int16_t	an_len;
	u_int16_t	an_type;
	u_int16_t	an_val[AN_MAX_DATALEN];
};

/*
 * Private LTV records (interpreted only by the driver). This is
 * a minor kludge to allow reading the interface statistics from
 * the driver.
 */
#define AN_RID_IFACE_STATS	0x0100
#define AN_RID_MGMT_XMIT	0x0200
#define AN_RID_ZERO_CACHE	0x0300
#define AN_RID_READ_CACHE	0x0400

struct an_80211_hdr {
	u_int16_t		frame_ctl;
	u_int16_t		dur_id;
	u_int8_t		addr1[6];
	u_int8_t		addr2[6];
	u_int8_t		addr3[6];
	u_int16_t		seq_ctl;
	u_int8_t		addr4[6];
};

#define AN_FCTL_VERS		0x0002
#define AN_FCTL_FTYPE		0x000C
#define AN_FCTL_STYPE		0x00F0
#define AN_FCTL_TODS		0x0100
#define AN_FCTL_FROMDS		0x0200
#define AN_FCTL_MOREFRAGS	0x0400
#define AN_FCTL_RETRY		0x0800
#define AN_FCTL_PM		0x1000
#define AN_FCTL_MOREDATA	0x2000
#define AN_FCTL_WEP		0x4000
#define AN_FCTL_ORDER		0x8000

#define AN_FTYPE_MGMT		0x0000
#define AN_FTYPE_CTL		0x0004
#define AN_FTYPE_DATA		0x0008

#define AN_STYPE_MGMT_ASREQ	0x0000	/* association request */
#define AN_STYPE_MGMT_ASRESP	0x0010	/* association response */
#define AN_STYPE_MGMT_REASREQ	0x0020	/* reassociation request */
#define AN_STYPE_MGMT_REASRESP	0x0030	/* reassociation response */
#define AN_STYPE_MGMT_PROBEREQ	0x0040	/* probe request */
#define AN_STYPE_MGMT_PROBERESP	0x0050	/* probe response */
#define AN_STYPE_MGMT_BEACON	0x0080	/* beacon */
#define AN_STYPE_MGMT_ATIM	0x0090	/* announcement traffic ind msg */
#define AN_STYPE_MGMT_DISAS	0x00A0	/* disassociation */
#define AN_STYPE_MGMT_AUTH	0x00B0	/* authentication */
#define AN_STYPE_MGMT_DEAUTH	0x00C0	/* deauthentication */

struct an_mgmt_hdr {
	u_int16_t		frame_ctl;
	u_int16_t		duration;
	u_int8_t		dst_addr[6];
	u_int8_t		src_addr[6];
	u_int8_t		bssid[6];
	u_int16_t		seq_ctl;
};

/*
 * Aironet IEEE signal strength cache
 *
 * driver keeps cache of last
 * MAXANCACHE packets to arrive including signal strength info.
 * daemons may read this via ioctl
 *
 * Each entry in the wi_sigcache has a unique macsrc.
 */
#define MAXANCACHE      10

struct an_sigcache {
	char	macsrc[6];	/* unique MAC address for entry */
	int	ipsrc;		/* ip address associated with packet */
	int	signal;		/* signal strength of the packet */
	int	noise;		/* noise value */
	int	quality;	/* quality of the packet */
};

struct an_ltv_key {
	u_int16_t		an_len;
	u_int16_t		an_type;
	u_int16_t		kindex;
	u_int8_t		mac[6];
	u_int16_t		klen;
	u_int8_t		key[16];
};

/*
 * Grrr. The manual says the statistics record is 384 bytes in length,
 * but the card says the record is 404 bytes. There's some padding left
 * at the end of this structure to account for any discrepancies.
 */
struct an_ltv_stats {
	u_int16_t		an_fudge;
	u_int16_t		an_len;			/* 0x00 */
	u_int16_t		an_type;		/* 0xXX */
	u_int16_t		an_spacer;		/* 0x02 */
	u_int32_t		an_rx_overruns;		/* 0x04 */
	u_int32_t		an_rx_plcp_csum_errs;	/* 0x08 */
	u_int32_t		an_rx_plcp_format_errs;	/* 0x0C */
	u_int32_t		an_rx_plcp_len_errs;	/* 0x10 */
	u_int32_t		an_rx_mac_crc_errs;	/* 0x14 */
	u_int32_t		an_rx_mac_crc_ok;	/* 0x18 */
	u_int32_t		an_rx_wep_errs;		/* 0x1C */
	u_int32_t		an_rx_wep_ok;		/* 0x20 */
	u_int32_t		an_retry_long;		/* 0x24 */
	u_int32_t		an_retry_short;		/* 0x28 */
	u_int32_t		an_retry_max;		/* 0x2C */
	u_int32_t		an_no_ack;		/* 0x30 */
	u_int32_t		an_no_cts;		/* 0x34 */
	u_int32_t		an_rx_ack_ok;		/* 0x38 */
	u_int32_t		an_rx_cts_ok;		/* 0x3C */
	u_int32_t		an_tx_ack_ok;		/* 0x40 */
	u_int32_t		an_tx_rts_ok;		/* 0x44 */
	u_int32_t		an_tx_cts_ok;		/* 0x48 */
	u_int32_t		an_tx_lmac_mcasts;	/* 0x4C */
	u_int32_t		an_tx_lmac_bcasts;	/* 0x50 */
	u_int32_t		an_tx_lmac_ucast_frags;	/* 0x54 */
	u_int32_t		an_tx_lmac_ucasts;	/* 0x58 */
	u_int32_t		an_tx_beacons;		/* 0x5C */
	u_int32_t		an_rx_beacons;		/* 0x60 */
	u_int32_t		an_tx_single_cols;	/* 0x64 */
	u_int32_t		an_tx_multi_cols;	/* 0x68 */
	u_int32_t		an_tx_defers_no;	/* 0x6C */
	u_int32_t		an_tx_defers_prot;	/* 0x70 */
	u_int32_t		an_tx_defers_energy;	/* 0x74 */
	u_int32_t		an_rx_dups;		/* 0x78 */
	u_int32_t		an_rx_partial;		/* 0x7C */
	u_int32_t		an_tx_too_old;		/* 0x80 */
	u_int32_t		an_rx_too_old;		/* 0x84 */
	u_int32_t		an_lostsync_max_retries;/* 0x88 */
	u_int32_t		an_lostsync_missed_beacons;/* 0x8C */
	u_int32_t		an_lostsync_arl_exceeded;/*0x90 */
	u_int32_t		an_lostsync_deauthed;	/* 0x94 */
	u_int32_t		an_lostsync_disassociated;/*0x98 */
	u_int32_t		an_lostsync_tsf_timing;	/* 0x9C */
	u_int32_t		an_tx_host_mcasts;	/* 0xA0 */
	u_int32_t		an_tx_host_bcasts;	/* 0xA4 */
	u_int32_t		an_tx_host_ucasts;	/* 0xA8 */
	u_int32_t		an_tx_host_failed;	/* 0xAC */
	u_int32_t		an_rx_host_mcasts;	/* 0xB0 */
	u_int32_t		an_rx_host_bcasts;	/* 0xB4 */
	u_int32_t		an_rx_host_ucasts;	/* 0xB8 */
	u_int32_t		an_rx_host_discarded;	/* 0xBC */
	u_int32_t		an_tx_hmac_mcasts;	/* 0xC0 */
	u_int32_t		an_tx_hmac_bcasts;	/* 0xC4 */
	u_int32_t		an_tx_hmac_ucasts;	/* 0xC8 */
	u_int32_t		an_tx_hmac_failed;	/* 0xCC */
	u_int32_t		an_rx_hmac_mcasts;	/* 0xD0 */
	u_int32_t		an_rx_hmac_bcasts;	/* 0xD4 */
	u_int32_t		an_rx_hmac_ucasts;	/* 0xD8 */
	u_int32_t		an_rx_hmac_discarded;	/* 0xDC */
	u_int32_t		an_tx_hmac_accepted;	/* 0xE0 */
	u_int32_t		an_ssid_mismatches;	/* 0xE4 */
	u_int32_t		an_ap_mismatches;	/* 0xE8 */
	u_int32_t		an_rates_mismatches;	/* 0xEC */
	u_int32_t		an_auth_rejects;	/* 0xF0 */
	u_int32_t		an_auth_timeouts;	/* 0xF4 */
	u_int32_t		an_assoc_rejects;	/* 0xF8 */
	u_int32_t		an_assoc_timeouts;	/* 0xFC */
	u_int32_t		an_reason_outside_table;/* 0x100 */
	u_int32_t		an_reason1;		/* 0x104 */
	u_int32_t		an_reason2;		/* 0x108 */
	u_int32_t		an_reason3;		/* 0x10C */
	u_int32_t		an_reason4;		/* 0x110 */
	u_int32_t		an_reason5;		/* 0x114 */
	u_int32_t		an_reason6;		/* 0x118 */
	u_int32_t		an_reason7;		/* 0x11C */
	u_int32_t		an_reason8;		/* 0x120 */
	u_int32_t		an_reason9;		/* 0x124 */
	u_int32_t		an_reason10;		/* 0x128 */
	u_int32_t		an_reason11;		/* 0x12C */
	u_int32_t		an_reason12;		/* 0x130 */
	u_int32_t		an_reason13;		/* 0x134 */
	u_int32_t		an_reason14;		/* 0x138 */
	u_int32_t		an_reason15;		/* 0x13C */
	u_int32_t		an_reason16;		/* 0x140 */
	u_int32_t		an_reason17;		/* 0x144 */
	u_int32_t		an_reason18;		/* 0x148 */
	u_int32_t		an_reason19;		/* 0x14C */
	u_int32_t		an_rx_mgmt_pkts;	/* 0x150 */
	u_int32_t		an_tx_mgmt_pkts;	/* 0x154 */
	u_int32_t		an_rx_refresh_pkts;	/* 0x158 */
	u_int32_t		an_tx_refresh_pkts;	/* 0x15C */
	u_int32_t		an_rx_poll_pkts;	/* 0x160 */
	u_int32_t		an_tx_poll_pkts;	/* 0x164 */
	u_int32_t		an_host_retries;	/* 0x168 */
	u_int32_t		an_lostsync_hostreq;	/* 0x16C */
	u_int32_t		an_host_tx_bytes;	/* 0x170 */
	u_int32_t		an_host_rx_bytes;	/* 0x174 */
	u_int32_t		an_uptime_usecs;	/* 0x178 */
	u_int32_t		an_uptime_secs;		/* 0x17C */
	u_int32_t		an_lostsync_better_ap;	/* 0x180 */
	u_int32_t		an_rsvd[10];
};

struct an_ltv_genconfig {
	/* General configuration. */
	u_int16_t		an_len;			/* 0x00 */
	u_int16_t		an_type;		/* XXXX */
	u_int16_t		an_opmode;		/* 0x02 */
	u_int16_t		an_rxmode;		/* 0x04 */
	u_int16_t		an_fragthresh;		/* 0x06 */
	u_int16_t		an_rtsthresh;		/* 0x08 */
	u_int8_t		an_macaddr[6];		/* 0x0A */
	u_int8_t		an_rates[8];		/* 0x10 */
	u_int16_t		an_shortretry_limit;	/* 0x18 */
	u_int16_t		an_longretry_limit;	/* 0x1A */
	u_int16_t		an_tx_msdu_lifetime;	/* 0x1C */
	u_int16_t		an_rx_msdu_lifetime;	/* 0x1E */
	u_int16_t		an_stationary;		/* 0x20 */
	u_int16_t		an_ordering;		/* 0x22 */
	u_int16_t		an_devtype;		/* 0x24 */
	u_int16_t		an_rsvd0[5];		/* 0x26 */
	/* Scanning associating. */
	u_int16_t		an_scanmode;		/* 0x30 */
	u_int16_t		an_probedelay;		/* 0x32 */
	u_int16_t		an_probe_energy_timeout;/* 0x34 */
	u_int16_t		an_probe_response_timeout;/*0x36 */
	u_int16_t		an_beacon_listen_timeout;/*0x38 */
	u_int16_t		an_ibss_join_net_timeout;/*0x3A */
	u_int16_t		an_auth_timeout;	/* 0x3C */
	u_int16_t		an_authtype;		/* 0x3E */
	u_int16_t		an_assoc_timeout;	/* 0x40 */
	u_int16_t		an_specified_ap_timeout;/* 0x42 */
	u_int16_t		an_offline_scan_interval;/*0x44 */
	u_int16_t		an_offline_scan_duration;/*0x46 */
	u_int16_t		an_link_loss_delay;	/* 0x48 */
	u_int16_t		an_max_beacon_lost_time;/* 0x4A */
	u_int16_t		an_refresh_interval;	/* 0x4C */
	u_int16_t		an_rsvd1;		/* 0x4E */
	/* Power save operation */
	u_int16_t		an_psave_mode;		/* 0x50 */
	u_int16_t		an_sleep_for_dtims;	/* 0x52 */
	u_int16_t		an_listen_interval;	/* 0x54 */
	u_int16_t		an_fast_listen_interval;/* 0x56 */
	u_int16_t		an_listen_decay;	/* 0x58 */
	u_int16_t		an_fast_listen_decay;	/* 0x5A */
	u_int16_t		an_rsvd2[2];		/* 0x5C */
	/* Ad-hoc (or AP) operation. */
	u_int16_t		an_beacon_period;	/* 0x60 */
	u_int16_t		an_atim_duration;	/* 0x62 */
	u_int16_t		an_rsvd3;		/* 0x64 */
	u_int16_t		an_ds_channel;		/* 0x66 */
	u_int16_t		an_rsvd4;		/* 0x68 */
	u_int16_t		an_dtim_period;		/* 0x6A */
	u_int16_t		an_rsvd5[2];		/* 0x6C */
	/* Radio operation. */
	u_int16_t		an_radiotype;		/* 0x70 */
	u_int16_t		an_diversity;		/* 0x72 */
	u_int16_t		an_tx_power;		/* 0x74 */
	u_int16_t		an_rss_thresh;		/* 0x76 */
	u_int16_t		an_modulation_type;	/* 0x78 */
	u_int16_t		an_short_preamble;	/* 0x7A */
	u_int16_t		an_home_product;	/* 0x7C */
	u_int16_t		an_rsvd6;		/* 0x7E */
	/* Aironet extensions. */
	u_int8_t		an_nodename[16];	/* 0x80 */
	u_int16_t		an_arl_thresh;		/* 0x90 */
	u_int16_t		an_arl_decay;		/* 0x92 */
	u_int16_t		an_arl_delay;		/* 0x94 */
	u_int8_t		an_rsvd7;		/* 0x96 */
	u_int8_t		an_rsvd8;		/* 0x97 */
	u_int8_t		an_magic_packet_action;	/* 0x98 */
	u_int8_t		an_magic_packet_ctl;	/* 0x99 */
	u_int16_t		an_rsvd9;
};

#define AN_OPMODE_IBSS_ADHOC			0x0000
#define AN_OPMODE_INFRASTRUCTURE_STATION	0x0001
#define AN_OPMODE_AP				0x0002
#define AN_OPMODE_AP_REPEATER			0x0003
#define AN_OPMODE_UNMODIFIED_PAYLOAD		0x0100
#define AN_OPMODE_AIRONET_EXTENSIONS		0x0200
#define AN_OPMODE_AP_EXTENSIONS			0x0400

#define AN_RXMODE_BC_MC_ADDR			0x0000
#define AN_RXMODE_BC_ADDR			0x0001
#define AN_RXMODE_ADDR				0x0002
#define AN_RXMODE_80211_MONITOR_CURBSS		0x0003
#define AN_RXMODE_80211_MONITOR_ANYBSS		0x0004
#define AN_RXMODE_LAN_MONITOR_CURBSS		0x0005
#define AN_RXMODE_NO_8023_HEADER		0x0100

#define AN_RATE_1MBPS				0x0002
#define AN_RATE_2MBPS				0x0004
#define AN_RATE_5_5MBPS				0x000B
#define AN_RATE_11MBPS				0x0016

#define AN_DEVTYPE_PC4500			0x0065
#define AN_DEVTYPE_PC4800			0x006D

#define AN_SCANMODE_ACTIVE			0x0000
#define AN_SCANMODE_PASSIVE			0x0001
#define AN_SCANMODE_AIRONET_ACTIVE		0x0002

#define AN_AUTHTYPE_NONE			0x0000
#define AN_AUTHTYPE_OPEN			0x0001
#define AN_AUTHTYPE_SHAREDKEY			0x0002
#define AN_AUTHTYPE_MASK			0x00FF
#define AN_AUTHTYPE_PRIVACY_IN_USE		0x0100
#define AN_AUTHTYPE_ALLOW_UNENCRYPTED		0x0200

#define AN_PSAVE_NONE				0x0000
#define AN_PSAVE_CAM				0x0001
#define AN_PSAVE_PSP				0x0002
#define AN_PSAVE_PSP_CAM			0x0003

#define AN_RADIOTYPE_80211_FH			0x0001
#define AN_RADIOTYPE_80211_DS			0x0002
#define AN_RADIOTYPE_LM2000_DS			0x0004

#define AN_DIVERSITY_FACTORY_DEFAULT		0x0000
#define AN_DIVERSITY_ANTENNA_1_ONLY		0x0001
#define AN_DIVERSITY_ANTENNA_2_ONLY		0x0002
#define AN_DIVERSITY_ANTENNA_1_AND_2		0x0003

#define AN_TXPOWER_FACTORY_DEFAULT		0x0000
#define AN_TXPOWER_50MW				50
#define AN_TXPOWER_100MW			100
#define AN_TXPOWER_250MW			250

/*
 * Valid SSID list. You can specify up to three SSIDs denoting
 * the service sets that you want to join. The first SSID always
 * defaults to "tsunami" which is a handy way to detect the
 * card.
 */
struct an_ltv_ssidlist {
	u_int16_t		an_len;
	u_int16_t		an_type;
	u_int16_t		an_ssid1_len;
	char			an_ssid1[32];
	u_int16_t		an_ssid2_len;
	char			an_ssid2[32];
	u_int16_t		an_ssid3_len;
	char			an_ssid3[32];
};

/*
 * Valid AP list.
 */
struct an_ltv_aplist {
	u_int16_t		an_len;
	u_int16_t		an_type;
	u_int8_t		an_ap1[8];
	u_int8_t		an_ap2[8];
	u_int8_t		an_ap3[8];
	u_int8_t		an_ap4[8];
};

/*
 * Driver name.
 */
struct an_ltv_drvname {
	u_int16_t		an_len;
	u_int16_t		an_type;
	u_int8_t		an_drvname[16];
};

/*
 * Frame encapsulation.
 */
struct an_rid_encap {
	u_int16_t		an_len;
	u_int16_t		an_type;
	u_int16_t		an_ethertype_default;
	u_int16_t		an_action_default;
	u_int16_t		an_ethertype0;
	u_int16_t		an_action0;
	u_int16_t		an_ethertype1;
	u_int16_t		an_action1;
	u_int16_t		an_ethertype2;
	u_int16_t		an_action2;
	u_int16_t		an_ethertype3;
	u_int16_t		an_action3;
	u_int16_t		an_ethertype4;
	u_int16_t		an_action4;
	u_int16_t		an_ethertype5;
	u_int16_t		an_action5;
	u_int16_t		an_ethertype6;
	u_int16_t		an_action6;
};

#define AN_ENCAP_ACTION_RX	0x0001
#define AN_ENCAP_ACTION_TX	0x0002

#define AN_RXENCAP_NONE		0x0000
#define AN_RXENCAP_RFC1024	0x0001

#define AN_TXENCAP_RFC1024	0x0000
#define AN_TXENCAP_80211	0x0002

/*
 * Card capabilities (read only).
 */
struct an_ltv_caps {
	u_int16_t		an_len;			/* 0x00 */
	u_int16_t		an_type;		/* XXXX */
	u_int8_t		an_oui[3];		/* 0x02 */
	u_int8_t		an_rsvd0;		/* 0x05 */
	u_int16_t		an_prodnum;		/* 0x06 */
	u_int8_t		an_manufname[32];	/* 0x08 */
	u_int8_t		an_prodname[16];	/* 0x28 */
	u_int8_t		an_prodvers[8];		/* 0x38 */
	u_int8_t		an_oemaddr[6];		/* 0x40 */
	u_int8_t		an_aironetaddr[6];	/* 0x46 */
	u_int16_t		an_radiotype;		/* 0x4C */
	u_int16_t		an_regdomain;		/* 0x4E */
	u_int8_t		an_callid[6];		/* 0x50 */
	u_int8_t		an_rates[8];		/* 0x56 */
	u_int8_t		an_rx_diversity;	/* 0x5E */
	u_int8_t		an_tx_diversity;	/* 0x5F */
	u_int16_t		an_tx_powerlevels[8];	/* 0x60 */
	u_int16_t		an_hwrev;		/* 0x70 */
	u_int16_t		an_hwcaps;		/* 0x72 */
	u_int16_t		an_temprange;		/* 0x74 */
	u_int16_t		an_fwrev;		/* 0x76 */
	u_int16_t		an_fwsubrev;		/* 0x78 */
	u_int16_t		an_ifacerev;		/* 0x7A */
	u_int16_t		an_softcaps;		/* 0x7C */
	u_int16_t		an_bootblockrev;	/* 0x7E */
	u_int16_t		an_req_hw_support;	/* 0x80 */
	u_int16_t		an_rsvd1;		/* 0x82 */
};

/*
 * Access point (read only)
 */
struct an_ltv_apinfo {
	u_int16_t		an_len;
	u_int16_t		an_type;
	u_int16_t		an_tim_addr;
	u_int16_t		an_airo_addr;
};

/*
 * Radio info (read only).
 */
struct an_ltv_radioinfo {
	u_int16_t		an_len;
	u_int16_t		an_type;
	/* ??? */
};

/*
 * Status (read only). Note: the manual claims this RID is 108 bytes
 * long (0x6A is the last datum, which is 2 bytes long) however when
 * this RID is read from the NIC, it returns a length of 110. To be
 * on the safe side, this structure is padded with an extra 16-bit
 * word. (There is a misprint in the manual which says the macaddr
 * field is 8 bytes long.)
 *
 * Also, the channel_set and current_channel fields appear to be
 * reversed. Either that, or the hop_period field is unused.
 */
struct an_ltv_status {
	u_int16_t		an_len;			/* 0x00 */
	u_int16_t		an_type;		/* 0xXX */
	u_int8_t		an_macaddr[6];		/* 0x02 */
	u_int16_t		an_opmode;		/* 0x08 */
	u_int16_t		an_errcode;		/* 0x0A */
	u_int16_t		an_cur_signal_strength;	/* 0x0C */
	u_int16_t		an_ssidlen;		/* 0x0E */
	u_int8_t		an_ssid[32];		/* 0x10 */
	u_int8_t		an_ap_name[16];		/* 0x30 */
	u_int8_t		an_cur_bssid[6];	/* 0x40 */
	u_int8_t		an_prev_bssid1[6];	/* 0x46 */
	u_int8_t		an_prev_bssid2[6];	/* 0x4C */
	u_int8_t		an_prev_bssid3[6];	/* 0x52 */
	u_int16_t		an_beacon_period;	/* 0x58 */
	u_int16_t		an_dtim_period;		/* 0x5A */
	u_int16_t		an_atim_duration;	/* 0x5C */
	u_int16_t		an_hop_period;		/* 0x5E */
	u_int16_t		an_cur_channel;		/* 0x62 */
	u_int16_t		an_channel_set;		/* 0x60 */
	u_int16_t		an_hops_to_backbone;	/* 0x64 */
	u_int16_t		an_ap_total_load;	/* 0x66 */
	u_int16_t		an_our_generated_load;	/* 0x68 */
	u_int16_t		an_accumulated_arl;	/* 0x6A */
	u_int16_t		an_cur_signal_quality;	/* 0x6C */
	u_int16_t		an_current_tx_rate;	/* 0x6E */
	u_int16_t		an_ap_device;		/* 0x70 */
	u_int16_t		an_normalized_rssi;	/* 0x72 */
	u_int16_t		an_short_pre_in_use;	/* 0x74 */
	u_int8_t		an_ap_ip_addr[4];	/* 0x76 */
	u_int16_t		an_max_noise_prev_sec;	/* 0x7A */
	u_int16_t		an_avg_noise_prev_min;	/* 0x7C */
	u_int16_t		an_max_noise_prev_min;	/* 0x7E */
	u_int16_t		an_spare[5];		/* 0x80 */
};

#define AN_STATUS_OPMODE_CONFIGURED		0x0001
#define AN_STATUS_OPMODE_MAC_ENABLED		0x0002
#define AN_STATUS_OPMODE_RX_ENABLED		0x0004
#define AN_STATUS_OPMODE_IN_SYNC		0x0010
#define AN_STATUS_OPMODE_ASSOCIATED		0x0020
#define AN_STATUS_OPMODE_ERROR			0x8000

/*
 * WEP config
 */
struct an_ltv_wepkey {
	u_int16_t		an_len;			/* 0x00 */
	u_int16_t		an_type;		/* 0xXX */
	u_int16_t		an_key_index;		/* 0x02 */
	u_int8_t		an_mac_addr[6];		/* 0x04 */
	u_int16_t		an_key_len;		/* 0x0A */
	u_int8_t		an_key[13];		/* 0x0C */
};

/*
 * These are all the LTV record types that we can read or write
 * from the Aironet. Not all of them are temendously useful, but I
 * list as many as I know about here for completeness.
 */

#pragma pack()

/*
 * Configuration (read/write)
 */
#define AN_RID_GENCONFIG	0xFF10	/* General configuration info */
#define AN_RID_SSIDLIST		0xFF11	/* Valid SSID list */
#define AN_RID_APLIST		0xFF12	/* Valid AP list */
#define AN_RID_DRVNAME		0xFF13	/* ID name of this node for diag */
#define AN_RID_ENCAP		0xFF14	/* Payload encapsulation type */
#define AN_RID_WEP_VOLATILE	0xFF15	/* Temporary WEP key configuration */
#define AN_RID_WEP_PERMANENT	0xFF16	/* Permanent WEP key configuration */
#define AN_RID_ACTUALCFG	0xFF20	/* Current configuration settings */

/*
 * Reporting (read only)
 */
#define AN_RID_CAPABILITIES	0xFF00	/* PC 4500/4800 capabilities */
#define AN_RID_APINFO		0xFF01	/* Access point info */
#define AN_RID_RADIOINFO	0xFF02	/* Radio info */
#define AN_RID_STATUS		0xFF50	/* Current status info */

/*
 * Statistics
 */
#define AN_RID_16BITS_CUM	0xFF60	/* Cumulative 16-bit stats counters */
#define AN_RID_16BITS_DELTA	0xFF61	/* 16-bit stats (since last clear) */
#define AN_RID_16BITS_DELTACLR	0xFF62	/* 16-bit stats, clear on read */
#define AN_RID_32BITS_CUM	0xFF68	/* Cumulative 32-bit stats counters */
#define AN_RID_32BITS_DELTA	0xFF69	/* 32-bit stats (since last clear) */
#define AN_RID_32BITS_DELTACLR	0xFF6A	/* 32-bit stats, clear on read */

#ifdef _KERNEL
#define AN_TX_RING_CNT	4
struct an_tx_ring_data {
	u_int16_t	an_tx_fids[AN_TX_RING_CNT];
	u_int16_t	an_tx_ring[AN_TX_RING_CNT];
	int		an_tx_prod;
	int		an_tx_cons;
};

struct an_softc	{
	struct device	sc_dev;
	struct arpcom	sc_arpcom;
	struct ifmedia	an_ifmedia;
	void		*sc_ih;
	struct timeout	an_stat_ch;

	bus_space_tag_t		an_btag;
	bus_space_handle_t	an_bhandle;

	struct an_ltv_genconfig	an_config;
	struct an_ltv_caps	an_caps;
	struct an_ltv_ssidlist	an_ssidlist;
	struct an_ltv_aplist	an_aplist;
	int			an_tx_rate;
	int			an_rxmode;
	int			an_gone;
	int			an_if_flags;
	u_int8_t		an_txbuf[1536];
	struct an_tx_ring_data	an_rdata;
	struct an_ltv_stats	an_stats;
	struct an_ltv_status	an_status;
	u_int8_t		an_associated;

	/* ANCACHE stuff */
	int			an_cache_iponly;
	int			an_cache_mcastonly;
	int			an_sigitems;
	struct an_sigcache	an_sigcache[MAXANCACHE];
	int			an_nextitem;
};

void	an_release_resources(struct device *);
int	an_alloc_port(struct device *, int, int);
int	an_alloc_memory(struct device *, int, int);
int	an_alloc_irq(struct device *, int, int);
int	an_probe(struct device *);
void	an_shutdown(void *);
int	an_attach(struct an_softc *);
void	an_init(struct an_softc *);
void	an_stop(struct an_softc *);
int	an_intr(void *);
#endif

#endif
