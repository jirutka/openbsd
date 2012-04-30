/* $OpenBSD: src/usr.sbin/ypldap/ldapclient.c,v 1.25 2012/04/30 11:28:25 jmatthew Exp $ */

/*
 * Copyright (c) 2008 Alexander Schrijver <aschrijver@openbsd.org>
 * Copyright (c) 2008 Pierre-Yves Ritschard <pyr@openbsd.org>
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

#include <sys/types.h>
#include <sys/param.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/tree.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <errno.h>
#include <err.h>
#include <event.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aldap.h"
#include "ypldap.h"

void    client_sig_handler(int, short, void *);
void	client_dispatch_dns(int, short, void *);
void    client_dispatch_parent(int, short, void *);
void    client_shutdown(void);
void    client_connect(int, short, void *);
void    client_configure(struct env *);
void    client_periodic_update(int, short, void *);
int	client_build_req(struct idm *, struct idm_req *, struct aldap_message *,
	    int, int);
int	client_search_idm(struct env *, struct idm *, struct aldap *,
	    char **, char *, int, int, enum imsg_type);
int	client_try_idm(struct env *, struct idm *);
int	client_addr_init(struct idm *);
int	client_addr_free(struct idm *);

struct aldap	*client_aldap_open(struct ypldap_addr *);

/*
 * dummy wrapper to provide aldap_init with its fd's.
 */
struct aldap *
client_aldap_open(struct ypldap_addr *addr)
{
	int			 fd = -1;
	struct ypldap_addr	 *p;

	for (p = addr; p != NULL; p = p->next) {
		char			 hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
		struct sockaddr		*sa = (struct sockaddr *)&p->ss;

		if (getnameinfo(sa, SA_LEN(sa), hbuf, sizeof(hbuf), sbuf,
			sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV))
				errx(1, "could not get numeric hostname");

		if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			return NULL;

		if (connect(fd, sa, SA_LEN(sa)) == 0)
			break;

		warn("connect to %s port %s (%s) failed", hbuf, sbuf, "tcp");
		close(fd);
	}

	if (fd == -1)
		return NULL;

	return aldap_init(fd);
}

int
client_addr_init(struct idm *idm)
{
        struct sockaddr_in      *sa_in;
        struct sockaddr_in6     *sa_in6;
        struct ypldap_addr         *h;

        for (h = idm->idm_addr; h != NULL; h = h->next) {
                switch (h->ss.ss_family) {
                case AF_INET:
                        sa_in = (struct sockaddr_in *)&h->ss;
                        if (ntohs(sa_in->sin_port) == 0)
                                sa_in->sin_port = htons(LDAP_PORT);
                        idm->idm_state = STATE_DNS_DONE;
                        break;
                case AF_INET6:
                        sa_in6 = (struct sockaddr_in6 *)&h->ss;
                        if (ntohs(sa_in6->sin6_port) == 0)
                                sa_in6->sin6_port = htons(LDAP_PORT);
                        idm->idm_state = STATE_DNS_DONE;
                        break;
                default:
                        fatalx("king bula sez: wrong AF in client_addr_init");
                        /* not reached */
                }
        }

        return (0);
}

int
client_addr_free(struct idm *idm)
{
        struct ypldap_addr         *h, *p;

	if (idm->idm_addr == NULL)
		return (-1);

	for (h = idm->idm_addr; h != NULL; h = p) {
		p = h->next;
		free(h);
	}

	idm->idm_addr = NULL;

	return (0);
}

void
client_sig_handler(int sig, short event, void *p)
{
	switch (sig) {
	case SIGINT:
	case SIGTERM:
		client_shutdown();
		break;
	default:
		fatalx("unexpected signal");
	}
}

