/*	$OpenBSD: src/sys/net/pf_table.c,v 1.15 2003/01/07 00:21:07 dhartmei Exp $	*/

/*
 * Copyright (c) 2002 Cedric Berger
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/socket.h>
#include <sys/mbuf.h>
#include <sys/kernel.h>

#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/ip_ipsp.h>
#include <net/pfvar.h>

#define ACCEPT_FLAGS(oklist)			\
	do {					\
		if ((flags & ~(oklist)) &	\
		   PFR_FLAG_ALLMASK)		\
			return (EINVAL);	\
	} while (0)

#define	FILLIN_SIN(sin, addr)			\
	do {					\
		(sin).sin_len = sizeof(sin);	\
		(sin).sin_family = AF_INET;	\
		(sin).sin_addr = (addr);	\
	} while (0)

#define	FILLIN_SIN6(sin6, addr)			\
	do {					\
		(sin6).sin6_len = sizeof(sin6);	\
		(sin6).sin6_family = AF_INET6;	\
		(sin6).sin6_addr = (addr);	\
	} while (0)

#define	AF_BITS(af)		(((af)==AF_INET)?32:128)
#define	ADDR_NETWORK(ad)	((ad)->pfra_net < AF_BITS((ad)->pfra_af))
#define	KENTRY_NETWORK(ke)	((ke)->pfrke_net < AF_BITS((ke)->pfrke_af))

struct pfr_walktree {
	enum pfrw_op {
		PFRW_MARK,
		PFRW_SWEEP,
		PFRW_ENQUEUE,
		PFRW_GET_ADDRS,
		PFRW_GET_ASTATS
	}	 pfrw_op;
	union {
		struct pfr_addr		*pfrw1_addr;
		struct pfr_astats	*pfrw1_astats;
		struct pfr_kentryworkq	*pfrw1_workq;
	}	 pfrw_1;
	int	 pfrw_free;
};
#define pfrw_addr	pfrw_1.pfrw1_addr
#define pfrw_astats	pfrw_1.pfrw1_astats
#define pfrw_workq	pfrw_1.pfrw1_workq
#define pfrw_cnt	pfrw_free

#define senderr(e)	do { rv = (e); goto _bad; } while (0)

struct pool		 pfr_ktable_pl;
struct pool		 pfr_kentry_pl;
struct sockaddr_in	 pfr_sin;
struct sockaddr_in6	 pfr_sin6;

int			 pfr_validate_addr(struct pfr_addr *);
int			 pfr_enqueue_addrs(struct pfr_ktable *,
			    struct pfr_kentryworkq *, int *);
struct pfr_kentry	*pfr_lookup_addr(struct pfr_ktable *,
			    struct pfr_addr *, int);
struct pfr_kentry	*pfr_create_kentry(struct pfr_addr *, long);
void			 pfr_destroy_kentry(struct pfr_kentry *);
void			 pfr_destroy_kentries(struct pfr_kentryworkq *);
void			 pfr_insert_kentries(struct pfr_ktable *,
			    struct pfr_kentryworkq *);
void			 pfr_remove_kentries(struct pfr_ktable *,
			    struct pfr_kentryworkq *);
void			 pfr_clstats_kentries(struct pfr_kentryworkq *, long);
void			 pfr_reset_feedback(struct pfr_addr *, int);
void			 pfr_prepare_network(union sockaddr_union *, int, int);
int			 pfr_route_kentry(struct pfr_ktable *,
			    struct pfr_kentry *);
int			 pfr_unroute_kentry(struct pfr_ktable *,
			    struct pfr_kentry *);
void			 pfr_copyout_addr(struct pfr_addr *,
			    struct pfr_kentry *);
int			 pfr_walktree(struct radix_node *, void *);
void			 pfr_insert_ktables(struct pfr_ktableworkq *);
void			 pfr_remove_ktables(struct pfr_ktableworkq *);
void			 pfr_clstats_ktables(struct pfr_ktableworkq *, long,
			    int);
struct pfr_ktable	*pfr_create_ktable(struct pfr_table *, long);
void			 pfr_destroy_ktable(struct pfr_ktable *);
void			 pfr_destroy_ktables(struct pfr_ktableworkq *);
int			 pfr_ktable_compare(struct pfr_ktable *,
			    struct pfr_ktable *);
struct pfr_ktable	*pfr_lookup_table(struct pfr_table *);

RB_PROTOTYPE(pfr_ktablehead, pfr_ktable, pfrkt_tree, pfr_ktable_compare);
RB_GENERATE(pfr_ktablehead, pfr_ktable, pfrkt_tree, pfr_ktable_compare);

struct pfr_ktablehead	 pfr_ktables;
struct pfr_table	 pfr_nulltable;
int			 pfr_ktable_cnt;

void
pfr_initialize(void)
{
	pool_init(&pfr_ktable_pl, sizeof(struct pfr_ktable), 0, 0, 0,
	    "pfr_ktable", NULL);
	pool_init(&pfr_kentry_pl, sizeof(struct pfr_kentry), 0, 0, 0,
	    "pfr_kentry", NULL);

	pfr_sin.sin_len = sizeof(pfr_sin);
	pfr_sin.sin_family = AF_INET;
	pfr_sin6.sin6_len = sizeof(pfr_sin6);
	pfr_sin6.sin6_family = AF_INET6;
}

int
pfr_clr_addrs(struct pfr_table *tbl, int *ndel, int flags)
{
	struct pfr_ktable	*kt;
	struct pfr_kentryworkq	 workq;
	int			 s, rv;

	ACCEPT_FLAGS(PFR_FLAG_ATOMIC+PFR_FLAG_DUMMY);
	kt = pfr_lookup_table(tbl);
	if (kt == NULL || !(kt->pfrkt_flags & PFR_TFLAG_ACTIVE))
		return (ESRCH);
	rv = pfr_enqueue_addrs(kt, &workq, ndel);
	if (rv)
		return rv;

	if (!(flags & PFR_FLAG_DUMMY)) {
		if (flags & PFR_FLAG_ATOMIC)
			s = splsoftnet();
		pfr_remove_kentries(kt, &workq);
		if (flags & PFR_FLAG_ATOMIC)
			splx(s);
		if (kt->pfrkt_cnt) {
			printf("pfr_clr_addrs: corruption detected.");
			kt->pfrkt_cnt = 0;
		}
	}
	return (0);
}

int
pfr_add_addrs(struct pfr_table *tbl, struct pfr_addr *addr, int size,
    int *nadd, int flags)
{
	struct pfr_ktable	*kt, *tmpkt;
	struct pfr_kentryworkq	 workq;
	struct pfr_kentry	*p, *q;
	struct pfr_addr		 ad;
	int			 i, rv, s, xadd = 0;
	long			 tzero = time.tv_sec;

	ACCEPT_FLAGS(PFR_FLAG_ATOMIC+PFR_FLAG_DUMMY+PFR_FLAG_FEEDBACK);
	kt = pfr_lookup_table(tbl);
	if (kt == NULL || !(kt->pfrkt_flags & PFR_TFLAG_ACTIVE))
		return (ESRCH);
	tmpkt = pfr_create_ktable(&pfr_nulltable, 0);
	if (tmpkt == NULL)
		return (ENOMEM);
	SLIST_INIT(&workq);
	for (i = 0; i < size; i++) {
		if (copyin(addr+i, &ad, sizeof(ad)))
			senderr(EFAULT);
		if (pfr_validate_addr(&ad))
			senderr(EFAULT);
		p = pfr_lookup_addr(kt, &ad, 1);
		q = pfr_lookup_addr(tmpkt, &ad, 1);
		if (flags & PFR_FLAG_FEEDBACK) {
			ad.pfra_fback = (q != NULL) ? PFR_FB_DUPLICATE :
			    ((p == NULL) ? PFR_FB_ADDED : PFR_FB_NONE);
			if (copyout(&ad, addr+i, sizeof(ad)))
				senderr(EFAULT);
		}
		if (q != NULL)
			continue;
		if (p == NULL) {
			p = pfr_create_kentry(&ad, tzero);
			if (p == NULL)
				senderr(ENOMEM);
			SLIST_INSERT_HEAD(&workq, p, pfrke_workq);
			pfr_route_kentry(tmpkt, p);
			xadd++;
		} else if (p->pfrke_not != ad.pfra_not)
			senderr(EEXIST);
	}
	if (!(flags & PFR_FLAG_DUMMY)) {
		if (flags & PFR_FLAG_ATOMIC)
			s = splsoftnet();
		pfr_insert_kentries(kt, &workq);
		if (flags & PFR_FLAG_ATOMIC)
			splx(s);
	}
	if (nadd != NULL)
		*nadd = xadd;
	return (0);
_bad:
	pfr_destroy_kentries(&workq);
	if (flags & PFR_FLAG_FEEDBACK)
		pfr_reset_feedback(addr, size);
	if (tmpkt != NULL)
		pfr_destroy_ktable(tmpkt);
	return (rv);
}

int
pfr_del_addrs(struct pfr_table *tbl, struct pfr_addr *addr, int size,
    int *ndel, int flags)
{
	struct pfr_ktable	*kt;
	struct pfr_kentryworkq	 workq;
	struct pfr_walktree	 w;
	struct pfr_kentry	*p;
	struct pfr_addr		 ad;
	int			 i, rv, s, xdel = 0;

	ACCEPT_FLAGS(PFR_FLAG_ATOMIC+PFR_FLAG_DUMMY+PFR_FLAG_FEEDBACK);
	kt = pfr_lookup_table(tbl);
	if (kt == NULL || !(kt->pfrkt_flags & PFR_TFLAG_ACTIVE))
		return (ESRCH);

	bzero(&w, sizeof(w));
	w.pfrw_op = PFRW_MARK;
	rv = rn_walktree(kt->pfrkt_ip4, pfr_walktree, &w);
	if (!rv)
		rv = rn_walktree(kt->pfrkt_ip6, pfr_walktree, &w);
	if (rv)
		return (rv);

	SLIST_INIT(&workq);
	for (i = 0; i < size; i++) {
		if (copyin(addr+i, &ad, sizeof(ad)))
			senderr(EFAULT);
		if (pfr_validate_addr(&ad))
			senderr(EINVAL);
		p = pfr_lookup_addr(kt, &ad, 1);
		if (flags & PFR_FLAG_FEEDBACK) {
			ad.pfra_fback = (p == NULL) ? PFR_FB_NONE :
			    (p->pfrke_mark ? PFR_FB_DUPLICATE :
			    PFR_FB_DELETED);
			if (copyout(&ad, addr+i, sizeof(ad)))
				senderr(EFAULT);
		}
		if (p != NULL) {
			if (p->pfrke_mark)
				continue;
			p->pfrke_mark = 1;
			SLIST_INSERT_HEAD(&workq, p, pfrke_workq);
			xdel++;
		}
	}
	if (!(flags & PFR_FLAG_DUMMY)) {
		if (flags & PFR_FLAG_ATOMIC)
			s = splsoftnet();
		pfr_remove_kentries(kt, &workq);
		if (flags & PFR_FLAG_ATOMIC)
			splx(s);
	}
	if (ndel != NULL)
		*ndel = xdel;
	return (0);
_bad:
	if (flags & PFR_FLAG_FEEDBACK)
		pfr_reset_feedback(addr, size);
	return (rv);
}

int
pfr_set_addrs(struct pfr_table *tbl, struct pfr_addr *addr, int size,
    int *size2, int *nadd, int *ndel, int *nchange, int flags)
{
	struct pfr_ktable	*kt, *tmpkt;
	struct pfr_kentryworkq	 addq, delq, changeq;
	struct pfr_walktree	 w;
	struct pfr_kentry	*p, *q;
	struct pfr_addr		 ad;
	int			 i, rv, s, xadd = 0, xdel = 0, xchange = 0;
	long			 tzero = time.tv_sec;

	ACCEPT_FLAGS(PFR_FLAG_ATOMIC+PFR_FLAG_DUMMY+PFR_FLAG_FEEDBACK);
	kt = pfr_lookup_table(tbl);
	if (kt == NULL || !(kt->pfrkt_flags & PFR_TFLAG_ACTIVE))
		return (ESRCH);
	tmpkt = pfr_create_ktable(&pfr_nulltable, 0);
	if (tmpkt == NULL)
		return (ENOMEM);

	bzero(&w, sizeof(w));
	w.pfrw_op = PFRW_MARK;
	rv = rn_walktree(kt->pfrkt_ip4, pfr_walktree, &w);
	if (!rv)
		rv = rn_walktree(kt->pfrkt_ip6, pfr_walktree, &w);
	if (rv)
		return (rv);

	SLIST_INIT(&addq);
	SLIST_INIT(&delq);
	SLIST_INIT(&changeq);
	for (i = 0; i < size; i++) {
		if (copyin(addr+i, &ad, sizeof(ad)))
			senderr(EFAULT);
		if (pfr_validate_addr(&ad))
			senderr(EINVAL);
		p = pfr_lookup_addr(kt, &ad, 1);
		if (p != NULL) {
			if (p->pfrke_mark) {
				ad.pfra_fback = PFR_FB_DUPLICATE;
				goto _skip;
			}
			p->pfrke_mark = 1;
			if (p->pfrke_not != ad.pfra_not) {
				SLIST_INSERT_HEAD(&changeq, p, pfrke_workq);
				ad.pfra_fback = PFR_FB_CHANGED;
				xchange++;
			} else
				ad.pfra_fback = PFR_FB_NONE;
		} else {
			q = pfr_lookup_addr(tmpkt, &ad, 1);
			if (q != NULL) {
				ad.pfra_fback = PFR_FB_DUPLICATE;
				goto _skip;
			}
			p = pfr_create_kentry(&ad, tzero);
			if (p == NULL)
				senderr(ENOMEM);
			SLIST_INSERT_HEAD(&addq, p, pfrke_workq);
			pfr_route_kentry(tmpkt, p);
			ad.pfra_fback = PFR_FB_ADDED;
			xadd++;
		}
_skip:
		if (flags & PFR_FLAG_FEEDBACK)
			if (copyout(&ad, addr+i, sizeof(ad)))
				senderr(EFAULT);
	}
	w.pfrw_op = PFRW_SWEEP;
	w.pfrw_workq = &delq;
	rv = rn_walktree(kt->pfrkt_ip4, pfr_walktree, &w);
	if (!rv)
		rv = rn_walktree(kt->pfrkt_ip6, pfr_walktree, &w);
	if (rv)
		senderr(rv);
	xdel = w.pfrw_cnt;
	if ((flags & PFR_FLAG_FEEDBACK) && *size2) {
		if (*size2 < size+xdel) {
			*size2 = size+xdel;
			senderr(0);
		}
		i = 0;
		SLIST_FOREACH(p, &delq, pfrke_workq) {
			pfr_copyout_addr(&ad, p);
			ad.pfra_fback = PFR_FB_DELETED;
			if (copyout(&ad, addr+size+i, sizeof(ad)))
				senderr(EFAULT);
			i++;
		}
	}
	if (!(flags & PFR_FLAG_DUMMY)) {
		if (flags & PFR_FLAG_ATOMIC)
			s = splsoftnet();
		pfr_insert_kentries(kt, &addq);
		pfr_remove_kentries(kt, &delq);
		SLIST_FOREACH(p, &changeq, pfrke_workq)
			p->pfrke_not ^= 1;
		pfr_clstats_kentries(&changeq, time.tv_sec);
		if (flags & PFR_FLAG_ATOMIC)
			splx(s);
	}
	if (nadd != NULL)
		*nadd = xadd;
	if (ndel != NULL)
		*ndel = xdel;
	if (nchange != NULL)
		*nchange = xchange;
	if ((flags & PFR_FLAG_FEEDBACK) && *size2)
		*size2 = size+xdel;
	return (0);
_bad:
	pfr_destroy_kentries(&addq);
	if (flags & PFR_FLAG_FEEDBACK)
		pfr_reset_feedback(addr, size);
	if (tmpkt != NULL)
		pfr_destroy_ktable(tmpkt);
	return (rv);
}

int
pfr_tst_addrs(struct pfr_table *tbl, struct pfr_addr *addr, int size,
	int *nmatch, int flags)
{
	struct pfr_ktable	*kt;
	struct pfr_kentry	*p;
	struct pfr_addr		 ad;
	int			 i, xmatch = 0;

	ACCEPT_FLAGS(PFR_FLAG_REPLACE);
	kt = pfr_lookup_table(tbl);
	if (kt == NULL || !(kt->pfrkt_flags & PFR_TFLAG_ACTIVE))
		return (ESRCH);

	for (i = 0; i < size; i++) {
		if (copyin(addr+i, &ad, sizeof(ad)))
			return (EFAULT);
		if (pfr_validate_addr(&ad))
			return (EINVAL);
		if (ADDR_NETWORK(&ad))
			return (EINVAL);
		p = pfr_lookup_addr(kt, &ad, 0);
		if (flags & PFR_FLAG_REPLACE)
			pfr_copyout_addr(&ad, p);
		ad.pfra_fback = (p == NULL) ? PFR_FB_NONE :
		    (p->pfrke_not ? PFR_FB_NOTMATCH : PFR_FB_MATCH);
		if (p != NULL && !p->pfrke_not)
			xmatch++;
		if (copyout(&ad, addr+i, sizeof(ad)))
			return (EFAULT);
	}
	if (nmatch != NULL)
		*nmatch = xmatch;
	return (0);
}

int
pfr_get_addrs(struct pfr_table *tbl, struct pfr_addr *addr, int *size,
	int flags)
{
	struct pfr_ktable	*kt;
	struct pfr_walktree	 w;
	int			 rv;

	ACCEPT_FLAGS(0);
	kt = pfr_lookup_table(tbl);
	if (kt == NULL || !(kt->pfrkt_flags & PFR_TFLAG_ACTIVE))
		return (ESRCH);
	if (kt->pfrkt_cnt > *size) {
		*size = kt->pfrkt_cnt;
		return (0);
	}

	bzero(&w, sizeof(w));
	w.pfrw_op = PFRW_GET_ADDRS;
	w.pfrw_addr = addr;
	w.pfrw_free = kt->pfrkt_cnt;
	rv = rn_walktree(kt->pfrkt_ip4, pfr_walktree, &w);
	if (!rv)
		rv = rn_walktree(kt->pfrkt_ip6, pfr_walktree, &w);
	if (rv)
		return (rv);

	if (w.pfrw_free) {
		printf("pfr_get_addrs: corruption detected.");
		return (ENOTTY);
	}
	*size = kt->pfrkt_cnt;
	return (0);
}

int
pfr_get_astats(struct pfr_table *tbl, struct pfr_astats *addr, int *size,
	int flags)
{
	struct pfr_ktable	*kt;
	struct pfr_walktree	 w;
	struct pfr_kentryworkq	 workq;
	int			 rv, s;
	long			 tzero = time.tv_sec;

	ACCEPT_FLAGS(PFR_FLAG_ATOMIC); /* XXX PFR_FLAG_CLSTATS disabled */
	kt = pfr_lookup_table(tbl);
	if (kt == NULL || !(kt->pfrkt_flags & PFR_TFLAG_ACTIVE))
		return (ESRCH);
	if (kt->pfrkt_cnt > *size) {
		*size = kt->pfrkt_cnt;
		return (0);
	}

	bzero(&w, sizeof(w));
	w.pfrw_op = PFRW_GET_ASTATS;
	w.pfrw_astats = addr;
	w.pfrw_free = kt->pfrkt_cnt;
	if (flags & PFR_FLAG_ATOMIC)
		s = splsoftnet();
	rv = rn_walktree(kt->pfrkt_ip4, pfr_walktree, &w);
	if (!rv)
		rv = rn_walktree(kt->pfrkt_ip6, pfr_walktree, &w);
	if (!rv && (flags & PFR_FLAG_CLSTATS)) {
		rv = pfr_enqueue_addrs(kt, &workq, NULL);
		if (rv)
			return rv;
		pfr_clstats_kentries(&workq, tzero);
	}
	if (flags & PFR_FLAG_ATOMIC)
		splx(s);
	if (rv)
		return (rv);

	if (w.pfrw_free) {
		printf("pfr_get_astats: corruption detected.");
		return (ENOTTY);
	}
	*size = kt->pfrkt_cnt;
	return (0);
}

