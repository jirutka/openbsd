/*	$NetBSD: tga.c,v 1.2 1995/08/03 01:17:28 cgd Exp $	*/

/*
 * Copyright (c) 1995 Carnegie-Mellon University.
 * All rights reserved.
 *
 * Author: Chris G. Demetriou
 * 
 * Permission to use, copy, modify and distribute this software and
 * its documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND 
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/device.h>
#include <sys/malloc.h>

#include <dev/pseudo/raster.h>
#include <dev/pseudo/rcons.h>
#include <dev/pseudo/ansicons.h>

#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcidevs.h>
#include <alpha/pci/tgareg.h>
#include <alpha/pci/tgavar.h>
#include <alpha/pci/bt485reg.h>

#include <machine/autoconf.h>
#include <machine/pte.h>

int	tgamatch __P((struct device *, void *, void *));
void	tgaattach __P((struct device *, struct device *, void *));

struct cfdriver tgacd = {
	NULL, "tga", tgamatch, tgaattach, DV_DULL, sizeof(struct tga_softc)
};

struct tga_devconfig tga_console_dc;

int	tga_identify __P((tga_reg_t *));
struct	tga_conf *tga_getconf __P((int));
void	tga_bell __P((void *));

struct ansicons_functions tga_acf = {
	tga_bell,
	rcons_cursor,		/* could use hardware cursor; who cares? */
	rcons_putstr,
	rcons_copycols,
	rcons_erasecols,
	rcons_copyrows,
	rcons_eraserows,
};

#define	TGAUNIT(dev)	minor(dev)

struct tga_ramdac_conf tga_ramdac_bt463 = {
	"Bt463",
#if 0
	XXX,
	YYY,
#endif
	/* XXX */
};

void	tga_bt485_set_cpos __P((struct tga_devconfig *, int, int));
void	tga_bt485_get_cpos __P((struct tga_devconfig *, int *, int *));

struct tga_ramdac_conf tga_ramdac_bt485 = {
	"Bt485",
	tga_bt485_set_cpos,
	tga_bt485_get_cpos,
	/* XXX */
};

#undef KB
#define KB		* 1024
#undef MB
#define	MB		* 1024 * 1024

struct tga_conf tga_configs[TGA_TYPE_UNKNOWN] = {
	/* TGA_TYPE_T8_01 */
	{
		"T8-01",
		&tga_ramdac_bt485,
		8,
		4 MB,
		2 KB,
		1,	{  2 MB,     0 },	{ 1 MB,    0 },
		0,	{     0,     0 },	{    0,    0 },
	},
	/* TGA_TYPE_T8_02 */
	{
		"T8-02",
		&tga_ramdac_bt485,
		8,
		4 MB,
		4 KB,
		1,	{  2 MB,     0 },	{ 2 MB,    0 },
		0,	{     0,     0 },	{    0,    0 },
	},
	/* TGA_TYPE_T8_22 */
	{
		"T8-22",
		&tga_ramdac_bt485,
		8,
		8 MB,
		4 KB,
		1,	{  4 MB,     0 },	{ 2 MB,    0 },
		1,	{  6 MB,     0 },	{ 2 MB,    0 },
	},
	/* TGA_TYPE_T8_44 */
	{
		"T8-44",
		&tga_ramdac_bt485,
		8,
		16 MB,
		4 KB,
		2,	{  8 MB, 12 MB },	{ 2 MB, 2 MB },
		2,	{ 10 MB, 14 MB },	{ 2 MB, 2 MB },
	},
	/* TGA_TYPE_T32_04 */
	{
		"T32-04",
		&tga_ramdac_bt463,
		32,
		16 MB,
		8 KB,
		1,	{  8 MB,     0 },	{ 4 MB,    0 },
		0,	{     0,     0 },	{    0,    0 },
	},
	/* TGA_TYPE_T32_08 */
	{
		"T32-08",
		&tga_ramdac_bt463,
		32,
		16 MB,
		16 KB,
		1,	{  8 MB,    0 },	{ 8 MB,    0 },
		0,	{     0,    0 },	{    0,    0 },
	},
	/* TGA_TYPE_T32_88 */
	{
		"T32-88",
		&tga_ramdac_bt463,
		32,
		32 MB,
		16 KB,
		1,	{ 16 MB,    0 },	{ 8 MB,    0 },
		1,	{ 24 MB,    0 },	{ 8 MB,    0 },
	},
};
#undef KB
#undef MB

int
tgamatch(parent, match, aux)
	struct device *parent;
	void *match, *aux;
{
	struct cfdata *cf = match;
	struct pci_attach_args *pa = aux;

	if (PCI_VENDOR(pa->pa_id) != PCI_VENDOR_DEC ||
	    PCI_PRODUCT(pa->pa_id) != PCI_PRODUCT_DEC_21030)
		return (0);

	return (1);
}