void
client_dispatch_dns(int fd, short event, void *p)
{
	struct imsg		 imsg;
	u_int16_t		 dlen;
	u_char			*data;
	struct ypldap_addr	*h;
	int			 n, wait_cnt = 0;
	struct idm		*idm;
	int			 shut = 0;

	struct env		*env = p;
	struct imsgev		*iev = env->sc_iev_dns;
	struct imsgbuf		*ibuf = &iev->ibuf;

	switch (event) {
	case EV_READ:
		if ((n = imsg_read(ibuf)) == -1)
			fatal("imsg_read error");
		if (n == 0)
			shut = 1;
		break;
	case EV_WRITE:
		if (msgbuf_write(&ibuf->w) == -1)
			fatal("msgbuf_write");
		imsg_event_add(iev);
		return;
	default:
		fatalx("unknown event");
	}

	for (;;) {
		if ((n = imsg_get(ibuf, &imsg)) == -1)
			fatal("client_dispatch_dns: imsg_get error");
		if (n == 0)
			break;

		switch (imsg.hdr.type) {
		case IMSG_HOST_DNS:
			TAILQ_FOREACH(idm, &env->sc_idms, idm_entry)
				if (idm->idm_id == imsg.hdr.peerid)
					break;
			if (idm == NULL) {
				log_warnx("IMSG_HOST_DNS with invalid peerID");
				break;
			}
			if (idm->idm_addr != NULL) {
				log_warnx("IMSG_HOST_DNS but addr != NULL!");
				break;
			}

			dlen = imsg.hdr.len - IMSG_HEADER_SIZE;
			if (dlen == 0) {	/* no data -> temp error */
				idm->idm_state = STATE_DNS_TEMPFAIL;
				break;
			}

			data = (u_char *)imsg.data;
			while (dlen >= sizeof(struct sockaddr_storage)) {
				if ((h = calloc(1, sizeof(struct ypldap_addr))) ==
				    NULL)
					fatal(NULL);
				memcpy(&h->ss, data, sizeof(h->ss));

				if (idm->idm_addr == NULL)
					h->next = NULL;
				else
					h->next = idm->idm_addr;

				idm->idm_addr = h;

				data += sizeof(h->ss);
				dlen -= sizeof(h->ss);
			}
			if (dlen != 0)
				fatalx("IMSG_HOST_DNS: dlen != 0");

			client_addr_init(idm);

			break;
		default:
			break;
		}
		imsg_free(&imsg);
	}

	TAILQ_FOREACH(idm, &env->sc_idms, idm_entry) {
		if (client_try_idm(env, idm) == -1)
			idm->idm_state = STATE_LDAP_FAIL;

		if (idm->idm_state < STATE_LDAP_DONE)
			wait_cnt++;
	}
	if (wait_cnt == 0)
		imsg_compose_event(env->sc_iev, IMSG_END_UPDATE, 0, 0, -1,
		    NULL, 0);

	if (!shut)
		imsg_event_add(iev);
	else {
		/* this pipe is dead, so remove the event handler */
		event_del(&iev->ev);
		event_loopexit(NULL);
	}
}

void
client_dispatch_parent(int fd, short event, void *p)
{
	int			 n;
	int			 shut = 0;
	struct imsg		 imsg;
	struct env		*env = p;
	struct imsgev		*iev = env->sc_iev;
	struct imsgbuf		*ibuf = &iev->ibuf;


	switch (event) {
	case EV_READ:
		if ((n = imsg_read(ibuf)) == -1)
			fatal("imsg_read error");
		if (n == 0)
			shut = 1;
		break;
	case EV_WRITE:
		if (msgbuf_write(&ibuf->w) == -1)
			fatal("msgbuf_write");
		imsg_event_add(iev);
		return;
	default:
		fatalx("unknown event");
	}

	for (;;) {
		if ((n = imsg_get(ibuf, &imsg)) == -1)
			fatal("client_dispatch_parent: imsg_get error");
		if (n == 0)
			break;

		switch (imsg.hdr.type) {
		case IMSG_CONF_START: {
			struct env	params;

			if (env->sc_flags & F_CONFIGURING) {
				log_warnx("configuration already in progress");
				break;
			}
			memcpy(&params, imsg.data, sizeof(params));
			log_debug("configuration starting");
			env->sc_flags |= F_CONFIGURING;
			purge_config(env);
			memcpy(&env->sc_conf_tv, &params.sc_conf_tv,
			    sizeof(env->sc_conf_tv));
			env->sc_flags |= params.sc_flags;
			break;
		}
		case IMSG_CONF_IDM: {
			struct idm	*idm;

			if (!(env->sc_flags & F_CONFIGURING))
				break;
			if ((idm = calloc(1, sizeof(*idm))) == NULL)
				fatal(NULL);
			memcpy(idm, imsg.data, sizeof(*idm));
			idm->idm_env = env;
			TAILQ_INSERT_TAIL(&env->sc_idms, idm, idm_entry);
			break;
		}
		case IMSG_CONF_END:
			env->sc_flags &= ~F_CONFIGURING;
			log_debug("applying configuration");
			client_configure(env);
			break;
		default:
			log_debug("client_dispatch_parent: unexpect imsg %d",
			    imsg.hdr.type);

			break;
		}
		imsg_free(&imsg);
	}
	if (!shut)
		imsg_event_add(iev);
	else {
		/* this pipe is dead, so remove the event handler */
		event_del(&iev->ev);
		event_loopexit(NULL);
	}
}

