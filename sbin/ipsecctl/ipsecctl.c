/*	$OpenBSD: src/sbin/ipsecctl/ipsecctl.c,v 1.51 2006/06/01 15:47:26 hshoexer Exp $	*/
/*
 * Copyright (c) 2004, 2005 Hans-Joerg Hoexer <hshoexer@openbsd.org>
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
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <net/pfkeyv2.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/ip_ipsp.h>
#include <arpa/inet.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include "ipsecctl.h"
#include "pfkey.h"

int		 ipsecctl_rules(char *, int);
FILE		*ipsecctl_fopen(const char *, const char *);
int		 ipsecctl_commit(int, struct ipsecctl *);
int		 ipsecctl_add_rule(struct ipsecctl *, struct ipsec_rule *);
void		 ipsecctl_free_rule(struct ipsec_rule *);
void		 ipsecctl_print_addr(struct ipsec_addr_wrap *);
void		 ipsecctl_print_proto(u_int8_t);
void		 ipsecctl_print_key(struct ipsec_key *);
void		 ipsecctl_print_flow(struct ipsec_rule *, int);
void		 ipsecctl_print_sa(struct ipsec_rule *, int);
void		 ipsecctl_print_rule(struct ipsec_rule *, int);
int		 ipsecctl_flush(int);
void		 ipsecctl_get_rules(struct ipsecctl *);
void		 ipsecctl_print_title(char *);
void		 ipsecctl_show_flows(int);
void		 ipsecctl_show_sas(int);
int		 ipsecctl_monitor(int);
void		 usage(void);
const char	*ipsecctl_lookup_option(char *, const char **);
static int	 unmask(struct ipsec_addr *, sa_family_t);

const char	*infile;	/* Used by parse.y */
const char	*showopt;

int		 first_title = 1;

static const char *showopt_list[] = {
	"flow", "sa", "all", NULL
};

static const char *direction[] = {"?", "in", "out"};
static const char *flowtype[] = {"?", "use", "acquire", "require", "deny",
    "bypass", "dontacq"};
static const char *satype[] = {"?", "esp", "ah", "ipcomp", "tcpmd5", "ipip"};
static const char *tmode[] = {"?", "transport", "tunnel"};
static const char *auth[] = {"?", "psk", "rsa"};

int
ipsecctl_rules(char *filename, int opts)
{
	FILE		*fin;
	struct ipsecctl	 ipsec;
	int		 action, error = 0;

	bzero(&ipsec, sizeof(ipsec));
	ipsec.opts = opts;
	TAILQ_INIT(&ipsec.rule_queue);
	TAILQ_INIT(&ipsec.group_queue);

	if (strcmp(filename, "-") == 0) {
		fin = stdin;
		infile = "stdin";
	} else {
		if ((fin = ipsecctl_fopen(filename, "r")) == NULL) {
			warn("%s", filename);
			return (1);
		}
		infile = filename;
	}

	if (parse_rules(fin, &ipsec) < 0) {
		warnx("Syntax error in config file: ipsec rules not loaded");
		error = 1;
	} else {
		if (opts & IPSECCTL_OPT_DELETE)
			action = ACTION_DELETE;
		else
			action = ACTION_ADD;

		if ((opts & IPSECCTL_OPT_NOACTION) == 0)
			if (ipsecctl_commit(action, &ipsec))
				err(1, NULL);
	}

	if (fin != stdin) {
		fclose(fin);
		fin = NULL;
	}
	return (error);
}

FILE *
ipsecctl_fopen(const char *name, const char *mode)
{
	struct stat	 st;
	FILE		*fp;

	fp = fopen(name, mode);
	if (fp == NULL)
		return (NULL);

	if (fstat(fileno(fp), &st)) {
		fclose(fp);
		return (NULL);
	}
	if (S_ISDIR(st.st_mode)) {
		fclose(fp);
		errno = EISDIR;
		return (NULL);
	}
	return (fp);
}

int
ipsecctl_commit(int action, struct ipsecctl *ipsec)
{
	struct ipsec_rule *rp;

	if (pfkey_init() == -1)
		errx(1, "ipsecctl_commit: failed to open PF_KEY socket");

	while ((rp = TAILQ_FIRST(&ipsec->rule_queue))) {
		TAILQ_REMOVE(&ipsec->rule_queue, rp, rule_entry);

		if (rp->type & RULE_IKE) {
			if (ike_ipsec_establish(action, rp) == -1)
				warnx("failed to %s rule %d",
				    action == ACTION_DELETE ? "delete" : "add",
				    rp->nr);
		} else {
			if (pfkey_ipsec_establish(action, rp) == -1)
				warnx("failed to %s rule %d",
				    action == ACTION_DELETE ? "delete" : "add",
				    rp->nr);
		}
		ipsecctl_free_rule(rp);
	}

	return (0);
}