int
pfr_clr_astats(struct pfr_table *tbl, struct pfr_addr *addr, int size,
    int *nzero, int flags)
{
	struct pfr_ktable	*kt;
	struct pfr_kentryworkq	 workq;
	struct pfr_kentry	*p;
	struct pfr_addr		 ad;
	int			 i, rv, s, xzero = 0;

	ACCEPT_FLAGS(PFR_FLAG_ATOMIC+PFR_FLAG_DUMMY+PFR_FLAG_FEEDBACK);
	kt = pfr_lookup_table(tbl);
	if (kt == NULL || !(kt->pfrkt_flags & PFR_TFLAG_ACTIVE))
		return (ESRCH);
	SLIST_INIT(&workq);
	for (i = 0; i < size; i++) {
		if (copyin(addr+i, &ad, sizeof(ad)))
			senderr(EFAULT);
		if (pfr_validate_addr(&ad))
			senderr(EINVAL);
		p = pfr_lookup_addr(kt, &ad, 1);
		if (flags & PFR_FLAG_FEEDBACK) {
			ad.pfra_fback = (p != NULL) ?
			    PFR_FB_CLEARED : PFR_FB_NONE;
			if (copyout(&ad, addr+i, sizeof(ad)))
				senderr(EFAULT);
		}
		if (p != NULL) {
			SLIST_INSERT_HEAD(&workq, p, pfrke_workq);
			xzero++;
		}
	}

	if (!(flags & PFR_FLAG_DUMMY)) {
		if (flags & PFR_FLAG_ATOMIC)
			s = splsoftnet();
		pfr_clstats_kentries(&workq, 0);
		if (flags & PFR_FLAG_ATOMIC)
			splx(s);
	}
	if (nzero != NULL)
		*nzero = xzero;
	return (0);
_bad:
	if (flags & PFR_FLAG_FEEDBACK)
		pfr_reset_feedback(addr, size);
	return (rv);
}