void
client_shutdown(void)
{
	log_info("ldap client exiting");
	_exit(0);
}

pid_t
ldapclient(int pipe_main2client[2])
{
	pid_t            pid, dns_pid;
	int              pipe_dns[2];
	struct passwd	*pw;
	struct event	 ev_sigint;
	struct event	 ev_sigterm;
	struct env	 env;

	switch (pid = fork()) {
	case -1:
		fatal("cannot fork");
		break;
	case 0:
		break;
	default:
		return (pid);
	}

	bzero(&env, sizeof(env));
	TAILQ_INIT(&env.sc_idms);

	if ((pw = getpwnam(YPLDAP_USER)) == NULL)
		fatal("getpwnam");

	if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, pipe_dns) == -1)
		fatal("socketpair");
	dns_pid = ypldap_dns(pipe_dns, pw);
	close(pipe_dns[1]);

#ifndef DEBUG
	if (chroot(pw->pw_dir) == -1)
		fatal("chroot");
	if (chdir("/") == -1)
		fatal("chdir");
#else
#warning disabling chrooting in DEBUG mode
#endif
	setproctitle("ldap client");
	ypldap_process = PROC_CLIENT;

#ifndef DEBUG
	if (setgroups(1, &pw->pw_gid) ||
	    setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) ||
	    setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid))
		fatal("cannot drop privileges");
#else
#warning disabling privilege revocation in DEBUG mode
#endif

	event_init();
	signal_set(&ev_sigint, SIGINT, client_sig_handler, NULL);
	signal_set(&ev_sigterm, SIGTERM, client_sig_handler, NULL);
	signal_add(&ev_sigint, NULL);
	signal_add(&ev_sigterm, NULL);

	close(pipe_main2client[0]);
	if ((env.sc_iev = calloc(1, sizeof(*env.sc_iev))) == NULL)
		fatal(NULL);
	if ((env.sc_iev_dns = calloc(1, sizeof(*env.sc_iev_dns))) == NULL)
		fatal(NULL);

	env.sc_iev->events = EV_READ;
	env.sc_iev->data = &env;
	imsg_init(&env.sc_iev->ibuf, pipe_main2client[1]);
	env.sc_iev->handler = client_dispatch_parent;
	event_set(&env.sc_iev->ev, env.sc_iev->ibuf.fd, env.sc_iev->events,
	    env.sc_iev->handler, &env);
	event_add(&env.sc_iev->ev, NULL);

	env.sc_iev_dns->events = EV_READ;
	env.sc_iev_dns->data = &env;
	imsg_init(&env.sc_iev_dns->ibuf, pipe_dns[0]);
	env.sc_iev_dns->handler = client_dispatch_dns;
	event_set(&env.sc_iev_dns->ev, env.sc_iev_dns->ibuf.fd,
	    env.sc_iev_dns->events, env.sc_iev_dns->handler, &env);
	event_add(&env.sc_iev_dns->ev, NULL);

	event_dispatch();
	client_shutdown();

	return (0);

}