int
tga_identify(regs)
	tga_reg_t *regs;
{
	int type;
	int deep, addrmask, wide;

	deep = (regs[TGA_REG_GDER] & 0x1) != 0;		/* XXX */
	addrmask = ((regs[TGA_REG_GDER] >> 2) & 0x7);	/* XXX */
	wide = (regs[TGA_REG_GDER] & 0x200) == 0;	/* XXX */

	type = TGA_TYPE_UNKNOWN;

	if (!deep) {
		/* 8bpp frame buffer */

		if (addrmask == 0x0) {
			/* 4MB core map; T8-01 or T8-02 */

			if (!wide)
				type = TGA_TYPE_T8_01;
			else
				type = TGA_TYPE_T8_02;
		} else if (addrmask == 0x1) {
			/* 8MB core map; T8-22 */

			if (wide)			/* sanity */
				type = TGA_TYPE_T8_22;
		} else if (addrmask == 0x3) {
			/* 16MB core map; T8-44 */

			if (wide)			/* sanity */
				type = TGA_TYPE_T8_44;
		}
	} else {
		/* 32bpp frame buffer */

		if (addrmask == 0x3) {
			/* 16MB core map; T32-04 or T32-08 */

			if (!wide)
				type = TGA_TYPE_T32_04;
			else
				type = TGA_TYPE_T32_08;
		} else if (addrmask == 0x7) {
			/* 32MB core map; T32-88 */

			if (wide)			/* sanity */
				type = TGA_TYPE_T32_88;
		}
	}

	return (type);
}

struct tga_conf *
tga_getconf(type)
	int type;
{

	if (type >= 0 && type < TGA_TYPE_UNKNOWN)
		return &tga_configs[type];

	return (NULL);
}

void
tga_getdevconfig(tag, dc)
	pcitag_t tag;
	struct tga_devconfig *dc;
{
	struct tga_conf *tgac;
	struct tga_ramdac_conf *tgar;
	struct raster *rap;
	struct rcons *rcp;
	vm_offset_t pcipa;
	int i;

	dc->dc_pcitag = tag;

	dc->dc_vaddr = 0;
	if (pci_map_mem(tag, 0x10, &dc->dc_vaddr, &pcipa))
		return;

	/* PA filled in by pci_map_mem is the PCI-bus PA, i.e. not shifted */
	dc->dc_paddr = k0segtophys(dc->dc_vaddr);

	dc->dc_regs = (tga_reg_t *)(dc->dc_vaddr + TGA_MEM_CREGS);
	dc->dc_tga_type = tga_identify(dc->dc_regs);
	tgac = dc->dc_tgaconf = tga_getconf(dc->dc_tga_type);
	if (tgac == NULL)
		return;
	tgar = tgac->tgac_ramdac;

	switch (dc->dc_regs[TGA_REG_VHCR] & 0x1ff) {		/* XXX */
	case 0:
		dc->dc_wid = 8192;
		break;

	case 1:
		dc->dc_wid = 8196;
		break;

	default:
		dc->dc_wid = (dc->dc_regs[TGA_REG_VHCR] & 0x1ff) * 4; /* XXX */
		break;
	}

	dc->dc_rowbytes = dc->dc_wid * (dc->dc_tgaconf->tgac_phys_depth / 8);

	if ((dc->dc_regs[TGA_REG_VHCR] & 0x00000001) != 0 &&	/* XXX */
	    (dc->dc_regs[TGA_REG_VHCR] & 0x80000000) != 0)	/* XXX */
		dc->dc_wid -= 4;

	dc->dc_ht = (dc->dc_regs[TGA_REG_VVCR] & 0x7ff);	/* XXX */

	/* XXX this seems to be what DEC does */
	dc->dc_regs[TGA_REG_VVBR] = 1;
	dc->dc_videobase = dc->dc_vaddr + tgac->tgac_dbuf[0] +
	    1 * tgac->tgac_vvbr_units;

	/* disable the cursor */
	(*tgar->tgar_set_cpos)(dc, TGA_CURSOR_OFF, 0);

	/* init black and white color map entries to 'sane' values. */
#if 0
	(*tgar->tga_set_cmap)(dc, 0, 0, 0, 0);
	(*tgar->tga_set_cmap)(dc, 255, 0xff, 0xff, 0xff);
#endif

	/* clear the screen */
	for (i = 0; i < dc->dc_ht * dc->dc_rowbytes; i += sizeof(u_int32_t))
		*(u_int32_t *)(dc->dc_videobase + i) = 0;

