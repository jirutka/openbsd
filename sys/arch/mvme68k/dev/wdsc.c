/*	$OpenBSD: src/sys/arch/mvme68k/dev/wdsc.c,v 1.3 1996/05/29 17:54:13 chuck Exp $ */

/*
 * Copyright (c) 1996 Steve Woodford
 * Copyright (c) 1982, 1990 The Regents of the University of California.
 * All rights reserved.
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
 *  This product includes software developed by the University of
 *  California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *  @(#)wdsc.c
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/device.h>
#include <scsi/scsi_all.h>
#include <scsi/scsiconf.h>
#include <mvme68k/dev/dmavar.h>
#include <mvme68k/dev/sbicreg.h>
#include <mvme68k/dev/sbicvar.h>
#include <mvme68k/dev/wdscreg.h>
#include <machine/autoconf.h>
#include <mvme68k/dev/pccreg.h>

int     wdscprint       __P((void *auxp, char *));
void    wdscattach      __P((struct device *, struct device *, void *));
int     wdscmatch       __P((struct device *, struct cfdata *, void *));

void    wdsc_enintr     __P((struct sbic_softc *));
int     wdsc_dmago      __P((struct sbic_softc *, char *, int, int));
int     wdsc_dmanext    __P((struct sbic_softc *));
void    wdsc_dmastop    __P((struct sbic_softc *));
int     wdsc_dmaintr    __P((struct sbic_softc *));
int     wdsc_scsiintr   __P((struct sbic_softc *));

struct scsi_adapter wdsc_scsiswitch = {
    sbic_scsicmd,
    sbic_minphys,
    0,          /* no lun support */
    0,          /* no lun support */
};

struct scsi_device wdsc_scsidev = {
    NULL,       /* use default error handler */
    NULL,       /* do not have a start functio */
    NULL,       /* have no async handler */
    NULL,       /* Use default done routine */
};

struct cfattach wdsc_ca = {
	sizeof(struct sbic_softc), (cfmatch_t)wdscmatch, wdscattach
};

struct cfdriver wdsc_cd = {
    NULL, "wdsc", DV_DULL, NULL, 0 
};

/*
 * Define 'scsi_nosync = 0x00' to enable sync SCSI mode.
 * This is untested as yet, use at your own risk...
 */
u_long      scsi_nosync  = 0xff;
int         shift_nosync = 0;

/*
 * Match for SCSI devices on the onboard WD33C93 chip
 */
int
wdscmatch(pdp, cdp, auxp)
    struct device *pdp;
    struct cfdata *cdp;
    void *auxp;
{
    /*
     * Match everything
     */
    return(1);
}


/*
 * Attach the wdsc driver
 */
void
wdscattach(pdp, dp, auxp)
    struct device *pdp, *dp;
    void *auxp;
{
    struct sbic_softc   *sc = (struct sbic_softc *)dp;
    struct confargs *ca = auxp;
    struct pccreg *pcc = (struct pccreg *)ca->ca_master;
    int                 ipl;
    int tmp;

    sc->sc_enintr  = wdsc_enintr;
    sc->sc_dmago   = wdsc_dmago;
    sc->sc_dmanext = wdsc_dmanext;
    sc->sc_dmastop = wdsc_dmastop;
    sc->sc_dmacmd  = 0;

    sc->sc_link.adapter_softc  = sc;
    sc->sc_link.adapter_target = 7;
    sc->sc_link.adapter        = &wdsc_scsiswitch;
    sc->sc_link.device         = &wdsc_scsidev;
    sc->sc_link.openings       = 2;

    printf(": target %d\n", sc->sc_link.adapter_target);

    sc->sc_cregs = (void *)pcc;
    sc->sc_sbicp = (sbic_regmap_p)ca->ca_vaddr;

    /*
     * Eveything is a valid dma address.
     * 
     */
    sc->sc_dmamask = 0;

    /*
     * The onboard WD33C93 of the '147 is usually clocked at 10MHz...
     * (We use 10 times this for accuracy in later calculations)
     */
    sc->sc_clkfreq = 100;

    /*
     * Initialise the hardware
     */
    sbicinit(sc);

    sc->sc_ipl = ca->ca_ipl;

    pcc->pcc_sbicirq = ca->ca_ipl | PCC_IRQ_INT;
    pcc->pcc_dmairq = ca->ca_ipl | PCC_IRQ_INT;
    pcc->pcc_dmacsr  = 0;

    /*
     * Fix up the interrupts
     */
    sc->sc_dmaih.ih_fn = wdsc_dmaintr;
    sc->sc_dmaih.ih_arg = sc;
    sc->sc_dmaih.ih_ipl = ca->ca_ipl;
    pccintr_establish(PCCV_DMA, &sc->sc_dmaih);

    sc->sc_sbicih.ih_fn = wdsc_scsiintr;
    sc->sc_sbicih.ih_arg = sc;
    sc->sc_sbicih.ih_ipl = ca->ca_ipl;
    pccintr_establish(PCCV_SBIC, &sc->sc_sbicih);

    pcc->pcc_sbicirq = ca->ca_ipl | PCC_IRQ_IEN | PCC_IRQ_INT;

    /*
     * Attach all scsi units on us, watching for boot device
     * (see dk_establish).
     */
    tmp = bootpart;
    if (ca->ca_paddr != bootaddr) 
	bootpart = -1;		/* invalid flag to dk_establish */
    config_found(dp, &sc->sc_link, wdscprint);
    bootpart = tmp;		/* restore old value */
}