int
pfr_validate_addr(struct pfr_addr *ad)
{
	switch (ad->pfra_af) {
	case AF_INET:
		if (ad->pfra_af > 32)
			return (-1);
		return (0);
	case AF_INET6:
		if (ad->pfra_af > 128)
			return (-1);
		return (0);
	default:
		return (-1);
	}
}

int
pfr_enqueue_addrs(struct pfr_ktable *kt, struct pfr_kentryworkq *workq,
	int *naddr)
{
	struct pfr_walktree	w;
	int			rv;

	SLIST_INIT(workq);
	bzero(&w, sizeof(w));
	w.pfrw_op = PFRW_ENQUEUE;
	w.pfrw_workq = workq;
	rv = rn_walktree(kt->pfrkt_ip4, pfr_walktree, &w);
	if (rv)
		return (rv);
	rv = rn_walktree(kt->pfrkt_ip6, pfr_walktree, &w);
	if (rv)
		return (rv);
	if (naddr != NULL)
		*naddr = w.pfrw_cnt;
	return (0);
}

struct pfr_kentry *
pfr_lookup_addr(struct pfr_ktable *kt, struct pfr_addr *ad, int exact)
{
	union sockaddr_union	 sa, mask;
	struct radix_node_head	*head;
	struct pfr_kentry	*ke;

	bzero(&sa, sizeof(sa));
	if (ad->pfra_af == AF_INET) {
		FILLIN_SIN(sa.sin, ad->pfra_ip4addr);
		head = kt->pfrkt_ip4;
	} else {
		FILLIN_SIN6(sa.sin6, ad->pfra_ip6addr);
		head = kt->pfrkt_ip6;
	}
	if (ADDR_NETWORK(ad)) {
		pfr_prepare_network(&mask, ad->pfra_af, ad->pfra_net);
		ke = (struct pfr_kentry *)rn_lookup(&sa, &mask, head);
	} else {
		ke = (struct pfr_kentry *)rn_match(&sa, head);
		if (exact && ke && KENTRY_NETWORK(ke))
			ke = NULL;
	}
	return (ke);
}