int
ipsecctl_add_rule(struct ipsecctl *ipsec, struct ipsec_rule *r)
{
	TAILQ_INSERT_TAIL(&ipsec->rule_queue, r, rule_entry);

	if ((ipsec->opts & IPSECCTL_OPT_VERBOSE) && !(ipsec->opts &
	    IPSECCTL_OPT_SHOW))
		ipsecctl_print_rule(r, ipsec->opts);

	return (0);
}

void
ipsecctl_free_rule(struct ipsec_rule *rp)
{
	/* src and dst are always used. */
	free(rp->src->name);
	free(rp->src);
	free(rp->dst->name);
	free(rp->dst);

	if (rp->local) {
		free(rp->local->name);
		free(rp->local);
	}
	if (rp->peer) {
		free(rp->peer->name);
		free(rp->peer);
	}
	if (rp->auth) {
		if (rp->auth->srcid)
			free(rp->auth->srcid);
		if (rp->auth->dstid)
			free(rp->auth->dstid);
		free(rp->auth);
	}
	if (rp->ikeauth) {
		if (rp->ikeauth->string)
			free(rp->ikeauth->string);
		free(rp->ikeauth);
	}
	if (rp->xfs)
		free(rp->xfs);
	if (rp->authkey) {
		free(rp->authkey->data);
		free(rp->authkey);
	}
	if (rp->enckey) {
		free(rp->enckey->data);
		free(rp->enckey);
	}
	free(rp);
}

void
ipsecctl_print_addr(struct ipsec_addr_wrap *ipa)
{
	int		bits;
	char		buf[NI_MAXHOST];

	if (ipa == NULL) {
		printf("?");
		return;
	}
	if (inet_ntop(ipa->af, &ipa->address, buf, sizeof(buf)) == NULL)
		printf("?");
	else
		printf("%s", buf);

	bits = unmask(&ipa->mask, ipa->af);
	if (bits != (ipa->af == AF_INET ? 32 : 128))
		printf("/%d", bits);
}

void
ipsecctl_print_proto(u_int8_t proto)
{
	struct protoent *p;

	if ((p = getprotobynumber(proto)) != NULL)
		printf("%s", p->p_name);
	else
		printf("%u", proto);
}

void
ipsecctl_print_key(struct ipsec_key *key)
{
	int	i;

	for (i = 0; i < (int)key->len; i++)
		printf("%02x", key->data[i]);
}

void
ipsecctl_print_flow(struct ipsec_rule *r, int opts)
{
	printf("flow %s %s", satype[r->satype], direction[r->direction]);

	if (r->proto) {
		printf(" proto ");
		ipsecctl_print_proto(r->proto);
	}
	printf(" from ");
	ipsecctl_print_addr(r->src);
	printf(" to ");
	ipsecctl_print_addr(r->dst);
	if (r->local) {
		printf(" local ");
		ipsecctl_print_addr(r->local);
	}
	if (r->peer) {
		printf(" peer ");
		ipsecctl_print_addr(r->peer);
	}
	if (r->auth) {
		if (r->auth->srcid)
			printf(" srcid %s", r->auth->srcid);
		if (r->auth->dstid)
			printf(" dstid %s", r->auth->dstid);
		if (r->auth->type > 0)
			printf(" %s", auth[r->auth->type]);
	}
	printf(" type %s", flowtype[r->flowtype]);
	printf("\n");
}

/* ARGSUSED1 */
void
ipsecctl_print_sa(struct ipsec_rule *r, int opts)
{
	printf("%s ", satype[r->satype]);
	/* tunnel/transport is only meaningful esp/ah/ipcomp */
	if (r->satype != IPSEC_TCPMD5 && r->satype != IPSEC_IPIP)
		printf("%s ", tmode[r->tmode]);
	printf("from ");
	ipsecctl_print_addr(r->src);
	printf(" to ");
	ipsecctl_print_addr(r->dst);
	printf(" spi 0x%08x", r->spi);

	if (r->satype != IPSEC_TCPMD5) {
		if (r->xfs && r->xfs->authxf)
			printf(" auth %s", r->xfs->authxf->name);
		if (r->xfs && r->xfs->encxf)
			printf(" enc %s", r->xfs->encxf->name);
		if (r->xfs && r->xfs->compxf)
			printf(" comp %s", r->xfs->compxf->name);
	}
	if (r->authkey) {
		if (r->satype == IPSEC_TCPMD5)
			printf(" ");
		else
			printf("\n\t");
		printf("authkey 0x");
		ipsecctl_print_key(r->authkey);
	}
	if (r->enckey) {
		if (r->satype == IPSEC_TCPMD5)
			printf(" ");
		else
			printf("\n\t");
		printf("enckey 0x");
		ipsecctl_print_key(r->enckey);
	}
	printf("\n");
}