/*
 * print diag if pnp is NULL else just extra
 */
int
wdscprint(auxp, pnp)
    void *auxp;
    char *pnp;
{
    if (pnp == NULL)
        return(UNCONF);
    return(QUIET);
}


/*
 * Enable DMA interrupts
 */
void
wdsc_enintr(dev)
    struct sbic_softc *dev;
{
    struct pccreg *pcc = dev->sc_cregs;

    dev->sc_flags |= SBICF_INTR;

    pcc->pcc_dmairq = dev->sc_ipl | PCC_IRQ_IEN | PCC_IRQ_INT;
}

/*
 * Prime the hardware for a DMA transfer
 */
int
wdsc_dmago(dev, addr, count, flags)
    struct sbic_softc *dev;
    char *addr;
    int count, flags;
{
    volatile struct pccreg *pc = dev->sc_cregs;

    /*
     * Set up the command word based on flags
     */
    if ( (flags & DMAGO_READ) == 0 )
        dev->sc_dmacmd = DMAC_CSR_ENABLE | DMAC_CSR_WRITE;
    else
        dev->sc_dmacmd = DMAC_CSR_ENABLE;

    dev->sc_flags |= SBICF_INTR;
    dev->sc_tcnt   = dev->sc_cur->dc_count << 1;

    /*
     * Prime the hardware.
     * Note, it's probably not necessary to do this here, since dmanext
     * is called just prior to the actual transfer.
     */
    pc->pcc_dmacsr   = 0;
    pc->pcc_dmairq   = dev->sc_ipl | PCC_IRQ_IEN | PCC_IRQ_INT;
    pc->pcc_dmadaddr = (unsigned long)dev->sc_cur->dc_addr;
    pc->pcc_dmabcnt  = (unsigned long)dev->sc_tcnt | (1 << 24);
    pc->pcc_dmacsr   = dev->sc_dmacmd;

    return(dev->sc_tcnt);
}

/*
 * Prime the hardware for the next DMA transfer
 */
int
wdsc_dmanext(dev)
    struct sbic_softc *dev;
{
    volatile struct pccreg *pc = dev->sc_cregs;

    if ( dev->sc_cur > dev->sc_last ) {
        /*
         * Shouldn't happen !!
         */
        printf("wdsc_dmanext at end !!!\n");
        wdsc_dmastop(dev);
        return(0);
    }

    dev->sc_tcnt = dev->sc_cur->dc_count << 1;

    /* 
     * Load the next DMA address
     */
    pc->pcc_dmacsr   = 0;
    pc->pcc_dmairq   = dev->sc_ipl | PCC_IRQ_IEN | PCC_IRQ_INT;
    pc->pcc_dmadaddr = (unsigned long)dev->sc_cur->dc_addr;
    pc->pcc_dmabcnt  = (unsigned long)dev->sc_tcnt | (1 << 24);
    pc->pcc_dmacsr   = dev->sc_dmacmd;

    return(dev->sc_tcnt);
}

/*
 * Stop DMA, and disable interrupts
 */
void
wdsc_dmastop(dev)
    struct sbic_softc *dev;
{
    volatile struct pccreg *pc = dev->sc_cregs;
    int                 s;

    s = splbio();

    pc->pcc_dmacsr    = 0;
    pc->pcc_dmairq    = dev->sc_ipl | PCC_IRQ_INT;

    splx(s);
}

/*
 * Come here following a DMA interrupt
 */
int
wdsc_dmaintr(dev)
    struct sbic_softc *dev;
{
    volatile struct pccreg *pc = dev->sc_cregs;
    int                 found = 0;

    /*
     * Really a DMA interrupt?
     */
    if ( (pc->pcc_dmairq & PCC_IRQ_INT) == 0 )
        return(0);

    /*
     * Was it a completion interrupt?
     * XXXSCW Note: Support for other DMA interrupts is required, eg. buserr
     */
    if ( pc->pcc_dmacsr & DMAC_CSR_DONE ) {
        ++found;

        pc->pcc_dmairq = dev->sc_ipl | PCC_IRQ_IEN | PCC_IRQ_INT;
    }

    return(found);
}

/*
 * Come here for SCSI interrupts
 */
int
wdsc_scsiintr(dev)
    struct sbic_softc *dev;
{
    volatile struct pccreg *pc = dev->sc_cregs;
    int                 found;

    /*
     * Really a SCSI interrupt?
     */
    if ( (pc->pcc_sbicirq & PCC_IRQ_INT) == 0 )
        return(0);

    /*
     * Go handle it
     */
    found = sbicintr(dev);

    /*
     * Acknowledge and clear the interrupt
     */
    pc->pcc_sbicirq = dev->sc_ipl | PCC_IRQ_IEN | PCC_IRQ_INT;

    return(found);
}
