/*	$NetBSD: output.c,v 1.9 1995/06/20 22:27:54 christos Exp $	*/

/*
 * Copyright (c) 1983, 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
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
 */

#ifndef lint
#if 0
static char sccsid[] = "@(#)output.c	8.1 (Berkeley) 6/5/93";
#else
static char rcsid[] = "$NetBSD: output.c,v 1.9 1995/06/20 22:27:54 christos Exp $";
#endif
#endif /* not lint */

/*
 * Routing Table Management Daemon
 */
#include "defs.h"

/*
 * Apply the function "f" to all non-passive
 * interfaces.  If the interface supports the
 * use of broadcasting use it, otherwise address
 * the output to the known router.
 */
void
toall(f, rtstate, skipif)
	void (*f) __P((struct sockaddr *, int, struct interface *, int));
	int rtstate;
	struct interface *skipif;
{
	register struct interface *ifp;
	register struct sockaddr *dst;
	register int flags;
	extern struct interface *ifnet;

	for (ifp = ifnet; ifp; ifp = ifp->int_next) {
		if (ifp->int_flags & IFF_PASSIVE || ifp == skipif)
			continue;
		dst = ifp->int_flags & IFF_BROADCAST ? &ifp->int_broadaddr :
		      ifp->int_flags & IFF_POINTOPOINT ? &ifp->int_dstaddr :
		      &ifp->int_addr;
		flags = ifp->int_flags & IFF_INTERFACE ? MSG_DONTROUTE : 0;
		(*f)(dst, flags, ifp, rtstate);
	}
}

/*
 * Output a preformed packet.
 */
/*ARGSUSED*/
void
sndmsg(dst, flags, ifp, rtstate)
	struct sockaddr *dst;
	int flags;
	struct interface *ifp;
	int rtstate;
{

	(*afswitch[dst->sa_family].af_output)(s, flags,
		dst, sizeof (struct rip));
	TRACE_OUTPUT(ifp, dst, sizeof (struct rip));
}

/*
 * Supply dst with the contents of the routing tables.
 * If this won't fit in one packet, chop it up into several.
 */
void
supply(dst, flags, ifp, rtstate)
	struct sockaddr *dst;
	int flags;
	struct interface *ifp;
	int rtstate;
{
	register struct rt_entry *rt;
	register struct netinfo *n = msg->rip_nets;
	register struct rthash *rh;
	struct rthash *base = hosthash;
	int doinghost = 1, size;
	void (*output) __P((int, int, struct sockaddr *, int)) =
		afswitch[dst->sa_family].af_output;
	int (*sendroute) __P((struct rt_entry *, struct sockaddr *)) =
		afswitch[dst->sa_family].af_sendroute;
	int npackets = 0;

	msg->rip_cmd = RIPCMD_RESPONSE;
	msg->rip_vers = RIP_VERSION_1;
	memset(msg->rip_res1, 0, sizeof(msg->rip_res1));
again:
	for (rh = base; rh < &base[ROUTEHASHSIZ]; rh++)
	for (rt = rh->cqh_first; rt != (void *) rh; rt = rt->rt_entry.cqe_next) {
		/*
		 * Don't resend the information on the network
		 * from which it was received (unless sending
		 * in response to a query).
		 */
		if (ifp && rt->rt_ifp == ifp &&
		    (rt->rt_state & RTS_INTERFACE) == 0)
			continue;
		if (rt->rt_state & RTS_EXTERNAL)
			continue;
		/*
		 * For dynamic updates, limit update to routes
		 * with the specified state.
		 */
		if (rtstate && (rt->rt_state & rtstate) == 0)
			continue;
		/*
		 * Limit the spread of subnet information
		 * to those who are interested.
		 */
		if (doinghost == 0 && rt->rt_state & RTS_SUBNET) {
			if (rt->rt_dst.sa_family != dst->sa_family)
				continue;
			if ((*sendroute)(rt, dst) == 0)
				continue;
		}
		size = (char *)n - packet;
		if (size > MAXPACKETSIZE - sizeof (struct netinfo)) {
			TRACE_OUTPUT(ifp, dst, size);
			(*output)(s, flags, dst, size);
			/*
			 * If only sending to ourselves,
			 * one packet is enough to monitor interface.
			 */
			if (ifp && (ifp->int_flags &
			   (IFF_BROADCAST | IFF_POINTOPOINT | IFF_REMOTE)) == 0)
				return;
			n = msg->rip_nets;
			npackets++;
		}
		(*afswitch[rt->rt_dst.sa_family].af_put)(n, &rt->rt_dst);
		n->rip_metric = htonl(rt->rt_metric);
		n++;
	}
	if (doinghost) {
		doinghost = 0;
		base = nethash;
		goto again;
	}
	if (n != msg->rip_nets || (npackets == 0 && rtstate == 0)) {
		size = (char *)n - packet;
		TRACE_OUTPUT(ifp, dst, size);
		(*output)(s, flags, dst, size);
	}
}