struct pfr_kentry *
pfr_create_kentry(struct pfr_addr *ad, long tzero)
{
	struct pfr_kentry	*ke;

	ke = pool_get(&pfr_kentry_pl, PR_NOWAIT);
	if (ke == NULL)
		return (NULL);
	bzero(ke, sizeof(*ke));

	if (ad->pfra_af == AF_INET)
		FILLIN_SIN(ke->pfrke_sa.sin, ad->pfra_ip4addr);
	else
		FILLIN_SIN6(ke->pfrke_sa.sin6, ad->pfra_ip6addr);
	ke->pfrke_af = ad->pfra_af;
	ke->pfrke_net = ad->pfra_net;
	ke->pfrke_not = ad->pfra_not;
	ke->pfrke_tzero = tzero;
	return (ke);
}

void
pfr_destroy_kentry(struct pfr_kentry *ke)
{
	if (ke != NULL)
		pool_put(&pfr_kentry_pl, ke);
}

void
pfr_destroy_kentries(struct pfr_kentryworkq *workq)
{
	struct pfr_kentry	*p, *q;

	for (p = SLIST_FIRST(workq); p != NULL; p = q) {
		q = SLIST_NEXT(p, pfrke_workq);
		pfr_destroy_kentry(p);
	}
}

void
pfr_insert_kentries(struct pfr_ktable *kt,
    struct pfr_kentryworkq *workq)
{
	struct pfr_kentry	*p;
	int			 rv, n = 0;