int
client_build_req(struct idm *idm, struct idm_req *ir, struct aldap_message *m,
    int min_attr, int max_attr)
{
	char	**ldap_attrs;
	int	 i, k;

	bzero(ir, sizeof(*ir));
	for (i = min_attr; i < max_attr; i++) {
		if (idm->idm_flags & F_FIXED_ATTR(i)) {
			if (strlcat(ir->ir_line, idm->idm_attrs[i],
			    sizeof(ir->ir_line)) >= sizeof(ir->ir_line))
				/*
				 * entry yields a line > 1024, trash it.
				 */
				return (-1);

			if (i == ATTR_UID) {
				ir->ir_key.ik_uid = strtonum(
				    idm->idm_attrs[i], 0,
				    UID_MAX, NULL);
			} else if (i == ATTR_GR_GID) {
				ir->ir_key.ik_gid = strtonum(
				    idm->idm_attrs[i], 0,
				    GID_MAX, NULL);
			}
		} else if (idm->idm_list & F_LIST(i)) {
			aldap_match_attr(m, idm->idm_attrs[i], &ldap_attrs);
			for (k = 0; k >= 0 && ldap_attrs && ldap_attrs[k] != NULL; k++) {
				/* XXX: Fail when attributes have illegal characters e.g. ',' */
				if (strlcat(ir->ir_line, ldap_attrs[k],
				    sizeof(ir->ir_line)) >= sizeof(ir->ir_line))
					continue;
				if (ldap_attrs[k+1] != NULL)
					if (strlcat(ir->ir_line, ",",
						    sizeof(ir->ir_line))
					    >= sizeof(ir->ir_line)) {
						aldap_free_attr(ldap_attrs);
						return (-1);
					}
			}
			aldap_free_attr(ldap_attrs);
		} else {
			if (aldap_match_attr(m, idm->idm_attrs[i], &ldap_attrs) == -1)
				return (-1);
			if (ldap_attrs[0] == NULL)
				return (-1);
			if (strlcat(ir->ir_line, ldap_attrs[0],
			    sizeof(ir->ir_line)) >= sizeof(ir->ir_line)) {
				aldap_free_attr(ldap_attrs);
				return (-1);
			}
			if (i == ATTR_UID) {
				ir->ir_key.ik_uid = strtonum(
				    ldap_attrs[0], 0, UID_MAX, NULL);
			} else if (i == ATTR_GR_GID) {
				ir->ir_key.ik_uid = strtonum(
				    ldap_attrs[0], 0, GID_MAX, NULL);
			}
			aldap_free_attr(ldap_attrs);
		}

		if (i + 1 != max_attr)
			if (strlcat(ir->ir_line, ":",
			    sizeof(ir->ir_line)) >= sizeof(ir->ir_line))
				return (-1);
	}

	return (0);
}

int
client_search_idm(struct env *env, struct idm *idm, struct aldap *al,
    char **attrs, char *filter, int min_attr, int max_attr,
    enum imsg_type type)
{
	struct idm_req		 ir;
	struct aldap_message	*m;
	const char		*errstr;
	char			*dn;

	dn = idm->idm_basedn;
	if (type == IMSG_GRP_ENTRY && idm->idm_groupdn[0] != '\0')
		dn = idm->idm_groupdn;

	if (aldap_search(al, dn, LDAP_SCOPE_SUBTREE,
		    filter, attrs, 0, 0, 0) == -1) {
		aldap_get_errno(al, &errstr);
		log_debug("%s", errstr);
		return (-1);
	}

	while ((m = aldap_parse(al)) != NULL) {
		if (al->msgid != m->msgid) {
			aldap_freemsg(m);
			return (-1);
		}
		/* end of the search result chain */
		if (m->message_type == LDAP_RES_SEARCH_RESULT) {
			aldap_freemsg(m);
			break;
		}
		/* search entry; the rest we won't handle */
		if (m->message_type != LDAP_RES_SEARCH_ENTRY) {
			aldap_freemsg(m);
			return (-1);
		}

		if (client_build_req(idm, &ir, m, min_attr, max_attr) == 0)
			imsg_compose_event(env->sc_iev, type, 0, 0, -1,
			    &ir, sizeof(ir));
		aldap_freemsg(m);
	}

