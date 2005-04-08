/* $OpenBSD: src/sbin/isakmpd/sysdep/openbsd/sysdep.c,v 1.33 2005/04/08 23:15:26 hshoexer Exp $	 */
/* $EOM: sysdep.c,v 1.9 2000/12/04 04:46:35 angelos Exp $	 */

/*
 * Copyright (c) 1998, 1999 Niklas Hallqvist.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This code was written under funding by Ericsson Radio Systems.
 */

#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#include "sysdep.h"

#include "monitor.h"
#include "util.h"

#include "app.h"
#include "conf.h"
#include "ipsec.h"

#include "pf_key_v2.h"
#define KEY_API(x) pf_key_v2_##x

#include "log.h"

/*
 * When select(2) has noticed our application needs attendance, this is what
 * gets called.  FD is the file descriptor causing the alarm.
 */
void
sysdep_app_handler(int fd)
{
	KEY_API(handler)(fd);
}

/* Check that the connection named NAME is active, or else make it active.  */
void
sysdep_connection_check(char *name)
{
	KEY_API(connection_check)(name);
}

/*
 * Generate a SPI for protocol PROTO and the source/destination pair given by
 * SRC, SRCLEN, DST & DSTLEN.  Stash the SPI size in SZ.
 */
u_int8_t *
sysdep_ipsec_get_spi(size_t *sz, u_int8_t proto, struct sockaddr *src,
    struct sockaddr *dst, u_int32_t seq)
{
	if (app_none) {
		*sz = IPSEC_SPI_SIZE;
		/* XXX should be random instead I think.  */
		return (u_int8_t *)strdup("\x12\x34\x56\x78");
	}
	return KEY_API(get_spi)(sz, proto, src, dst, seq);
}

struct sa_kinfo *
sysdep_ipsec_get_kernel_sa(u_int8_t *spi, size_t spi_sz, u_int8_t proto,
    struct sockaddr *dst)
{
	if (app_none)
		return 0;
	return KEY_API(get_kernel_sa)(spi, spi_sz, proto, dst);
}

/* Force communication on socket FD to go in the clear.  */
int
sysdep_cleartext(int fd, int af)
{
	int level, sw;
	struct {
		int             ip_proto;	/* IP protocol */
		int             auth_level;
		int             esp_trans_level;
		int             esp_network_level;
		int             ipcomp_level;
	} optsw[] = {
	    {
		IPPROTO_IP,
		IP_AUTH_LEVEL,
		IP_ESP_TRANS_LEVEL,
		IP_ESP_NETWORK_LEVEL,
#ifdef IP_IPCOMP_LEVEL
		IP_IPCOMP_LEVEL
#else
		0
#endif
	    }, {
		IPPROTO_IPV6,
		IPV6_AUTH_LEVEL,
		IPV6_ESP_TRANS_LEVEL,
		IPV6_ESP_NETWORK_LEVEL,
#ifdef IPV6_IPCOMP_LEVEL
		IPV6_IPCOMP_LEVEL
#else
		0
#endif
	    },
	};

	if (app_none)
		return 0;

	switch (af) {
	case AF_INET:
		sw = 0;
		break;
	case AF_INET6:
		sw = 1;
		break;
	default:
		log_print("sysdep_cleartext: unsupported protocol family %d", af);
		return -1;
	}

	/*
         * Need to bypass system security policy, so I can send and
         * receive key management datagrams in the clear.
         */
	level = IPSEC_LEVEL_BYPASS;
	if (monitor_setsockopt(fd, optsw[sw].ip_proto, optsw[sw].auth_level,
	    (char *) &level, sizeof level) == -1) {
		log_error("sysdep_cleartext: "
		    "setsockopt (%d, %d, IP_AUTH_LEVEL, ...) failed", fd,
		    optsw[sw].ip_proto);
		return -1;
	}
	if (monitor_setsockopt(fd, optsw[sw].ip_proto, optsw[sw].esp_trans_level,
	    (char *) &level, sizeof level) == -1) {
		log_error("sysdep_cleartext: "
		    "setsockopt (%d, %d, IP_ESP_TRANS_LEVEL, ...) failed", fd,
		    optsw[sw].ip_proto);
		return -1;
	}
	if (monitor_setsockopt(fd, optsw[sw].ip_proto, optsw[sw].esp_network_level,
	    (char *) &level, sizeof level) == -1) {
		log_error("sysdep_cleartext: "
		    "setsockopt (%d, %d, IP_ESP_NETWORK_LEVEL, ...) failed", fd,
		    optsw[sw].ip_proto);
		return -1;
	}
	if (optsw[sw].ipcomp_level &&
	    monitor_setsockopt(fd, optsw[sw].ip_proto, optsw[sw].ipcomp_level,
	    (char *) &level, sizeof level) == -1 &&
	    errno != ENOPROTOOPT) {
		log_error("sysdep_cleartext: "
		    "setsockopt (%d, %d, IP_IPCOMP_LEVEL, ...) failed,", fd,
		    optsw[sw].ip_proto);
		return -1;
	}
	return 0;
}

int
sysdep_ipsec_delete_spi(struct sa *sa, struct proto *proto, int incoming)
{
	if (app_none)
		return 0;
	return KEY_API(delete_spi)(sa, proto, incoming);
}

int
sysdep_ipsec_enable_sa(struct sa *sa, struct sa *isakmp_sa)
{
	if (app_none)
		return 0;
	return KEY_API(enable_sa)(sa, isakmp_sa);
}

int
sysdep_ipsec_group_spis(struct sa *sa, struct proto *proto1,
    struct proto *proto2, int incoming)
{
	if (app_none)
		return 0;
	return KEY_API(group_spis)(sa, proto1, proto2, incoming);
}

int
sysdep_ipsec_set_spi(struct sa *sa, struct proto *proto, int incoming,
    struct sa *isakmp_sa)
{
	if (app_none)
		return 0;
	return KEY_API(set_spi) (sa,proto, incoming, isakmp_sa);
}