	SLIST_FOREACH(p, workq, pfrke_workq) {
		rv = pfr_route_kentry(kt, p);
		if (rv) {
			printf("pfr_insert_kentries: cannot route entry "
			    "(code=%d).\n", rv);
			break;
		}
		n++;
	}
	kt->pfrkt_cnt += n;
}

void
pfr_remove_kentries(struct pfr_ktable *kt,
    struct pfr_kentryworkq *workq)
{
	struct pfr_kentry	*p;
	int			 n = 0;

	SLIST_FOREACH(p, workq, pfrke_workq) {
		pfr_unroute_kentry(kt, p);
		n++;
	}
	kt->pfrkt_cnt -= n;
	pfr_destroy_kentries(workq);
}

void
pfr_clstats_kentries(struct pfr_kentryworkq *workq, long tzero)
{
	struct pfr_kentry	*p;
	int			 s, n = 0;

	SLIST_FOREACH(p, workq, pfrke_workq) {
		s = splsoftnet();
		bzero(p->pfrke_packets, sizeof(p->pfrke_packets));
		bzero(p->pfrke_bytes, sizeof(p->pfrke_bytes));
		splx(s);
		p->pfrke_tzero = tzero;
		n++;
	}
}

void
pfr_reset_feedback(struct pfr_addr *addr, int size)
{
	struct pfr_addr	ad;
	int		i;

	for (i = 0; i < size; i++) {
		if (copyin(addr+i, &ad, sizeof(ad)))
			break;
		ad.pfra_fback = PFR_FB_NONE;
		if (copyout(&ad, addr+i, sizeof(ad)))
			break;
	}
}

void
pfr_prepare_network(union sockaddr_union *sa, int af, int net)
{
	int	i;

	bzero(sa, sizeof(*sa));
	if (af == AF_INET) {
		sa->sin.sin_len = sizeof(sa->sin);
		sa->sin.sin_family = AF_INET;
		sa->sin.sin_addr.s_addr = htonl(-1 << (32-net));
	} else {
		sa->sin6.sin6_len = sizeof(sa->sin6);
		sa->sin6.sin6_family = AF_INET;
		for (i = 0; i < 4; i++) {
			if (net <= 32) {
				sa->sin6.sin6_addr.s6_addr32[i] =
				    htonl(-1 << (32-net));
				break;
			}
			sa->sin6.sin6_addr.s6_addr32[i] = 0xFFFFFFFF;
			net -= 32;
		}
	}
}

int
pfr_route_kentry(struct pfr_ktable *kt, struct pfr_kentry *ke)
{
	union sockaddr_union	 mask;
	struct radix_node	*rn;
	struct radix_node_head	*head;
	int			 s;

	bzero(ke->pfrke_node, sizeof(ke->pfrke_node));
	if (ke->pfrke_af == AF_INET)
		head = kt->pfrkt_ip4;
	else
		head = kt->pfrkt_ip6;

	s = splsoftnet();
	if (KENTRY_NETWORK(ke)) {
		pfr_prepare_network(&mask, ke->pfrke_af, ke->pfrke_net);
		rn = rn_addroute(&ke->pfrke_sa, &mask, head, ke->pfrke_node);
	} else
		rn = rn_addroute(&ke->pfrke_sa, NULL, head, ke->pfrke_node);
	splx(s);

	if (rn == NULL) {
		printf("pfr_route_kentry: no memory for mask\n");
		return (-1);
	}
	return (0);
}

int
pfr_unroute_kentry(struct pfr_ktable *kt, struct pfr_kentry *ke)
{
	union sockaddr_union	 mask;
	struct radix_node	*rn;
	struct radix_node_head	*head;
	int			 s;

	if (ke->pfrke_af == AF_INET)
		head = kt->pfrkt_ip4;
	else
		head = kt->pfrkt_ip6;

	s = splsoftnet();
	if (KENTRY_NETWORK(ke)) {
		pfr_prepare_network(&mask, ke->pfrke_af, ke->pfrke_net);
		rn = rn_delete(&ke->pfrke_sa, &mask, head);
	} else
		rn = rn_delete(&ke->pfrke_sa, NULL, head);
	splx(s);

	if (rn == NULL) {
		printf("pfr_unroute_kentry: delete failed\n");
		return (-1);
	}
	return (0);
}

