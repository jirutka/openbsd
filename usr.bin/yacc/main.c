/*	$OpenBSD: src/usr.bin/yacc/main.c,v 1.13 2001/11/07 11:42:53 deraadt Exp $	*/
/*	$NetBSD: main.c,v 1.5 1996/03/19 03:21:38 jtc Exp $	*/

/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Robert Paul Corbett.
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
char copyright[] =
"@(#) Copyright (c) 1989 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)main.c	5.5 (Berkeley) 5/24/93";
#else
static char rcsid[] = "$OpenBSD: src/usr.bin/yacc/main.c,v 1.13 2001/11/07 11:42:53 deraadt Exp $";
#endif
#endif /* not lint */

#include <sys/types.h>
#include <fcntl.h>
#include <paths.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "defs.h"

char dflag;
char lflag;
char rflag;
char tflag;
char vflag;

char *symbol_prefix;
char *file_prefix = "y";
char *temp_form = "yacc.XXXXXXXXXXX";

int lineno;
int outline;

int explicit_file_name;

char *action_file_name;
char *code_file_name;
char *defines_file_name;
char *input_file_name = "";
char *output_file_name;
char *text_file_name;
char *union_file_name;
char *verbose_file_name;

FILE *action_file;	/*  a temp file, used to save actions associated    */
			/*  with rules until the parser is written	    */
FILE *code_file;	/*  y.code.c (used when the -r option is specified) */
FILE *defines_file;	/*  y.tab.h					    */
FILE *input_file;	/*  the input file				    */
FILE *output_file;	/*  y.tab.c					    */
FILE *text_file;	/*  a temp file, used to save text until all	    */
			/*  symbols have been defined			    */
FILE *union_file;	/*  a temp file, used to save the union		    */
			/*  definition until all symbol have been	    */
			/*  defined					    */
FILE *verbose_file;	/*  y.output					    */

int nitems;
int nrules;
int nsyms;
int ntokens;
int nvars;

int   start_symbol;
char  **symbol_name;
short *symbol_value;
short *symbol_prec;
char  *symbol_assoc;

short *ritem;
short *rlhs;
short *rrhs;
short *rprec;
char  *rassoc;
short **derives;
char *nullable;

void onintr __P((int));
void set_signals __P((void));
void usage __P((void));
void getargs __P((int, register char *[]));
void create_file_names __P((void));
void open_files __P((void));

volatile sig_atomic_t sigdie;

void
done(k)
int k;
{
    if (action_file)
	unlink(action_file_name);
    if (text_file)
	unlink(text_file_name);
    if (union_file)
	unlink(union_file_name);
    if (sigdie)
	_exit(k);
    exit(k);
}


void
onintr(signo)
	int signo;
{
    sigdie = 1;
    done(1);
}


void
set_signals()
{
#ifdef SIGINT
    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
	signal(SIGINT, onintr);
#endif
#ifdef SIGTERM
    if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
	signal(SIGTERM, onintr);
#endif
#ifdef SIGHUP
    if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
	signal(SIGHUP, onintr);
#endif
}


void
usage()
{
    fprintf(stderr, "usage: %s [-dlrtv] [-b file_prefix] [-o outputfile] [-p symbol_prefix] filename\n", __progname);
    exit(1);
}


void
getargs(argc, argv)
int argc;
char *argv[];
{
    register int i;
    register char *s;

    for (i = 1; i < argc; ++i)
    {
	s = argv[i];
	if (*s != '-') break;
	switch (*++s)
	{
	case '\0':
	    input_file = stdin;
	    if (i + 1 < argc) usage();
	    return;

	case '-':
	    ++i;
	    goto no_more_options;

	case 'b':
	    if (*++s)
		 file_prefix = s;
	    else if (++i < argc)
		file_prefix = argv[i];
	    else
		usage();
	    continue;

	case 'd':
	    dflag = 1;
	    break;

	case 'l':
	    lflag = 1;
	    break;

        case 'o':
            if (*++s)
	        output_file_name = s;
            else if (++i < argc)
                output_file_name = argv[i];
            else
                usage();
            explicit_file_name = 1;
            continue;

	case 'p':
	    if (*++s)
		symbol_prefix = s;
	    else if (++i < argc)
		symbol_prefix = argv[i];
	    else
		usage();
	    continue;

	case 'r':
	    rflag = 1;
	    break;

	case 't':
	    tflag = 1;
	    break;

	case 'v':
	    vflag = 1;
	    break;

	default:
	    usage();
	}

	for (;;)
	{
	    switch (*++s)
	    {
	    case '\0':
		goto end_of_option;

	    case 'd':
		dflag = 1;
		break;

	    case 'l':
		lflag = 1;
		break;

	    case 'r':
		rflag = 1;
		break;

	    case 't':
		tflag = 1;
		break;

	    case 'v':
		vflag = 1;
		break;

	    default:
		usage();
	    }
	}
end_of_option:;
    }

no_more_options:;
    if (i + 1 != argc) usage();
    input_file_name = argv[i];
}


char *
allocate(n)
unsigned n;
{
    register char *p;

    p = NULL;
    if (n)
    {
	p = CALLOC(1, n);
	if (!p) no_space();
    }
    return (p);
}