	/* initialize the raster */
	rap = &dc->dc_raster;
	rap->width = dc->dc_wid;
	rap->height = dc->dc_ht;
	rap->depth = tgac->tgac_phys_depth;
	rap->linelongs = dc->dc_rowbytes / sizeof(u_int32_t);
	rap->pixels = (u_int32_t *)dc->dc_videobase;

	/* initialize the raster console blitter */
	rcp = &dc->dc_rcons;
	rcp->rc_sp = rap;
	rcp->rc_crow = rcp->rc_ccol = -1;
	rcp->rc_crowp = &rcp->rc_crow;
	rcp->rc_ccolp = &rcp->rc_ccol;
	rcons_init(rcp, 34, 80);
}

void
tgaattach(parent, self, aux)
	struct device *parent, *self;
	void *aux;
{
	struct pci_attach_args *pa = aux;
	struct tga_softc *sc = (struct tga_softc *)self;
	pci_revision_t rev;
	pcireg_t pci_intrdata;
	int console;

	console = (pa->pa_tag == tga_console_dc.dc_pcitag);
	if (console)
		sc->sc_dc = &tga_console_dc;
	else {
		sc->sc_dc = (struct tga_devconfig *)
		    malloc(sizeof(struct tga_devconfig), M_DEVBUF, M_WAITOK);
		tga_getdevconfig(pa->pa_tag, sc->sc_dc);
	}
	if (sc->sc_dc->dc_vaddr == NULL) {
		printf(": couldn't map memory space; punt!\n");
		return;
	}

	printf(": DC21030 ");
	rev = PCI_REVISION(pa->pa_class);
	switch (rev) {
	case 1: case 2: case 3:
		printf("step %c", 'A' + rev - 1);
		break;

	default:
		printf("unknown stepping (0x%x)", rev);
		break;
	}
	printf(", ");

	if (sc->sc_dc->dc_tgaconf == NULL) {
		printf("unknown board configuration\n");
		return;
	}
	printf("board type %s\n", sc->sc_dc->dc_tgaconf->tgac_name);
	printf("%s: %d x %d, %dbpp, %s RAMDAC\n", sc->sc_dev.dv_xname,
	    sc->sc_dc->dc_wid, sc->sc_dc->dc_ht,
	    sc->sc_dc->dc_tgaconf->tgac_phys_depth, 
	    sc->sc_dc->dc_tgaconf->tgac_ramdac->tgar_name);
#if 0
	/* not done here; printed by wscons attach. */
	if (console)
		printf("%s: console\n", sc->sc_dev.dv_xname);
#endif

#if 0
	pci_intrdata = pci_conf_read(sc->sc_pcitag, PCI_INTERRUPT_REG);
	if (PCI_INTERRUPT_PIN(pci_intrdata) != PCI_INTERRUPT_PIN_NONE) {
		sc->sc_intr = pci_map_int(sc->sc_pcitag, PCI_IPL_TTY,
		    tgaintr, sc);
		if (sc->sc_intr == NULL)
			printf("%s: WARNING: couldn't map interrupt\n",
			    sc->sc_dev.dv_xname);
	}
#endif

	if (!wscattach_output(self, console, &sc->sc_dc->dc_ansicons, &tga_acf,
	    &sc->sc_dc->dc_rcons, sc->sc_dc->dc_rcons.rc_maxrow,
	    sc->sc_dc->dc_rcons.rc_maxcol, 0, 0)) {
		panic("tgaattach: wscattach failed");
		/* NOTREACHED */
	}
}

int
tgaopen(dev, flag, mode, p)
	dev_t dev;
	int flag, mode;
	struct proc *p;
{
	struct tga_softc *sc;
	int unit = TGAUNIT(dev);

	if (unit >= tgacd.cd_ndevs)
		return ENXIO;
	sc = tgacd.cd_devs[unit];
	if ((sc = tgacd.cd_devs[unit]) == NULL || sc->sc_dc->dc_tgaconf == NULL)
		return ENXIO;

	return (0);
}

int
tgaclose(dev, flag, mode, p)
	dev_t dev;
	int flag, mode;
	struct proc *p;
{
	struct tga_softc *sc = tgacd.cd_devs[TGAUNIT(dev)];

	return (0);
}

int
tgaioctl(dev, cmd, data, flag, p)
	dev_t dev;
	u_long cmd;
	caddr_t data;
	int flag;
	struct proc *p;
{
	struct tga_softc *sc = tgacd.cd_devs[TGAUNIT(dev)];

	return (ENOTTY);
}

