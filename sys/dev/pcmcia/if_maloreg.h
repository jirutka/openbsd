/*	$OpenBSD: src/sys/dev/pcmcia/if_maloreg.h,v 1.4 2007/05/28 13:51:09 mglocker Exp $ */

/*
 * Copyright (c) 2007 Marcus Glocker <mglocker@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
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

/* I/O registers */
#define MALO_REG_HOST_STATUS		0x00
#define MALO_REG_CARD_INTR_CAUSE	0x02
#define MALO_REG_HOST_INTR_MASK		0x04
#define MALO_REG_CMD_READ		0x12
#define MALO_REG_CMD_WRITE_LEN		0x18
#define MALO_REG_CMD_WRITE		0x1a
#define MALO_REG_CARD_STATUS		0x20
#define MALO_REG_HOST_INTR_CAUSE	0x22
#define MALO_REG_RBAL			0x28
#define MALO_REG_CMD_READ_LEN		0x30
#define MALO_REG_SCRATCH		0x3f
#define MALO_REG_CARD_INTR_MASK		0x44

/* I/O register values */
#define MALO_VAL_SCRATCH_READY		0x00
#define MALO_VAL_SCRATCH_FW_LOADED	0x5a
#define MALO_VAL_HOST_INTR_MASK_ON	0x001f
#define MALO_VAL_DNLD_OVER		(1 << 2)

/* interrupt reasons */
#define MALO_VAL_HOST_INTR_RX		(1 << 0)
#define MALO_VAL_HOST_INTR_CMD		(1 << 3)

/* FW commands */
#define MALO_CMD_RESP			0x8000
#define MALO_CMD_HWSPEC			0x0003
#define MALO_CMD_RESET			0x0005
#define MALO_CMD_STATUS			0x000a
#define MALO_CMD_RADIO			0x001c
#define MALO_CMD_CHANNEL		0x001d
#define MALO_CMD_TXPOWER		0x001e
#define MALO_CMD_ANTENNA		0x0020
#define MALO_CMD_MACCTRL		0x0028
#define MALO_CMD_MACADDR		0x004d

/* FW command values */
#define MALO_CMD_RADIO_OFF		0x0000
#define MALO_CMD_RADIO_ON		0x0001
#define MALO_CMD_RADIO_LONG_P		0x0000
#define MALO_CMD_RADIO_SHORT_P		0x0002
#define MALO_CMD_RADIO_AUTO_P		0x0004
#define MALO_CMD_MACCTRL_RX_ON		0x0001
#define MALO_CMD_MACCTRL_TX_ON		0x0002