void
pfr_copyout_addr(struct pfr_addr *ad, struct pfr_kentry *ke)
{
	bzero(ad, sizeof(*ad));
	if (ke == NULL)
		return;
	ad->pfra_af = ke->pfrke_af;
	ad->pfra_net = ke->pfrke_net;
	ad->pfra_not = ke->pfrke_not;
	if (ad->pfra_af == AF_INET)
		ad->pfra_ip4addr = ke->pfrke_sa.sin.sin_addr;
	else
		ad->pfra_ip6addr = ke->pfrke_sa.sin6.sin6_addr;
}

int
pfr_walktree(struct radix_node *rn, void *arg)
{
	struct pfr_kentry	*ke = (struct pfr_kentry *)rn;
	struct pfr_walktree	*w = arg;
	int			 s;

	switch (w->pfrw_op) {
	case PFRW_MARK:
		ke->pfrke_mark = 0;
		break;
	case PFRW_SWEEP:
		if (ke->pfrke_mark)
			break;
		/* fall trough */
	case PFRW_ENQUEUE:
		SLIST_INSERT_HEAD(w->pfrw_workq, ke, pfrke_workq);
		w->pfrw_cnt++;
		break;
	case PFRW_GET_ADDRS:
		if (w->pfrw_free-- > 0) {
			struct pfr_addr ad;

			pfr_copyout_addr(&ad, ke);
			if (copyout(&ad, w->pfrw_addr, sizeof(ad)))
				return (EFAULT);
			w->pfrw_addr++;
		}
		break;
	case PFRW_GET_ASTATS:
		if (w->pfrw_free-- > 0) {
			struct pfr_astats as;

			pfr_copyout_addr(&as.pfras_a, ke);

			s = splsoftnet();
			bcopy(ke->pfrke_packets, as.pfras_packets,
			    sizeof(as.pfras_packets));
			bcopy(ke->pfrke_bytes, as.pfras_bytes,
			    sizeof(as.pfras_bytes));
			splx(s);
			as.pfras_tzero = ke->pfrke_tzero;

			if (copyout(&as, w->pfrw_astats, sizeof(as)))
				return (EFAULT);
			w->pfrw_astats++;
		}
		break;
	}
	return (0);
}


int
pfr_clr_tables(int *ndel, int flags)
{
	struct pfr_ktableworkq	 workq;
	struct pfr_ktable	*p;
	int			 s, xdel = 0;

	ACCEPT_FLAGS(PFR_FLAG_ATOMIC+PFR_FLAG_DUMMY);
	SLIST_INIT(&workq);
	RB_FOREACH(p, pfr_ktablehead, &pfr_ktables) {
		SLIST_INSERT_HEAD(&workq, p, pfrkt_workq);
		xdel++;
	}

	if (!(flags & PFR_FLAG_DUMMY)) {
		if (flags & PFR_FLAG_ATOMIC)
			s = splsoftnet();
		pfr_remove_ktables(&workq);
		if (flags & PFR_FLAG_ATOMIC)
			splx(s);
	}
	if (ndel != NULL)
		*ndel = xdel;
	return (0);
}

int
pfr_add_tables(struct pfr_table *tbl, int size, int *nadd, int flags)
{
	struct pfr_ktableworkq	 workq, changeq;
	struct pfr_ktable	*p, *q, key;
	int			 i, rv, s, xadd = 0;
	long			 tzero = time.tv_sec;

	ACCEPT_FLAGS(PFR_FLAG_ATOMIC+PFR_FLAG_DUMMY);
	SLIST_INIT(&workq);
	SLIST_INIT(&changeq);
	for (i = 0; i < size; i++) {
		if (copyin(tbl+i, &key.pfrkt_t, sizeof(key.pfrkt_t)))
			senderr(EFAULT);
		if (key.pfrkt_name[PF_TABLE_NAME_SIZE-1])
			senderr(EINVAL);
		key.pfrkt_flags = PFR_TFLAG_ACTIVE+PFR_TFLAG_PERSIST;
		p = RB_FIND(pfr_ktablehead, &pfr_ktables, &key);
		if (p == NULL) {
			p = pfr_create_ktable(&key.pfrkt_t, tzero);
			if (p == NULL)
				senderr(ENOMEM);
			SLIST_FOREACH(q, &workq, pfrkt_workq) {
				if (!strcmp(p->pfrkt_name, q->pfrkt_name))
					goto _skip;
			}
			SLIST_INSERT_HEAD(&workq, p, pfrkt_workq);
			xadd++;
		} else if (!(p->pfrkt_flags & PFR_TFLAG_PERSIST)) {
			SLIST_INSERT_HEAD(&changeq, p, pfrkt_workq);
		}
_skip:
	}
	if (!(flags & PFR_FLAG_DUMMY)) {
		if (flags & PFR_FLAG_ATOMIC)
			s = splsoftnet();
		pfr_insert_ktables(&workq);
		SLIST_FOREACH(p, &changeq, pfrkt_workq)
			p->pfrkt_flags |= PFR_TFLAG_PERSIST;
		if (flags & PFR_FLAG_ATOMIC)
			splx(s);
	} else
		 pfr_destroy_ktables(&workq);
	if (nadd != NULL)
		*nadd = xadd;
	return (0);
_bad:
	pfr_destroy_ktables(&workq);
	return (rv);
}