void
create_file_names()
{
    int i, len;
    char *tmpdir;

    if (!(tmpdir = getenv("TMPDIR")))
	tmpdir = _PATH_TMP;

    len = strlen(tmpdir);
    i = len + strlen(temp_form) + 1;
    if (len && tmpdir[len-1] != '/')
	++i;

    action_file_name = MALLOC(i);
    if (action_file_name == 0) no_space();
    text_file_name = MALLOC(i);
    if (text_file_name == 0) no_space();
    union_file_name = MALLOC(i);
    if (union_file_name == 0) no_space();

    strcpy(action_file_name, tmpdir);
    strcpy(text_file_name, tmpdir);
    strcpy(union_file_name, tmpdir);

    if (len && tmpdir[len - 1] != '/')
    {
	action_file_name[len] = '/';
	text_file_name[len] = '/';
	union_file_name[len] = '/';
	++len;
    }

    strcpy(action_file_name + len, temp_form);
    strcpy(text_file_name + len, temp_form);
    strcpy(union_file_name + len, temp_form);

    action_file_name[len + 5] = 'a';
    text_file_name[len + 5] = 't';
    union_file_name[len + 5] = 'u';

    len = strlen(file_prefix);

    if (!output_file_name)
    {
        output_file_name = MALLOC(len + 7);
        if (output_file_name == 0)
	    no_space();
        strcpy(output_file_name, file_prefix);
        strcpy(output_file_name + len, OUTPUT_SUFFIX);
    }

    if (rflag)
    {
	code_file_name = MALLOC(len + 8);
	if (code_file_name == 0)
	    no_space();
	strcpy(code_file_name, file_prefix);
	strcpy(code_file_name + len, CODE_SUFFIX);
    }
    else
	code_file_name = output_file_name;

    if (dflag)
    {
        if (explicit_file_name)
	{
	    char *suffix;

	    defines_file_name = MALLOC(strlen(output_file_name)+1);
	    if (defines_file_name == 0)
	        no_space();
	    strcpy(defines_file_name, output_file_name);

            /* does the output_file_name have a known suffix */
            if ((suffix = strrchr(output_file_name, '.')) != 0 &&
                (!strcmp(suffix, ".c") ||	/* good, old-fashioned C */
                 !strcmp(suffix, ".C") ||	/* C++, or C on Windows */
                 !strcmp(suffix, ".cc") ||	/* C++ */
                 !strcmp(suffix, ".cxx") ||	/* C++ */
                 !strcmp(suffix, ".cpp")))	/* C++ (Windows) */
            {
                strncpy(defines_file_name, output_file_name,
                    suffix - output_file_name + 1);
                defines_file_name[suffix - output_file_name + 1] = 'h';
                defines_file_name[suffix - output_file_name + 2] = '\0';
            } else {
                fprintf(stderr,"%s: suffix of output file name %s"
                    " not recognized, no -d file generated.\n",
                    __progname, output_file_name);
                dflag = 0;
                free(defines_file_name);
                defines_file_name = 0;
            }
	}
	else
	{
	    defines_file_name = MALLOC(len + 7);
	    if (defines_file_name == 0)
	        no_space();
	    strcpy(defines_file_name, file_prefix);
	    strcpy(defines_file_name + len, DEFINES_SUFFIX);
	}
    }

    if (vflag)
    {
	verbose_file_name = MALLOC(len + 8);
	if (verbose_file_name == 0)
	    no_space();
	strcpy(verbose_file_name, file_prefix);
	strcpy(verbose_file_name + len, VERBOSE_SUFFIX);
    }
}


FILE *
fsopen(name, mode)
    char *name;
    char *mode;
{
    FILE *fp = NULL;
    int fd, mod = O_RDONLY;

    if (strchr(mode, 'w'))
	mod = O_RDWR;
    if ((fd = open(name, mod | O_EXCL|O_CREAT, 0666)) == -1 ||
	(fp = fdopen(fd, mode)) == NULL) {
	if (fd != -1)
	    close(fd);
    }
    return (fp);
}

void
open_files()
{
    int fd;

    create_file_names();

    if (input_file == 0)
    {
	input_file = fopen(input_file_name, "r");
	if (input_file == 0)
	    open_error(input_file_name);
    }

    fd = mkstemp(action_file_name);
    if (fd == -1 || (action_file = fdopen(fd, "w")) == NULL)
	open_error(action_file_name);

    fd = mkstemp(text_file_name);
    if (fd == -1 || (text_file = fdopen(fd, "w")) == NULL)
	open_error(text_file_name);

    if (vflag)
    {
	verbose_file = fopen(verbose_file_name, "w");
	if (verbose_file == 0)
	    open_error(verbose_file_name);
    }

    if (dflag)
    {
	defines_file = fopen(defines_file_name, "w");
	if (defines_file == 0)
	    open_error(defines_file_name);
	fd = mkstemp(union_file_name);
	if (fd == -1 || (union_file = fdopen(fd, "w")) == NULL)
	    open_error(union_file_name);
    }

    output_file = fopen(output_file_name, "w");
    if (output_file == 0)
	open_error(output_file_name);

    if (rflag)
    {
	code_file = fopen(code_file_name, "w");
	if (code_file == 0)
	    open_error(code_file_name);
    }
    else
	code_file = output_file;
}


int
main(argc, argv)
int argc;
char *argv[];
{
    set_signals();
    getargs(argc, argv);
    open_files();
    reader();
    lr0();
    lalr();
    make_parser();
    verbose();
    output();
    done(0);
    /*NOTREACHED*/
    return (0);
}