void
ipsecctl_print_rule(struct ipsec_rule *r, int opts)
{
	if (opts & IPSECCTL_OPT_VERBOSE2)
		printf("@%d ", r->nr);

	if (r->type & RULE_FLOW)
		ipsecctl_print_flow(r, opts);
	if (r->type & RULE_SA)
		ipsecctl_print_sa(r, opts);
	if (r->type & RULE_IKE)
		ike_print_config(r, opts);
}

int
ipsecctl_flush(int opts)
{
	if (opts & IPSECCTL_OPT_NOACTION)
		return (0);

	if (pfkey_init() == -1)
		errx(1, "ipsecctl_flush: failed to open PF_KEY socket");

	if (pfkey_ipsec_flush() == -1)
		errx(1, "ipsecctl_flush: failed to flush");

	return (0);
}

void
ipsecctl_get_rules(struct ipsecctl *ipsec)
{
	struct sadb_msg *msg;
	struct ipsec_rule *rule;
	int		 mib[4];
	size_t		 need;
	char		*buf, *lim, *next;

	mib[0] = CTL_NET;
	mib[1] = PF_KEY;
	mib[2] = PF_KEY_V2;
	mib[3] = NET_KEY_SPD_DUMP;

	if (sysctl(mib, 4, NULL, &need, NULL, 0) == -1)
		err(1, "ipsecctl_get_rules: sysctl");
	if (need == 0)
		return;
	if ((buf = malloc(need)) == NULL)
		err(1, "ipsecctl_get_rules: malloc");
	if (sysctl(mib, 4, buf, &need, NULL, 0) == -1)
		err(1, "ipsecctl_get_rules: sysctl");
	lim = buf + need;

	for (next = buf; next < lim; next += msg->sadb_msg_len *
	    PFKEYV2_CHUNK) {
		msg = (struct sadb_msg *)next;
		if (msg->sadb_msg_len == 0)
			break;

		rule = calloc(1, sizeof(struct ipsec_rule));
		if (rule == NULL)
			err(1, "ipsecctl_get_rules: calloc");
		rule->nr = ipsec->rule_nr++;
		rule->type |= RULE_FLOW;

		if (pfkey_parse(msg, rule))
			errx(1, "ipsecctl_get_rules: "
			    "failed to parse PF_KEY message");

		ipsecctl_add_rule(ipsec, rule);
	}

	free(buf);
}

void
ipsecctl_print_title(char *title)
{
	if (!first_title)
		printf("\n");
	first_title = 0;
	printf("%s\n", title);
}

void
ipsecctl_show_flows(int opts)
{
	struct ipsecctl ipsec;
	struct ipsec_rule *rp;

	bzero(&ipsec, sizeof(ipsec));
	ipsec.opts = opts;
	TAILQ_INIT(&ipsec.rule_queue);

	ipsecctl_get_rules(&ipsec);

	if (opts & IPSECCTL_OPT_SHOWALL)
		ipsecctl_print_title("FLOWS:");

	if (TAILQ_FIRST(&ipsec.rule_queue) == 0) {
		if (opts & IPSECCTL_OPT_SHOWALL)
			printf("No flows\n");
		return;
	}

	while ((rp = TAILQ_FIRST(&ipsec.rule_queue))) {
		TAILQ_REMOVE(&ipsec.rule_queue, rp, rule_entry);

		ipsecctl_print_rule(rp, ipsec.opts);

		free(rp->src->name);
		free(rp->src);
		free(rp->dst->name);
		free(rp->dst);
		if (rp->local) {
			free(rp->local->name);
			free(rp->local);
		}
		if (rp->peer) {
			free(rp->peer->name);
			free(rp->peer);
		}
		if (rp->auth) {
			if (rp->auth->srcid)
				free(rp->auth->srcid);
			if (rp->auth->dstid)
				free(rp->auth->dstid);
			free(rp->auth);
		}
		free(rp);
	}
}