int
pfr_del_tables(struct pfr_table *tbl, int size, int *ndel, int flags)
{
	struct pfr_ktableworkq	 workq, changeq;
	struct pfr_ktable	*p, *q, key;
	int			 i, s, xdel = 0;

	ACCEPT_FLAGS(PFR_FLAG_ATOMIC+PFR_FLAG_DUMMY);
	SLIST_INIT(&workq);
	SLIST_INIT(&changeq);
	for (i = 0; i < size; i++) {
		if (copyin(tbl+i, &key.pfrkt_t, sizeof(key.pfrkt_t)))
			return (EFAULT);
		p = RB_FIND(pfr_ktablehead, &pfr_ktables, &key);
		if (p != NULL) {
                        struct pfr_ktableworkq *queue;

                        queue = (p->pfrkt_refcnt > 0) ? &changeq : &workq;
			SLIST_FOREACH(q, queue, pfrkt_workq)
				if (!strcmp(p->pfrkt_name, q->pfrkt_name))
					goto _skip;
			SLIST_INSERT_HEAD(queue, p, pfrkt_workq);
			if (queue == &workq)
				xdel++;
		}
_skip:
	}

	if (!(flags & PFR_FLAG_DUMMY)) {
		if (flags & PFR_FLAG_ATOMIC)
			s = splsoftnet();
		pfr_remove_ktables(&workq);
		SLIST_FOREACH(p, &changeq, pfrkt_workq)
			p->pfrkt_flags &= ~PFR_TFLAG_PERSIST;
		if (flags & PFR_FLAG_ATOMIC)
			splx(s);
	}
	if (ndel != NULL)
		*ndel = xdel;
	return (0);
}

int
pfr_get_tables(struct pfr_table *tbl, int *size, int flags)
{
	struct pfr_ktable	*p;
	int			 n = pfr_ktable_cnt;

	ACCEPT_FLAGS(0);
	if (n > *size) {
		*size = n;
		return (0);
	}
	RB_FOREACH(p, pfr_ktablehead, &pfr_ktables) {
		if (n-- <= 0)
			continue;
		if (copyout(&p->pfrkt_t, tbl++, sizeof(*tbl)))
			return (EFAULT);
	}
	if (n) {
		printf("pfr_get_tables: corruption detected.");
		return (ENOTTY);
	}
	*size = pfr_ktable_cnt;
	return (0);
}

int
pfr_get_tstats(struct pfr_tstats *tbl, int *size, int flags)
{
	struct pfr_ktable	*p;
	struct pfr_ktableworkq	 workq;
	int			 s, n = pfr_ktable_cnt;
	long			 tzero = time.tv_sec;

	ACCEPT_FLAGS(PFR_FLAG_ATOMIC); /* XXX PFR_FLAG_CLSTATS disabled */
	if (n > *size) {
		*size = n;
		return (0);
	}
	SLIST_INIT(&workq);
	if (flags & PFR_FLAG_ATOMIC)
		s = splsoftnet();
	RB_FOREACH(p, pfr_ktablehead, &pfr_ktables) {
		if (n-- <= 0)
			continue;
		if (!(flags & PFR_FLAG_ATOMIC))
			s = splsoftnet();
		if (copyout(&p->pfrkt_ts, tbl++, sizeof(*tbl))) {
			splx(s);
			return (EFAULT);
		}
		if (!(flags & PFR_FLAG_ATOMIC))
			splx(s);
		SLIST_INSERT_HEAD(&workq, p, pfrkt_workq);
	}
	if (flags & PFR_FLAG_CLSTATS)
		pfr_clstats_ktables(&workq, tzero,
		    flags & PFR_FLAG_RECURSE);
	if (flags & PFR_FLAG_ATOMIC)
		splx(s);
	if (n) {
		printf("pfr_get_tstats: corruption detected.");
		return (ENOTTY);
	}
	*size = pfr_ktable_cnt;
	return (0);
}

int
pfr_clr_tstats(struct pfr_table *tbl, int size, int *nzero, int flags)
{
	struct pfr_ktableworkq	 workq;
	struct pfr_ktable	*p, key;
	int			 i, s, xzero = 0;
	long			 tzero = time.tv_sec;

	ACCEPT_FLAGS(PFR_FLAG_ATOMIC+PFR_FLAG_CLSTATS+PFR_FLAG_RECURSE);
	SLIST_INIT(&workq);
	for (i = 0; i < size; i++) {
		if (copyin(tbl+i, &key.pfrkt_t, sizeof(key.pfrkt_t)))
			return (EFAULT);
		p = RB_FIND(pfr_ktablehead, &pfr_ktables, &key);
		if (p != NULL) {
			SLIST_INSERT_HEAD(&workq, p, pfrkt_workq);
			xzero++;
		}
	}

	if (!(flags & PFR_FLAG_DUMMY)) {
		if (flags & PFR_FLAG_ATOMIC)
			s = splsoftnet();
		pfr_clstats_ktables(&workq, tzero, flags & PFR_FLAG_RECURSE);
		if (flags & PFR_FLAG_ATOMIC)
			splx(s);
	}
	if (nzero != NULL)
		*nzero = xzero;
	return (0);
}

void
pfr_insert_ktables(struct pfr_ktableworkq *workq)
{
	struct pfr_ktable	*p;
	int			 n = 0;

	/* insert into tree */
	SLIST_FOREACH(p, workq, pfrkt_workq) {
		RB_INSERT(pfr_ktablehead, &pfr_ktables, p);
		n++;
	}
	pfr_ktable_cnt += n;
}

void
pfr_remove_ktables(struct pfr_ktableworkq *workq)
{
	struct pfr_kentryworkq	 addrq;
	struct pfr_ktable	*p;
	int			 n = 0;

	SLIST_FOREACH(p, workq, pfrkt_workq) {
		RB_REMOVE(pfr_ktablehead, &pfr_ktables, p);
		if (pfr_enqueue_addrs(p, &addrq, NULL))
			printf("pfr_remove_ktables: enqueue failed");
		pfr_destroy_kentries(&addrq);
		n++;
	}
	pfr_ktable_cnt -= n;
	pfr_destroy_ktables(workq);
}

void
pfr_clstats_ktables(struct pfr_ktableworkq *workq, long tzero, int recurse)
{
	struct pfr_kentryworkq	 addrq;
	struct pfr_ktable	*p;
	int			 s;

	SLIST_FOREACH(p, workq, pfrkt_workq) {
		if (recurse) {
			if (pfr_enqueue_addrs(p, &addrq, NULL))
				printf("pfr_clr_tstats: enqueue failed");
			pfr_clstats_kentries(&addrq, tzero);
		}
		s = splsoftnet();
		bzero(p->pfrkt_packets, sizeof(p->pfrkt_packets));
		bzero(p->pfrkt_bytes, sizeof(p->pfrkt_bytes));
		p->pfrkt_match = p->pfrkt_nomatch = 0;
		splx(s);
		p->pfrkt_tzero = tzero;
	}
}