	return (0);
}

int
client_try_idm(struct env *env, struct idm *idm)
{
	const char		*where;
	char			*attrs[ATTR_MAX+1];
	int			 i, j;
	struct aldap_message	*m;
	struct aldap		*al;

	where = "connect";
	if ((al = client_aldap_open(idm->idm_addr)) == NULL)
		return (-1);

	if (idm->idm_flags & F_NEEDAUTH) {
		where = "binding";
		if (aldap_bind(al, idm->idm_binddn, idm->idm_bindcred) == -1)
			goto bad;

		where = "parsing";
		if ((m = aldap_parse(al)) == NULL)
			goto bad;
		where = "verifying msgid";
		if (al->msgid != m->msgid) {
			aldap_freemsg(m);
			goto bad;
		}
		aldap_freemsg(m);
	}

	bzero(attrs, sizeof(attrs));
	for (i = 0, j = 0; i < ATTR_MAX; i++) {
		if (idm->idm_flags & F_FIXED_ATTR(i))
			continue;
		attrs[j++] = idm->idm_attrs[i];
	}
	attrs[j] = NULL;

	/*
	 * build password line.
	 */
	where = "search";
	log_debug("searching password entries");
	if (client_search_idm(env, idm, al, attrs,
	    idm->idm_filters[FILTER_USER], 0, ATTR_MAX, IMSG_PW_ENTRY) == -1)
		goto bad;

	bzero(attrs, sizeof(attrs));
	for (i = ATTR_GR_MIN, j = 0; i < ATTR_GR_MAX; i++) {
		if (idm->idm_flags & F_FIXED_ATTR(i))
			continue;
		attrs[j++] = idm->idm_attrs[i];
	}
	attrs[j] = NULL;

	/*
	 * build group line.
	 */
	where = "search";
	log_debug("searching group entries");
	if (client_search_idm(env, idm, al, attrs,
	    idm->idm_filters[FILTER_GROUP], ATTR_GR_MIN, ATTR_GR_MAX,
	    IMSG_GRP_ENTRY) == -1)
		goto bad;

	aldap_close(al);

	idm->idm_state = STATE_LDAP_DONE;

	return (0);
bad:
	aldap_close(al);
	log_debug("directory %s errored out in %s", idm->idm_name, where);
	return (-1);
}

void
client_periodic_update(int fd, short event, void *p)
{
	struct env	*env = p;

	struct idm	*idm;
	int		 fail_cnt = 0;

	/* If LDAP isn't finished, notify the master process to trash the
	 * update. */
	TAILQ_FOREACH(idm, &env->sc_idms, idm_entry) {
		if (idm->idm_state < STATE_LDAP_DONE)
			fail_cnt++;

		idm->idm_state = STATE_NONE;

		client_addr_free(idm);
	}
	if (fail_cnt > 0) {
		log_debug("trash the update");
		imsg_compose_event(env->sc_iev, IMSG_TRASH_UPDATE, 0, 0, -1,
		    NULL, 0);
	}

	client_configure(env);
}

void
client_configure(struct env *env)
{
	struct timeval	 tv;
	struct idm	*idm;
        u_int16_t        dlen;

	log_debug("connecting to directories");

	imsg_compose_event(env->sc_iev, IMSG_START_UPDATE, 0, 0, -1, NULL, 0);

	/* Start the DNS lookups */
	TAILQ_FOREACH(idm, &env->sc_idms, idm_entry) {
		dlen = strlen(idm->idm_name) + 1;
		imsg_compose_event(env->sc_iev_dns, IMSG_HOST_DNS, idm->idm_id,
		    0, -1, idm->idm_name, dlen);
	}

	tv.tv_sec = env->sc_conf_tv.tv_sec;
	tv.tv_usec = env->sc_conf_tv.tv_usec;
	evtimer_set(&env->sc_conf_ev, client_periodic_update, env);
	evtimer_add(&env->sc_conf_ev, &tv);
}