void
ipsecctl_show_sas(int opts)
{
	struct sadb_msg *msg;
	int		 mib[5];
	size_t		 need = 0;
	char		*buf, *lim, *next;

	mib[0] = CTL_NET;
	mib[1] = PF_KEY;
	mib[2] = PF_KEY_V2;
	mib[3] = NET_KEY_SADB_DUMP;
	mib[4] = SADB_SATYPE_UNSPEC;

	if (opts & IPSECCTL_OPT_SHOWALL)
		ipsecctl_print_title("SADB:");

	/* When the SADB is empty we get ENOENT, no need to err(). */
	if (sysctl(mib, 5, NULL, &need, NULL, 0) == -1 && errno != ENOENT)
		err(1, "ipsecctl_show_sas: sysctl");
	if (need == 0) {
		if (opts & IPSECCTL_OPT_SHOWALL)
			printf("No entries\n");
		return;
	}
	if ((buf = malloc(need)) == NULL)
		err(1, "ipsecctl_show_sas: malloc");
	if (sysctl(mib, 5, buf, &need, NULL, 0) == -1)
		err(1, "ipsecctl_show_sas: sysctl");
	lim = buf + need;
	for (next = buf; next < lim;
	    next += msg->sadb_msg_len * PFKEYV2_CHUNK) {
		msg = (struct sadb_msg *)next;
		if (msg->sadb_msg_len == 0)
			break;
		pfkey_print_sa(msg, opts);
	}

	free(buf);
}

int
ipsecctl_monitor(int opts)
{
	return (pfkey_monitor(opts));
}

__dead void
usage(void)
{
	extern char	*__progname;

	fprintf(stderr, "usage: %s [-dFnv] [-D macro=value] [-f file]"
	    " [-s modifier]\n", __progname);
	exit(1);
}

const char *
ipsecctl_lookup_option(char *cmd, const char **list)
{
	if (cmd != NULL && *cmd)
		for (; *list; list++)
			if (!strncmp(cmd, *list, strlen(cmd)))
				return (*list);
	return (NULL);
}

int
main(int argc, char *argv[])
{
	int		 error = 0;
	int		 ch;
	int		 opts = 0;
	char		*rulesopt = NULL;

	if (argc < 2)
		usage();

	while ((ch = getopt(argc, argv, "D:df:Fmnvs:")) != -1) {
		switch (ch) {
		case 'D':
			if (cmdline_symset(optarg) < 0)
				warnx("could not parse macro definition %s",
				    optarg);
			break;
		case 'd':
			opts |= IPSECCTL_OPT_DELETE;
			break;
		case 'f':
			rulesopt = optarg;
			break;

		case 'F':
			opts |= IPSECCTL_OPT_FLUSH;
			break;

		case 'm':
			opts |= IPSECCTL_OPT_MONITOR;
			break;

		case 'n':
			opts |= IPSECCTL_OPT_NOACTION;
			break;

		case 'v':
			if (opts & IPSECCTL_OPT_VERBOSE)
				opts |= IPSECCTL_OPT_VERBOSE2;
			opts |= IPSECCTL_OPT_VERBOSE;
			break;

		case 's':
			showopt = ipsecctl_lookup_option(optarg, showopt_list);
			if (showopt == NULL) {
				warnx("Unknown show modifier '%s'", optarg);
				usage();
				/* NOTREACHED */
			}
			opts |= IPSECCTL_OPT_SHOW;
			break;

		default:
			usage();
			/* NOTREACHED */
		}
	}

	if (argc != optind) {
		warnx("unknown command line argument: %s ...", argv[optind]);
		usage();
		/* NOTREACHED */
	}
	if (opts & IPSECCTL_OPT_FLUSH)
		if (ipsecctl_flush(opts))
			error = 1;

	if (rulesopt != NULL)
		if (ipsecctl_rules(rulesopt, opts))
			error = 1;

	if (showopt != NULL) {
		switch (*showopt) {
		case 'f':
			ipsecctl_show_flows(opts);
			break;
		case 's':
			ipsecctl_show_sas(opts);
			break;
		case 'a':
			opts |= IPSECCTL_OPT_SHOWALL;
			ipsecctl_show_flows(opts);
			ipsecctl_show_sas(opts);
		}
	}

	if (opts & IPSECCTL_OPT_MONITOR)
		if (ipsecctl_monitor(opts))
			error = 1;

	exit(error);
}

/* ARGSUSED1 */
static int
unmask(struct ipsec_addr *ipa, sa_family_t af)
{
	int		i = 31, j = 0, b = 0;
	u_int32_t	tmp;

	while (j < 4 && ipa->addr32[j] == 0xffffffff) {
		b += 32;
		j++;
	}
	if (j < 4) {
		tmp = ntohl(ipa->addr32[j]);
		for (i = 31; tmp & (1 << i); --i)
			b++;
	}
	return (b);
}