int
tgammap(dev, offset, nprot)
	dev_t dev;
	int offset;
	int nprot;
{
	struct tga_softc *sc = tgacd.cd_devs[TGAUNIT(dev)];

	if (offset > sc->sc_dc->dc_tgaconf->tgac_cspace_size)
		return -1;
	return alpha_btop(sc->sc_dc->dc_paddr + offset);
}

void
tga_bell(id)
	void *id;
{

	/* XXX */
	printf("tga_bell: not implemented\n");
}


/*
 * Bt485-specific functions.
 */

void
tga_bt485_wr_reg(tgaregs, btreg, val)
	volatile tga_reg_t *tgaregs;
	u_int btreg;
	u_int8_t val;
{

	if (btreg > BT485_REG_MAX)
		panic("tga_bt485_wr_reg: reg %d out of range\n", btreg);

	tgaregs[TGA_REG_EPDR] = (btreg << 9) | (0 << 8 ) | val; /* XXX */
	wbflush();
}

u_int8_t
tga_bt485_rd_reg(tgaregs, btreg)
	volatile tga_reg_t *tgaregs;
	u_int btreg;
{
	tga_reg_t rdval;

	if (btreg > BT485_REG_MAX)
		panic("tga_bt485_rd_reg: reg %d out of range\n", btreg);

	tgaregs[TGA_REG_EPSR] = (btreg << 1) | 0x1;		/* XXX */
	wbflush();

	rdval = tgaregs[TGA_REG_EPDR];
	return (rdval >> 16) & 0xff;				/* XXX */
}

void
tga_bt485_set_cpos(dc, x, y)
	struct tga_devconfig *dc;
	int x, y;
{

	if (x == TGA_CURSOR_OFF || y == TGA_CURSOR_OFF) {
		u_int8_t regval;

		regval = tga_bt485_rd_reg(dc->dc_regs, BT485_REG_COMMAND_2);
		regval &= ~0x03;				/* XXX */
		regval |= 0x00;					/* XXX */
		tga_bt485_wr_reg(dc->dc_regs, BT485_REG_COMMAND_2, regval);
	}

	/*
	 * RAMDAC cursors are 1-based, and position is bottom-right
	 * of displayed cursor!
	 */
	x += 64;
	y += 64;

	/* XXX CONSTANTS */
	tga_bt485_rd_reg(dc->dc_regs, BT485_REG_CURSOR_X_LOW,
	    x & 0xff);
	tga_bt485_rd_reg(dc->dc_regs, BT485_REG_CURSOR_X_HIGH,
	    (x >> 8) & 0x0f);
	tga_bt485_rd_reg(dc->dc_regs, BT485_REG_CURSOR_Y_LOW,
	    y & 0xff);
	tga_bt485_rd_reg(dc->dc_regs, BT485_REG_CURSOR_Y_HIGH,
	    (y >> 8) & 0x0f);
}

void
tga_bt485_get_cpos(dc, xp, yp)
	struct tga_devconfig *dc;
	int *xp, *yp;
{
	u_int8_t regval;

	regval = tga_bt485_rd_reg(dc->dc_regs, BT485_REG_COMMAND_2);
	if ((regval & 0x03) == 0x00) {			/* XXX */
		*xp = *yp = TGA_CURSOR_OFF;
		return;
	}

	regval = tga_bt485_rd_reg(dc->dc_regs, BT485_REG_CURSOR_X_LOW);
	*xp = regval;
	regval = tga_bt485_rd_reg(dc->dc_regs, BT485_REG_CURSOR_X_HIGH);
	*xp |= regval << 8;				/* XXX */

	regval = tga_bt485_rd_reg(dc->dc_regs, BT485_REG_CURSOR_Y_LOW);
	*yp = regval;
	regval = tga_bt485_rd_reg(dc->dc_regs, BT485_REG_CURSOR_Y_HIGH);
	*yp |= regval << 8;				/* XXX */

	/*
	 * RAMDAC cursors are 1-based, and position is bottom-right
	 * of displayed cursor!
	 */
	(*xp) -= 64;
	(*yp) -= 64;
}

void
tga_console(bus, device, function)
	int bus, device, function;
{
	struct tga_devconfig *dcp = &tga_console_dc;

	tga_getdevconfig(pci_make_tag(bus, device, function), dcp);

	/* sanity checks */
	if (dcp->dc_vaddr == NULL)
		panic("tga_console(%d, %d, %d): couldn't map memory space",
		    bus, device, function);
	if (dcp->dc_tgaconf == NULL)
		panic("tga_console(%d, %d, %d): unknown board configuration",
		    bus, device, function);

	wsc_console(&dcp->dc_ansicons, &tga_acf, &dcp->dc_rcons,
	    dcp->dc_rcons.rc_maxrow, dcp->dc_rcons.rc_maxcol, 0, 0);
}