struct pfr_ktable *
pfr_create_ktable(struct pfr_table *tbl, long tzero)
{
	struct pfr_ktable	*kt;

	kt = pool_get(&pfr_ktable_pl, PR_NOWAIT);
	if (kt == NULL)
		return (NULL);
	bzero(kt, sizeof(*kt));
	kt->pfrkt_t = *tbl;

	if (!rn_inithead((void **)&kt->pfrkt_ip4,
	    offsetof(struct sockaddr_in, sin_addr) * 8) ||
	    !rn_inithead((void **)&kt->pfrkt_ip6,
	    offsetof(struct sockaddr_in6, sin6_addr) * 8)) {
		pfr_destroy_ktable(kt);
		return (NULL);
	}
	kt->pfrkt_tzero = tzero;

	return (kt);
}

void
pfr_destroy_ktable(struct pfr_ktable *kt)
{
	if (kt == NULL)
		return;
	if (kt->pfrkt_ip4 != NULL)
		free((caddr_t)kt->pfrkt_ip4, M_RTABLE);
	if (kt->pfrkt_ip6 != NULL)
		free((caddr_t)kt->pfrkt_ip6, M_RTABLE);
	pool_put(&pfr_ktable_pl, kt);
}

void
pfr_destroy_ktables(struct pfr_ktableworkq *workq)
{
	struct pfr_ktable	*p, *q;

	for (p = SLIST_FIRST(workq); p; p = q) {
		q = SLIST_NEXT(p, pfrkt_workq);
		pfr_destroy_ktable(p);
	}
}

int
pfr_ktable_compare(struct pfr_ktable *p, struct pfr_ktable *q)
{
	return (strncmp(p->pfrkt_name, q->pfrkt_name, PF_TABLE_NAME_SIZE));
}

struct pfr_ktable *
pfr_lookup_table(struct pfr_table *tbl)
{
	/* struct pfr_ktable start like a struct pfr_table */
	return RB_FIND(pfr_ktablehead, &pfr_ktables, (struct pfr_ktable *)tbl);
}


/*
 * Return 1 if the addresses a and b match (with mask m), otherwise return 0.
 * If n is 0, they match if they are equal. If n is != 0, they match if they
 * are different.
 */
int
pfr_match_addr(struct pfr_ktable *kt, struct pf_addr *a, sa_family_t af)
{
	struct pfr_kentry	*ke = NULL;
	int			 match;

	switch (af) {
	case AF_INET:
		pfr_sin.sin_addr.s_addr = a->addr32[0];
		ke = (struct pfr_kentry *)rn_match(&pfr_sin, kt->pfrkt_ip4);
		break;
	case AF_INET6:
		bcopy(&a, &pfr_sin6.sin6_addr, sizeof(pfr_sin6.sin6_addr));
		ke = (struct pfr_kentry *)rn_match(&pfr_sin6, kt->pfrkt_ip6);
		break;
	}
	match = (ke && !ke->pfrke_not);
	if (match)
		kt->pfrkt_match++;
	else
		kt->pfrkt_nomatch++;
	return (match);
}

void
pfr_update_stats(struct pfr_ktable *kt, struct pf_addr *a, sa_family_t af,
    u_int64_t len, int dir_out, int op_pass, int notrule)
{
	struct pfr_kentry	*ke = NULL;

	switch (af) {
	case AF_INET:
		pfr_sin.sin_addr.s_addr = a->addr32[0];
		ke = (struct pfr_kentry *)rn_match(&pfr_sin, kt->pfrkt_ip4);
		break;
	case AF_INET6:
		bcopy(&a, &pfr_sin6.sin6_addr, sizeof(pfr_sin6.sin6_addr));
		ke = (struct pfr_kentry *)rn_match(&pfr_sin6, kt->pfrkt_ip6);
		break;
	}
	if (ke == NULL || ke->pfrke_not != notrule) {
		if (op_pass != PFR_OP_PASS)
			printf("pfr_update_stats: assertion failed.");
		op_pass = PFR_OP_XPASS;
	}
	kt->pfrkt_packets[dir_out][op_pass]++;
	kt->pfrkt_bytes[dir_out][op_pass] += len;
	if (op_pass != PFR_OP_XPASS) {
		ke->pfrke_packets[dir_out][op_pass]++;
		ke->pfrke_bytes[dir_out][op_pass] += len;
	}
}

struct pfr_ktable *
pfr_attach_table(char *name)
{
	struct pfr_ktable *p, key;
	struct pfr_ktableworkq workq;

	bzero(&key, sizeof(key));
	strlcpy(key.pfrkt_name, name, sizeof(key.pfrkt_name));
	key.pfrkt_flags = PFR_TFLAG_ACTIVE;
	p = RB_FIND(pfr_ktablehead, &pfr_ktables, &key);
	if (p == NULL) {
		p = pfr_create_ktable(&key.pfrkt_t, time.tv_sec);
		if (p == NULL)
			return NULL;
		SLIST_INIT(&workq);
		SLIST_INSERT_HEAD(&workq, p, pfrkt_workq);
		pfr_insert_ktables(&workq);
	}
	p->pfrkt_refcnt++;
	return p;
}

void
pfr_detach_table(struct pfr_ktable *kt)
{
	struct pfr_ktableworkq workq;

	if (kt->pfrkt_refcnt <= 0)
		printf("pfr_detach_table, refcount = %d\n",
		    kt->pfrkt_refcnt);
	else {
		kt->pfrkt_refcnt--;
		if (kt->pfrkt_refcnt == 0 &&
		    !(kt->pfrkt_flags & PFR_TFLAG_PERSIST)) {
			kt->pfrkt_flags &= ~PFR_TFLAG_ACTIVE;
			SLIST_INIT(&workq);
			SLIST_INSERT_HEAD(&workq, kt, pfrkt_workq);
			pfr_remove_ktables(&workq);
		}
	}
}
