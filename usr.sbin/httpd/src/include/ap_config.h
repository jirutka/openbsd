/*	$OpenBSD: src/usr.sbin/httpd/src/include/ap_config.h,v 1.12 2002/07/17 13:11:02 henning Exp $ */

/* ====================================================================
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2000-2002 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Apache" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 *
 * Portions of this software are based upon public domain software
 * originally written at the National Center for Supercomputing Applications,
 * University of Illinois, Urbana-Champaign.
 */

#ifndef AP_CONFIG_H
#define AP_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ap_config.h: system-dependant #defines and includes...
 * See PORTING for a listing of what they mean
 */

#include "ap_mmn.h"		/* MODULE_MAGIC_NUMBER_ */

/*
 * Support for platform dependent autogenerated defines
 */
#if !defined(WIN32) && !defined(NETWARE)
#include "ap_config_auto.h"
#else
/* not available under WIN32, so provide important entries manually */
#undef HAVE_UNISTD_H
#endif

/* Have to include sys/stat.h before ../win32/os.h so we can override
stat() properly */
#ifndef NETWARE
#include <sys/types.h>
#endif
#include <sys/stat.h>


/* So that we can use inline on some critical functions, and use
 * GNUC attributes (such as to get -Wall warnings for printf-like
 * functions).  Only do this in gcc 2.7 or later ... it may work
 * on earlier stuff, but why chance it.
 *
 * We've since discovered that the gcc shipped with NeXT systems
 * as "cc" is completely broken.  It claims to be __GNUC__ and so
 * on, but it doesn't implement half of the things that __GNUC__
 * means.  In particular it's missing inline and the __attribute__
 * stuff.  So we hack around it.  PR#1613. -djg
 */
#if !defined(__GNUC__) || __GNUC__ < 2 || \
    (__GNUC__ == 2 && __GNUC_MINOR__ < 7) ||\
    defined(NEXT)
#define ap_inline
#define __attribute__(__x)
#define ENUM_BITFIELD(e,n,w)  signed int n : w
#else
#define ap_inline __inline__
#define USE_GNU_INLINE
#define ENUM_BITFIELD(e,n,w)  e n : w
#endif

#include "os.h"

/* Define one of these according to your system. */
#if defined(MINT)
typedef int rlim_t;
#define JMP_BUF sigjmp_buf
#define NO_LONG_DOUBLE
#define HAVE_FLOCK_SERIALIZED_ACCEPT
#define _BSD_SOURCE
#define EAGAIN EWOULDBLOCK
int initgroups (char *, int);     
char *crypt (const char *pw, const char *salt);
int gethostname (char *name, int namelen);

#elif defined(MPE)
#include <sys/times.h>
#define NO_SETSID
#define NO_KILLPG
#define NO_WRITEV
#define HAVE_SHMGET 1
#define USE_SHMGET_SCOREBOARD
/* 
   UID/GID isn't a native concept for MPE, and it's definitely not a 100%
   Unix implementation.  There isn't a traditional superuser concept either,
   so we're forced to liberalize SHM security a bit so the parent & children
   can communicate when they're running with different UIDs within the same
   GID (the GID will *always* be the same on MPE).  Thus the weird SHM_R and
   SHM_W below.
*/
#define SHM_R 0440  /* Read permission */
#define SHM_W 0220  /* Write permission */
#define NEED_INITGROUPS
#define NEED_STRCASECMP
#define NEED_STRDUP
#define NEED_STRNCASECMP
extern void GETPRIVMODE();
extern void GETUSERMODE();
extern char *inet_ntoa();
#define NO_SLACK
#define S_IEXEC  S_IXUSR
#define S_IREAD  S_IRUSR
#define S_IWRITE S_IWUSR
#define PF_INET  AF_INET
#define HAVE_FCNTL_SERIALIZED_ACCEPT

#elif defined(SUNOS4)
#define HAVE_GMTOFF 1
#undef NO_KILLPG
#undef NO_SETSID
char *crypt(const char *pw, const char *salt);
char *mktemp(char *);
#define HAVE_MMAP 1
#define USE_MMAP_SCOREBOARD
#define USE_MMAP_FILES
#include <sys/time.h>
#define NEED_STRERROR
typedef int rlim_t;
#define memmove(a,b,c) bcopy(b,a,c)
#define NO_LINGCLOSE
#define HAVE_FLOCK_SERIALIZED_ACCEPT
#define NEED_DIFFTIME
#define HAVE_SYSLOG 1

#elif defined(SOLARIS2)
#undef HAVE_GMTOFF
#define NO_KILLPG
#undef NO_SETSID
#define bzero(a,b) memset(a,0,b)
#define HAVE_SYSVSEM_SERIALIZED_ACCEPT
#define HAVE_FCNTL_SERIALIZED_ACCEPT
#define HAVE_PTHREAD_SERIALIZED_ACCEPT
#if !defined(USE_SYSVSEM_SERIALIZED_ACCEPT) && \
    !defined(USE_PTHREAD_SERIALIZED_ACCEPT)
#define USE_FCNTL_SERIALIZED_ACCEPT
#endif
#define NEED_UNION_SEMUN
#define HAVE_MMAP 1
#define USE_MMAP_SCOREBOARD
#define USE_MMAP_FILES
int gethostname(char *name, int namelen);
#define HAVE_SYSLOG 1
#define SYS_SIGLIST _sys_siglist

#elif defined(IRIX)
#undef HAVE_GMTOFF
/* IRIX has killpg, but it's only in _BSD_COMPAT, so don't use it in case
 * there's some weird conflict with non-BSD signals */
#define NO_KILLPG
#undef NO_SETSID
#define HAVE_FLOCK_SERIALIZED_ACCEPT
#define HAVE_FCNTL_SERIALIZED_ACCEPT
#define HAVE_USLOCK_SERIALIZED_ACCEPT
#define HAVE_SYSVSEM_SERIALIZED_ACCEPT
#if !defined(USE_FLOCK_SERIALIZED_ACCEPT) && \
    !defined(USE_USLOCK_SERIALIZED_ACCEPT) && \
    !defined(USE_SYSVSEM_SERIALIZED_ACCEPT)
#define USE_FCNTL_SERIALIZED_ACCEPT
#endif
#define HAVE_SHMGET 1
#define USE_SHMGET_SCOREBOARD
#define HAVE_MMAP 1
#define USE_MMAP_FILES
#define NO_LONG_DOUBLE
#define NO_LINGCLOSE
#define HAVE_SYSLOG 1

#elif defined(HIUX)
#undef HAVE_GMTOFF
#define NO_KILLPG
#undef NO_SETSID
#ifndef _HIUX_SOURCE
#define _HIUX_SOURCE
#endif
#define HAVE_SHMGET 1
#define USE_SHMGET_SCOREBOARD
#define SELECT_NEEDS_CAST
#define HAVE_SYSLOG 1

#elif defined(HPUX) || defined(HPUX10)
#undef HAVE_GMTOFF
#define NO_KILLPG
#undef NO_SETSID
#define HAVE_FCNTL_SERIALIZED_ACCEPT
#ifndef _HPUX_SOURCE
#define _HPUX_SOURCE
#endif
#define HAVE_SHMGET 1
#define USE_SHMGET_SCOREBOARD
#define HAVE_SYSLOG 1
#ifndef HPUX10
#define SELECT_NEEDS_CAST
typedef int rlim_t;
#endif

#elif defined(HPUX11)
#ifndef _HPUX_SOURCE
#define _HPUX_SOURCE
#endif
#define HAVE_SHMGET
#define USE_SHMGET_SCOREBOARD
#undef  HAVE_GMTOFF
#define HAVE_FCNTL_SERIALIZED_ACCEPT
/* feeling brave?  want to try using POSIX mutexes? */
/* #define HAVE_MMAP */
/* #define USE_MMAP_SCOREBOARD */
/* #define USE_MMAP_FILES */
/* #define HAVE_PTHREAD_SERIALIZED_ACCEPT */
#define NO_KILLPG
#undef  NO_SETSID
#define HAVE_SYSLOG

#elif defined(AIX)
#undef HAVE_GMTOFF
#undef NO_KILLPG
#undef NO_SETSID
#ifndef __ps2__
#define HAVE_MMAP 1
#define USE_MMAP_SCOREBOARD
#define USE_MMAP_FILES
#define HAVE_SYSLOG 1
#ifndef DEFAULT_GROUP
#define DEFAULT_GROUP "nobody"
#endif
#endif
#ifndef DEFAULT_USER
#define DEFAULT_USER "nobody"
#endif
#ifdef NEED_RLIM_T
typedef int rlim_t;
#endif
#define HAVE_FCNTL_SERIALIZED_ACCEPT
#define HAVE_SYSVSEM_SERIALIZED_ACCEPT
#define NEED_UNION_SEMUN
#if !defined(USE_PTHREAD_SERIALIZED_ACCEPT)
#define USE_FCNTL_SERIALIZED_ACCEPT
#endif
#ifdef USEBCOPY
#define memmove(a,b,c) bcopy(b,a,c)
#endif
#if AIX >= 51
#define NET_SIZE_T socklen_t
#elif AIX >= 42
#define NET_SIZE_T size_t
#endif

#elif defined(ULTRIX)
/* we don't want to use sys/resource.h under
   Ultrix although this header exists. */
#undef HAVE_SYS_RESOURCE_H
#define HAVE_GMTOFF 1
#undef NO_KILLPG
#undef NO_SETSID
#define ULTRIX_BRAIN_DEATH
#define NEED_STRDUP
/* If you have Ultrix 4.3, and are using cc, const is broken */
#ifndef __ultrix__		/* Hack to check for pre-Ultrix 4.4 cc */
#define const			/* Not implemented */
#endif

#elif defined(OSF1)
#define HAVE_GMTOFF 1
#undef NO_KILLPG
#undef NO_SETSID
#define HAVE_MMAP 1
#define USE_MMAP_SCOREBOARD
#define USE_MMAP_FILES
#define NO_LONG_DOUBLE
#define HAVE_SYSLOG 1
#define HAVE_FLOCK_SERIALIZED_ACCEPT
#define SINGLE_LISTEN_UNSERIALIZED_ACCEPT

#elif defined(PARAGON)
#define HAVE_GMTOFF 1
#undef NO_KILLPG
#undef NO_SETSID
#define HAVE_MMAP 1
#define USE_MMAP_SCOREBOARD
#define USE_MMAP_FILES
#define NO_LONG_DOUBLE
#define HAVE_SYSLOG 1
typedef int rlim_t;

#elif defined(SEQUENT)
#define DEFAULT_USER "nobody"
#define DEFAULT_GROUP "nobody"
#define NO_SHMGET 1
#define HAVE_MMAP 1
#define HAVE_SYSLOG 1
#define USE_MMAP_FILES 1
#define USE_MMAP_SCOREBOARD 1
#define HAVE_FCNTL_SERIALIZED_ACCEPT 1
#define JMP_BUF sigjmp_buf
#undef NO_SETSID
#if SEQUENT < 40
typedef int rlim_t;
#define NO_GETTIMEOFDAY
#undef HAVE_SYS_RESOURCE_H /* exists but does not provide *rlimit funcs */
#include <sys/times.h>
#endif
#if SEQUENT < 42
#define NEED_STRCASECMP
#define NEED_STRNCASECMP
#endif
#if SEQUENT < 44
#define NO_KILLPG 1
#define NET_SIZE_T int
#endif
#if SEQUENT >= 44
#undef NO_KILLPG
#define NET_SIZE_T size_t
#endif

#elif defined(NEXT)
typedef unsigned short mode_t;
typedef int rlim_t;
#define HAVE_GMTOFF 1
#undef NO_KILLPG
#define NO_SETSID
#define NEED_STRDUP
#define NO_LINGCLOSE
#undef _POSIX_SOURCE
#ifndef FD_CLOEXEC
#define FD_CLOEXEC 1
#endif
#ifndef S_ISDIR
#define S_ISDIR(m)      (((m)&(S_IFMT)) == (S_IFDIR))
#endif
#ifndef S_ISREG
#define S_ISREG(m)      (((m)&(S_IFMT)) == (S_IFREG))
#endif
#ifndef S_IXUSR
#define S_IXUSR 00100
#endif
#ifndef S_IRGRP
#define S_IRGRP 00040
#endif
#ifndef S_IXGRP
#define S_IXGRP 00010
#endif
#ifndef S_IROTH
#define S_IROTH 00004
#endif
#ifndef S_IXOTH
#define S_IXOTH 00001
#endif
#ifndef S_IRUSR
#define S_IRUSR S_IREAD
#endif
#ifndef S_IWUSR
#define S_IWUSR S_IWRITE
#endif
#ifndef S_IWGRP
#define S_IWGRP	000020
#endif
#ifndef S_IWOTH
#define S_IWOTH 000002
#endif

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

/* PR#2293 fix */
#define	ap_wait_t	union wait
#define waitpid(a,b,c) wait4((a) == -1 ? 0 : (a),(union wait *)(b),c,NULL)
#define WEXITSTATUS(status)     (int)( WIFEXITED(status) ? ( (status).w_retcode ) : -1)
#define WTERMSIG(status)	(int)( (status).w_termsig )

typedef int pid_t;
#define USE_LONGJMP
#define NO_USE_SIGACTION
#define HAVE_SYSLOG 1

#if defined(__DYNAMIC__)
#define HAVE_DYLD
#define DYLD_CANT_UNLOAD
#endif

#elif defined(DARWIN) /* Darwin (Mac OS) */
#undef PLATFORM
#define PLATFORM "Darwin"
#define HAVE_DYLD
#define HAVE_GMTOFF
#define HAVE_MMAP
#define USE_MMAP_FILES
#define USE_MMAP_SCOREBOARD
#ifdef MAC_OS_X_SERVER
#define MAP_TMPFILE
#endif /* MAC_OS_X_SERVER */
#define HAVE_RESOURCE
#define HAVE_SNPRINTF
#define JMP_BUF jmp_buf
#define USE_LONGJMP
#define HAVE_FLOCK_SERIALIZED_ACCEPT
#define HAVE_FCNTL_SERIALIZED_ACCEPT
#define USE_FLOCK_SERIALIZED_ACCEPT
#define SINGLE_LISTEN_UNSERIALIZED_ACCEPT

#elif defined(LINUX)

#if LINUX > 1
#include <features.h>

/* libc4 systems probably still work, it probably doesn't define
 *  __GNU_LIBRARY__
 * libc5 systems define __GNU_LIBRARY__ == 1, but don't define __GLIBC__
 * glibc 2.x and later systems define __GNU_LIBRARY__ == 6, but list it as
 * "deprecated in favour of __GLIBC__"; the value 6 will never be changed.
 * glibc 1.x systems (i.e. redhat 4.x on sparc/alpha) should have
 * __GLIBC__ < 2
 * all glibc based systems need crypt.h
 */
#if defined(__GNU_LIBRARY__) && __GNU_LIBRARY__ > 1
#include <crypt.h>
#endif

/* glibc 2.0.0 through 2.0.4 need size_t * here, where 2.0.5 needs socklen_t *
 * there's no way to discern between these two libraries.  But using int should
 * be portable because otherwise these libs would be hopelessly broken with
 * reams of existing networking code.  We'll use socklen_t * for 2.1.x and
 * later.
 *
 * int works for all the earlier libs, and is picked up by default later.
 */
#if defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ > 0))
#define NET_SIZE_T socklen_t
#endif

#define HAVE_SHMGET 1
#define USE_SHMGET_SCOREBOARD
#define HAVE_MMAP 1
#define USE_MMAP_FILES

#if LINUX > 20
/* see Pine.LNX.4.21.0011041233550.1897-100000@twinlark.arctic.org
 * in new-httpd archives for performance numbers indicating these
 * are the right choices for linux 2.2.x and later
 */
#define HAVE_SYSVSEM_SERIALIZED_ACCEPT
#define HAVE_FCNTL_SERIALIZED_ACCEPT
#define SINGLE_LISTEN_UNSERIALIZED_ACCEPT 
#include <sys/sem.h>
#if _SEM_SEMUN_UNDEFINED
#define NEED_UNION_SEMUN
#endif
#else
#define USE_FCNTL_SERIALIZED_ACCEPT
#endif

#define SYS_SIGLIST	_sys_siglist

#else
#define USE_FCNTL_SERIALIZED_ACCEPT
#endif

#undef HAVE_GMTOFF
#undef NO_KILLPG
#undef NO_SETSID
#undef NEED_STRDUP
#include <sys/time.h>
#define HAVE_SYSLOG 1

/* glibc 2.1 and later finally define rlim_t */
#if !defined(__GLIBC__) || __GLIBC__ < 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ < 1)
typedef int rlim_t;
#endif

#elif defined(SCO)
#undef HAVE_GMTOFF
#undef NO_KILLPG
#undef NO_SETSID
#define NEED_INITGROUPS
#define NO_WRITEV
#include <sys/time.h>
#define HAVE_SYSLOG 1
#undef HAVE_SYS_RESOURCE_H

#elif defined(ATHEOS)

#include <features.h>
#include <crypt.h>
#include <sys/time.h>

#define HAVE_FCNTL_SERIALIZED_ACCEPT
#define USE_FCNTL_SERIALIZED_ACCEPT

#undef HAVE_GMTOFF
#undef NO_KILLPG
#undef NO_SETSID
#undef NEED_STRDUP
#define HAVE_SYSLOG 1

#ifdef PLATFORM
#undef PLATFORM
#endif
#define PLATFORM "AtheOS"

#elif defined(SCO5)

#define HAVE_FCNTL_SERIALIZED_ACCEPT
#define HAVE_MMAP 1
#define USE_MMAP_SCOREBOARD
#define USE_MMAP_FILES
#define SecureWare
#define HAVE_SYSLOG 1

/* Although SCO 5 defines these in <strings.h> (note the "s") they don't have
   consts. Sigh. */
extern int strcasecmp(const char *, const char *);
extern int strncasecmp(const char *, const char *, unsigned);

#elif defined(AUX3)
/* These are to let -Wall compile more cleanly */
extern int strcasecmp(const char *, const char *);
extern int strncasecmp(const char *, const char *, unsigned);
extern int set42sig(), getopt(), getpeername(), bzero();
extern int listen(), bind(), socket(), getsockname();
extern int accept(), gethostname(), connect(), lstat();
extern int select(), killpg(), shutdown();
extern int initgroups(), setsockopt();
extern char *shmat();
extern int shmctl();
extern int shmget();
extern char *sbrk();
extern char *crypt();
#include <sys/time.h>
#undef HAVE_GMTOFF
#undef NO_KILLPG
#undef NO_SETSID
#define NEED_STRDUP
/* fcntl() locking is expensive with NFS */
#define HAVE_FLOCK_SERIALIZED_ACCEPT
#define SINGLE_LISTEN_UNSERIALIZED_ACCEPT
#define HAVE_SHMGET 1
#define USE_SHMGET_SCOREBOARD
/*
 * NOTE: If when you run Apache under A/UX and you get a warning
 * that httpd couldn't move break, then the below value for
 * MOVEBREAK (64megs) is too large for your setup. Try reducing
 * to 0x2000000 which is still PLENTY of space. I doubt if
 * even on heavy systems sbrk() would be called at all...
 */
#define MOVEBREAK		0x4000000
#define NO_LINGCLOSE
#define NO_SLACK
#define HAVE_SYSLOG 1
#undef HAVE_SYS_RESOURCE_H	/* exists but does not provide *rlimit funcs */

#elif defined(SVR4)
#define NO_KILLPG
#undef  NO_SETSID
#undef NEED_STRDUP
#ifndef MPRAS
#define NEED_STRCASECMP
#ifndef ENCORE
#define NEED_STRNCASECMP
#endif /* ENCORE */
#endif /* MPRAS */
#define bzero(a,b) memset(a,0,b)
/* A lot of SVR4 systems need this */
#define HAVE_FCNTL_SERIALIZED_ACCEPT
#ifdef SNI
#define HAVE_SYSVSEM_SERIALIZED_ACCEPT
#endif
#ifndef USE_SYSVSEM_SERIALIZED_ACCEPT
#define USE_FCNTL_SERIALIZED_ACCEPT
#endif
#define HAVE_SYSLOG 1
#define NET_SIZE_T size_t
#define HAVE_SHMGET 1
#define USE_SHMGET_SCOREBOARD
#ifdef _OSD_POSIX /* BS2000-POSIX mainframe needs initgroups */
#define NEED_HASHBANG_EMUL /* execve() doesn't start shell scripts by default */
#define _KMEMUSER          /* Enable SHM_R/SHM_W defines in <shm.h> */
#undef NEED_STRCASECMP
#undef NEED_STRNCASECMP
#undef bzero
#endif /*_OSD_POSIX*/

#elif defined(UW)
#define HAVE_FCNTL_SERIALIZED_ACCEPT
#if UW < 700
#define NO_LINGCLOSE
#define NO_KILLPG
#else
#define SINGLE_LISTEN_UNSERIALIZED_ACCEPT
#endif
#undef  NO_SETSID
#undef NEED_STRDUP
#define NEED_STRCASECMP
#define NEED_STRNCASECMP
#define bzero(a,b) memset(a,0,b)
#define HAVE_MMAP 1
#define USE_MMAP_SCOREBOARD
#define USE_MMAP_FILES
#define HAVE_SHMGET 1
#undef USE_SHMGET_SCOREBOARD	/* force use of mmap() scoreboard */
#include <sys/time.h>
#if UW >= 200
#define _POSIX_SOURCE
#endif
#define NET_SIZE_T size_t
#define HAVE_SYSLOG 1

#elif defined(DGUX)
#define NO_KILLPG
#undef  NO_SETSID
#undef NEED_STRDUP
#ifdef _IX86_DG
#undef NEED_STRCASECMP
#undef NEED_STRNCASECMP
#else
#define NEED_STRCASECMP
#define NEED_STRNCASECMP
#endif
#define bzero(a,b) memset(a,0,b)
/* A lot of SVR4 systems need this */
#define HAVE_FCNTL_SERIALIZED_ACCEPT
#define ap_inet_addr inet_network
#define HAVE_SYSLOG 1

#elif defined(__NetBSD__) || defined(__OpenBSD__) || defined(NETBSD)
#define HAVE_GMTOFF 1
#undef NO_KILLPG
#undef NO_SETSID
#define HAVE_SYSLOG 1
#ifndef DEFAULT_USER
#define DEFAULT_USER "nobody"
#endif
#ifndef DEFAULT_GROUP
#define DEFAULT_GROUP "nogroup"
#endif
#define HAVE_SHMGET 1
#define HAVE_MMAP 1
#define USE_MMAP_SCOREBOARD
#define USE_MMAP_FILES
#if defined __OpenBSD__
#define HAVE_SYSVSEM_SERIALIZED_ACCEPT
#define USE_SYSVSEM_SERIALIZED_ACCEPT
#endif
#define HAVE_FLOCK_SERIALIZED_ACCEPT
#define SINGLE_LISTEN_UNSERIALIZED_ACCEPT

#elif defined(UTS21)
#undef HAVE_GMTOFF
#undef NO_KILLPG
#define NO_SETSID
#define NEED_WAITPID
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#define HAVE_SYSLOG 1
#define USE_LONGJMP
#define JMP_BUF jmp_buf
#define NO_USE_SIGACTION
#define NEED_STRERROR
#define NEED_STRSTR
#define NEED_HASHBANG_EMUL
#define NDELAY_PIPE_RETURNS_ZERO
#define NO_DATA NO_ADDRESS
#define	ap_wait_t		union wait
#define WEXITSTATUS(status)	(int)((status).w_retcode)
#define WTERMSIG(status)	(int)((status).w_termsig)
#define strftime(buf,bufsize,fmt,tm)    ascftime(buf,fmt,tm)
#undef HAVE_SYS_RESOURCE_H /* exists but does not provide *rlimit funcs */
#include <sys/types.h>
#include <sys/time.h>     

#elif defined(APOLLO)
#undef HAVE_GMTOFF
#undef NO_KILLPG
#undef NO_SETSID
#define HAVE_SYSLOG 1

#elif defined(__FreeBSD__) || defined(__bsdi__)
#if defined(__FreeBSD__)
#include <osreldate.h>
#endif
#define HAVE_GMTOFF 1
#undef NO_KILLPG
#undef NO_SETSID
#define HAVE_MMAP 1
#define USE_MMAP_SCOREBOARD
#define USE_MMAP_FILES
#ifndef DEFAULT_USER
#define DEFAULT_USER "nobody"
#endif
#ifndef DEFAULT_GROUP
#define DEFAULT_GROUP "nogroup"
#endif
#if defined(__bsdi__) || \
(defined(__FreeBSD_version) && (__FreeBSD_version < 220000))
typedef quad_t rlim_t;
#endif
#define HAVE_FLOCK_SERIALIZED_ACCEPT
#define SINGLE_LISTEN_UNSERIALIZED_ACCEPT
#define HAVE_SYSLOG 1
#define SYS_SIGLIST sys_siglist
#if (defined(__FreeBSD_version) && (__FreeBSD_version >= 400000))
#define NET_SIZE_T socklen_t
#endif

#elif defined(QNX)
#ifndef crypt
char *crypt(const char *pw, const char *salt);
#endif
#ifndef initgroups
int initgroups(char *, int);
#endif
#ifndef strncasecmp
#define strncasecmp strnicmp
#endif
#undef NO_KILLPG
#undef NO_SETSID
#define NEED_INITGROUPS
#define NEED_SELECT_H
#define NEED_PROCESS_H
#include <unix.h>
#define HAVE_MMAP 1
#define USE_POSIX_SCOREBOARD
#define HAVE_FLOCK_SERIALIZED_ACCEPT
#define SINGLE_LISTEN_UNSERIALIZED_ACCEPT
#define HAVE_SYSLOG 1

#elif defined(LYNXOS)
#undef HAVE_GMTOFF
#undef USE_MMAP_SCOREBOARD
#undef USE_SHMGET_SCOREBOARD
#undef HAVE_FCNTL_SERIALIZED_ACCEPT
#undef HAVE_FLOCK_SERIALIZED_ACCEPT
#define HAVE_NONE_SERIALIZED_ACCEPT
#define USE_LONGJMP
#undef NO_KILLPG
#undef NO_SETSID
#undef NO_USE_SIGACTION
#undef NO_LINGCLOSE
extern char *crypt(char *pw, char *salt);
typedef int rlim_t;
#define HAVE_SYSLOG 1

#elif defined(UXPDS)
#undef NEED_STRCASECMP
#undef NEED_STRNCASECMP
#undef NEED_STRDUP
#undef HAVE_GMTOFF
#define NO_KILLPG
#undef NO_SETSID
#define bzero(a,b) memset(a,0,b)
#define HAVE_FCNTL_SERIALIZED_ACCEPT
#define HAVE_MMAP 1
#define USE_MMAP_SCOREBOARD
#define USE_MMAP_FILES
#define HAVE_SYSLOG 1

#elif defined(OS2)
/* Defines required for EMX OS/2 port. */
#define NO_KILLPG
#define NEED_STRCASECMP
#define NEED_STRNCASECMP
#define NEED_PROCESS_H
#define NO_SETSID
#define NO_TIMES
#define CASE_BLIND_FILESYSTEM
/* Add some drive name support */
#define chdir _chdir2
#include <sys/time.h>
#define MAXSOCKETS 2048
#define USE_OS2_SCOREBOARD
#define NO_RELIABLE_PIPED_LOGS
#define HAVE_OS2SEM_SERIALIZED_ACCEPT
#define SINGLE_LISTEN_UNSERIALIZED_ACCEPT
#define NO_SLACK
#define FOPEN_REQUIRES_T

#elif defined(__MACHTEN__)
typedef int rlim_t;
#undef NO_KILLPG
#define NO_SETSID
#define HAVE_GMTOFF 1
#ifndef __MACHTEN_PPC__
#ifndef __MACHTEN_68K__
#define __MACHTEN_68K__
#endif
#define HAVE_FLOCK_SERIALIZED_ACCEPT
#define NO_USE_SIGACTION
#define JMP_BUF sigjmp_buf
#define USE_LONGJMP
#undef NEED_STRDUP
#else
#define HAVE_SHMGET 1
#define USE_SHMGET_SCOREBOARD
#define HAVE_FCNTL_SERIALIZED_ACCEPT
#endif

/* Convex OS v11 */
#elif defined(CONVEXOS11)
#undef HAVE_GMTOFF
#undef NO_KILLPG
#undef NO_SETSID
#undef NEED_STRDUP
#define HAVE_MMAP 1
#define USE_MMAP_SCOREBOARD
#define USE_MMAP_FILES
#define HAVE_SYSLOG 1

#define NO_TIMEZONE
#include <stdio.h>
#include <sys/types.h>
typedef int rlim_t;

#elif defined(ISC)
#include <net/errno.h>
#define NO_KILLPG
#undef NO_SETSID
#define HAVE_SHMGET 1
#define USE_SHMGET_SCOREBOARD
#define HAVE_FCNTL_SERIALIZED_ACCEPT
#define HAVE_SYSLOG 1

#elif defined(NEWSOS)
#define HAVE_SHMGET 1
#define USE_SHMGET_SCOREBOARD
#define USE_LONGJMP
#define NO_SETSID
#define NO_USE_SIGACTION
#define NEED_WAITPID
#define NO_OTHER_CHILD
#define HAVE_SYSLOG 1
#include <sys/time.h>
#include <stdlib.h>
#include <sys/types.h>
typedef int pid_t;
typedef int rlim_t;
typedef int mode_t;

#elif defined(RISCIX)
#include <sys/time.h>
typedef int rlim_t;
#define NO_USE_SIGACTION
#define USE_LONGJMP
#define NEED_STRCASECMP
#define NEED_STRNCASECMP
#define NEED_STRDUP

#elif defined(BEOS)
#undef PLATFORM
#define PLATFORM "BeOS"
#include <stddef.h>
#include <kernel/OS.h>

#define HAVE_BEOS_SERIALIZED_ACCEPT
#define SINGLE_LISTEN_UNSERIALIZED_ACCEPT

#define NO_WRITEV
#define NO_KILLPG
#define NEED_INITGROUPS
#define PF_INET AF_INET
#define S_IEXEC S_IXUSR

#elif defined(BONE)
#undef PLATFORM
#define PLATFORM "BeOS BONE"
#include <kernel/OS.h>

#define NO_KILLPG
#define NEED_INITGROUPS
#define S_IEXEC S_IXUSR
#define HAVE_BEOS_SERIALIZED_ACCEPT
#define SINGLE_LISTEN_UNSERIALIZED_ACCEPT

#elif defined(_CX_SX)
#define JMP_BUF sigjmp_buf
#include <sys/types.h>
#include <sys/time.h>

#elif defined(WIN32)

/* All windows stuff is now in os/win32/os.h */

#elif defined(TPF) /* IBM Transaction Processing Facility operating system */

#include <tpfeq.h>
#include <tpfio.h>
#include <sysapi.h>
#include <sysgtime.h>
#define PRIMECRAS 0x010000
#define JMP_BUF jmp_buf
#define HAVE_SHMGET
#undef  HAVE_SYS_RESOURCE_H
#define NEED_INITGROUPS
#define NEED_SIGNAL_INTERRUPT
#include <strings.h>
#ifndef __strings_h
#define NEED_STRCASECMP
#define NEED_STRNCASECMP
#endif
#define NEED_STRDUP
#define NO_DBM_REWRITEMAP
#define NO_GETTIMEOFDAY
#define NO_LINGCLOSE
#define NO_MMAP
#define NO_OTHER_CHILD
#define NO_PIPED_LOGS
#define NO_RELIABLE_PIPED_LOGS
#define NO_SETSID
#define NO_SLACK
#define NO_TIMES
#define NO_USE_SIGACTION
#define USE_LONGJMP
#define USE_SHMGET_SCOREBOARD
/*#define USE_TPF_SCOREBOARD*/
#define USE_TPF_ACCEPT
#define HAVE_TPF_CORE_SERIALIZED_ACCEPT
#define USE_TPF_SELECT
#define S_IREAD S_IRUSR
#define S_IWRITE S_IWUSR
#define S_IEXEC S_IXUSR
#include <unistd.h>
#define crypt(buf,salt) ((char *)buf)
#undef  offsetof
#define offsetof(s_type,field) ((size_t)&(((s_type*)0)->field))

#elif defined(__TANDEM)
#define NO_WRITEV
#define NO_KILLPG
#define NEED_INITGROUPS
#define NO_SLACK

#elif defined(OS390)                /* IBM OS/390 Operating System      */
#define HAVE_MMAP
#define HAVE_SHMGET
#define USE_SHMGET_SCOREBOARD
#define USE_MMAP_FILES
#define NEED_UNION_SEMUN
#define HAVE_SYSVSEM_SERIALIZED_ACCEPT
#define _POSIX_SOURCE
#include <signal.h>
#ifdef SIGDUMP  /* SIGDUMP is not defined by OS/390 v1r2 */
#define NSIG SIGDUMP+1
#else
#define NSIG 40
#endif
#define JMP_BUF sigjmp_buf
#define _XOPEN_SOURCE_EXTENDED 1
#define _OPEN_MSGQ_EXT
#define _XOPEN_SOURCE
#define SHM_R S_IRUSR
#define SHM_W S_IWUSR
#include <sys/time.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/socket.h>
#define NET_SIZE_T size_t
#define NEED_HASHBANG_EMUL

#elif defined(CYGWIN)               /* Cygwin 1.x POSIX layer for Win32 */
#define JMP_BUF jmp_buf
#define NO_KILLPG
#define USE_LONGJMP
#define GDBM_STATIC
#define HAVE_MMAP 1
#define USE_MMAP_SCOREBOARD
#define USE_MMAP_FILES
#define HAVE_SYSLOG 1
#define HAVE_FCNTL_SERIALIZED_ACCEPT
#define HAVE_PTHREAD_SERIALIZED_ACCEPT
#define SINGLE_LISTEN_UNSERIALIZED_ACCEPT
#if !defined(USE_FNCTL_SERIALIZED_ACCEPT)
#define USE_PTHREAD_SERIALIZED_ACCEPT
#endif


#else
/* Unknown system - Edit these to match */
#ifdef BSD
#define HAVE_GMTOFF 1
#else
#undef HAVE_GMTOFF
#endif
/* NO_KILLPG is set on systems that don't have killpg */
#undef NO_KILLPG
/* NO_SETSID is set on systems that don't have setsid */
#undef NO_SETSID
/* NEED_STRDUP is set on stupid systems that don't have strdup. */
#undef NEED_STRDUP
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif /* HAVE_SYS_PARAM_H */

/* stuff marked API_EXPORT is part of the API, and intended for use
 * by modules
 */
#ifndef API_EXPORT
#define API_EXPORT(type)    type
#endif

/* Stuff marked API_EXPORT_NONSTD is part of the API, and intended for
 * use by modules.  The difference between API_EXPORT and
 * API_EXPORT_NONSTD is that the latter is required for any functions
 * which use varargs or are used via indirect function call.  This
 * is to accomodate the two calling conventions in windows dlls.
 */
#ifndef API_EXPORT_NONSTD
#define API_EXPORT_NONSTD(type)    type
#endif

#ifndef MODULE_VAR_EXPORT
#define MODULE_VAR_EXPORT
#endif
#ifndef API_VAR_EXPORT
#define API_VAR_EXPORT
#endif

/* modules should not used functions marked CORE_EXPORT
 * or CORE_EXPORT_NONSTD */
#ifndef CORE_EXPORT
#define CORE_EXPORT	API_EXPORT
#endif
#ifndef CORE_EXPORT_NONSTD
#define CORE_EXPORT_NONSTD	API_EXPORT_NONSTD
#endif

/* On Darwin, symbols that conflict with loaded dylibs
 * (eg. System framework) need to be declared as private symbols with
 * __private_extern__.
 * For other systems, make that a no-op.
 */
#if defined(DARWIN) && defined(__DYNAMIC__)
#define ap_private_extern __private_extern__
#else
#define ap_private_extern
#endif

/*
 * The particular directory style your system supports. If you have dirent.h
 * in /usr/include (POSIX) or /usr/include/sys (SYSV), #include 
 * that file and define DIR_TYPE to be dirent. Otherwise, if you have 
 * /usr/include/sys/dir.h, define DIR_TYPE to be direct and include that
 * file. If you have neither, I'm confused.
 */

#ifndef NETWARE
#include <sys/types.h>
#endif
#include <stdarg.h>

#if !defined(NEXT) && !defined(WIN32)
#include <dirent.h>
#define DIR_TYPE dirent
#elif !defined(WIN32)
#include <sys/dir.h>
#define DIR_TYPE direct
#else
#define DIR_TYPE dirent
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __TANDEM
#include <strings.h>
#endif
#include "ap_ctype.h"
#if !defined(MPE) && !defined(WIN32) && !defined(TPF) && !defined(__TANDEM) && !defined(NETWARE)
#include <sys/file.h>
#endif
#if !defined(WIN32) && !defined(NETWARE)
#include <sys/socket.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif /* HAVE_SYS_SELECT_H */
#ifndef TPF
#include <netinet/in.h>
#endif /* TPF */
#if defined(OS390) && !defined(NO_ADDRESS)
#define NO_ADDRESS NO_DATA  /* Not defined properly by OS/390 v1r2 */
#endif
#include <netdb.h>
#include <sys/ioctl.h>
#if !defined(MPE) && !defined(BEOS) && !defined(TPF)
#include <arpa/inet.h>		/* for inet_ntoa */
#endif
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#ifndef BEOS
#define closesocket(s) close(s)
#endif
#ifndef O_BINARY
#define O_BINARY (0)
#endif
#endif /* ndef WIN32 */

#include <limits.h>
#include <time.h>		/* for ctime */
#ifdef WIN32
#define strftime(s,max,format,tm)  os_strftime(s,max,format,tm)
#endif
#include <signal.h>
#ifdef NETWARE
#undef SIGKILL
#undef SA_NOCLDSTOP
#undef SIGALRM
#undef SIGCHILD
#undef SIGCONT
#undef SIGHUP
#undef SIGPIPE
#undef SIGQUIT
#undef SIGSTOP
#undef SIGTSTP
#undef SIGTTIN
#undef SIGTTOU
#undef SIGUSR1
#undef SIGUSR2
#undef SIG_BLOCK
#undef SIG_SETMASK
#undef SIG_UNBLOCK
#endif
#if defined(TPF) && defined(NSIG)
#undef NSIG
#endif
#include <errno.h>
#if !defined(QNX) && !defined(CONVEXOS11) && !defined(NEXT) && !defined(TPF) && !defined(NETWARE) && !defined(MPE)
#include <memory.h>
#endif

#ifdef NEED_PROCESS_H
#include <process.h>
#endif

#if defined(WIN32) || defined(USE_HSREGEX)
#include "hsregex.h"
#else
#include <regex.h>
#endif

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#ifdef SUNOS4
int getrlimit(int, struct rlimit *);
int setrlimit(int, struct rlimit *);
#endif
#endif
#ifdef USE_MMAP_SCOREBOARD
#if !defined(OS2) && !defined(WIN32)
/* This file is not needed for OS/2 */
#include <sys/mman.h>
#endif
#endif
#if !defined(MAP_ANON) && defined(MAP_ANONYMOUS)
#define MAP_ANON MAP_ANONYMOUS
#endif

#if defined(USE_MMAP_FILES) && (defined(NO_MMAP) || !defined(HAVE_MMAP))
#undef USE_MMAP_FILES
#endif

#if defined(USE_MMAP_SCOREBOARD) && (defined(NO_MMAP) || !defined(HAVE_MMAP))
#undef USE_MMAP_SCOREBOARD
#endif

#if defined(USE_SHMGET_SCOREBOARD) && (defined(NO_SHMGET) || !defined(HAVE_SHMGET))
#undef USE_SHMGET_SCOREBOARD
#endif

/* A USE_FOO_SERIALIZED_ACCEPT implies a HAVE_FOO_SERIALIZED_ACCEPT */
#if defined(USE_USLOCK_SERIALIZED_ACCEPT) && !defined(HAVE_USLOCK_SERIALIZED_ACCEPT)
#define HAVE_USLOCK_SERIALIZED_ACCEPT
#endif
#if defined(USE_PTHREAD_SERIALIZED_ACCEPT) && !defined(HAVE_PTHREAD_SERIALIZED_ACCEPT)
#define HAVE_PTHREAD_SERIALIZED_ACCEPT
#endif
#if defined(USE_SYSVSEM_SERIALIZED_ACCEPT) && !defined(HAVE_SYSVSEM_SERIALIZED_ACCEPT)
#define HAVE_SYSVSEM_SERIALIZED_ACCEPT
#endif
#if defined(USE_FCNTL_SERIALIZED_ACCEPT) && !defined(HAVE_FCNTL_SERIALIZED_ACCEPT)
#define HAVE_FCNTL_SERIALIZED_ACCEPT
#endif
#if defined(USE_FLOCK_SERIALIZED_ACCEPT) && !defined(HAVE_FLOCK_SERIALIZED_ACCEPT)
#define HAVE_FLOCK_SERIALIZED_ACCEPT
#endif
#if defined(USE_OS2SEM_SERIALIZED_ACCEPT) && !defined(HAVE_OS2SEM_SERIALIZED_ACCEPT)
#define HAVE_OS2SEM_SERIALIZED_ACCEPT
#endif
#if defined(USE_TPF_CORE_SERIALIZED_ACCEPT) && !defined(HAVE_TPF_CORE_SERIALIZED_ACCEPT)
#define HAVE_TPF_CORE_SERIALIZED_ACCEPT
#endif
#if defined(USE_BEOS_SERIALIZED_ACCEPT) && !defined(HAVE_BEOS_SERIALIZED_ACCEPT)
#define HAVE_BEOS_SERIALIZED_ACCEPT
#endif
#if defined(USE_NONE_SERIALIZED_ACCEPT) && !defined(HAVE_NONE_SERIALIZED_ACCEPT)
#define HAVE_NONE_SERIALIZED_ACCEPT
#endif

#ifndef LOGNAME_MAX
#define LOGNAME_MAX 25
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef ultrix
#define ULTRIX_BRAIN_DEATH
#endif

#ifndef S_ISLNK
#define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#endif

#ifndef INADDR_NONE
#define INADDR_NONE ((unsigned long) -1)
#endif

/*
 * Replace signal function with sigaction equivalent
 */
#ifndef NO_USE_SIGACTION
typedef void Sigfunc(int);

#if defined(SIG_IGN) && !defined(SIG_ERR)
#define SIG_ERR ((Sigfunc *)-1)
#endif

/*
 * For some strange reason, QNX defines signal to signal. Eliminate it.
 */
#ifdef signal
#undef signal
#endif
#define signal(s,f)	ap_signal(s,f)
Sigfunc *signal(int signo, Sigfunc * func);
#endif

#include <setjmp.h>

#if defined(USE_LONGJMP)
#define ap_longjmp(x, y)        longjmp((x), (y))
#define ap_setjmp(x)            setjmp(x)
#ifndef JMP_BUF
#define JMP_BUF jmp_buf
#endif
#else
#define ap_longjmp(x, y)        siglongjmp((x), (y))
#define ap_setjmp(x)            sigsetjmp((x), 1)
#ifndef JMP_BUF
#define JMP_BUF sigjmp_buf
#endif
#endif

/* Majority of os's want to verify FD_SETSIZE */
#if !defined(WIN32) && !defined(TPF) && !defined(NETWARE)
#define CHECK_FD_SETSIZE
#endif

#ifdef USE_TPF_SELECT
#define ap_select(_a, _b, _c, _d, _e)	\
	tpf_select(_a, _b, _c, _d, _e)
#elif defined(SELECT_NEEDS_CAST)
#define ap_select(_a, _b, _c, _d, _e)   \
    select((_a), (int *)(_b), (int *)(_c), (int *)(_d), (_e))
#else
#define ap_select(_a, _b, _c, _d, _e)   \
	select(_a, _b, _c, _d, _e)
#endif

#ifdef USE_TPF_ACCEPT
#define ap_accept(_fd, _sa, _ln)	tpf_accept(_fd, _sa, _ln)
#else
#define ap_accept(_fd, _sa, _ln)	accept(_fd, _sa, _ln)
#endif

#ifdef NEED_SIGNAL_INTERRUPT
#define ap_check_signals()	tpf_process_signals()
#else
#define ap_check_signals()
#endif

#ifdef ULTRIX_BRAIN_DEATH
#define ap_fdopen(d,m) fdopen((d), (char *)(m))
#else
#define ap_fdopen(d,m) fdopen((d), (m))
#endif

#ifndef ap_inet_addr
#define ap_inet_addr inet_addr
#endif

#ifdef NO_OTHER_CHILD
#define NO_RELIABLE_PIPED_LOGS
#endif

/* When the underlying OS doesn't support exec() of scripts which start
 * with a HASHBANG (#!) followed by interpreter name and args, define this.
 */
#ifdef NEED_HASHBANG_EMUL
extern int ap_execle(const char *filename, const char *arg,...);
extern int ap_execve(const char *filename, char * const argv[],
                     char * const envp[]);
/* ap_execle() is a wrapper function around ap_execve(). */
#define execle  ap_execle
#define execve(path,argv,envp)  ap_execve(path,argv,envp)
#endif

/* Finding offsets of elements within structures.
 * Taken from the X code... they've sweated portability of this stuff
 * so we don't have to.  Sigh...
 */

#if defined(CRAY) || (defined(__arm) && !defined(LINUX))
#ifdef __STDC__
#define XtOffset(p_type,field) _Offsetof(p_type,field)
#else
#ifdef CRAY2
#define XtOffset(p_type,field) \
	(sizeof(int)*((unsigned int)&(((p_type)NULL)->field)))

#else /* !CRAY2 */

#define XtOffset(p_type,field) ((unsigned int)&(((p_type)NULL)->field))

#endif /* !CRAY2 */
#endif /* __STDC__ */
#else /* ! (CRAY || __arm) */

#define XtOffset(p_type,field) \
	((long) (((char *) (&(((p_type)NULL)->field))) - ((char *) NULL)))

#endif /* !CRAY */

#ifdef offsetof
#define XtOffsetOf(s_type,field) offsetof(s_type,field)
#else
#define XtOffsetOf(s_type,field) XtOffset(s_type*,field)
#endif

/*
 * NET_SIZE_T exists because of shortsightedness on the POSIX committee.  BSD
 * systems used "int *" as the parameter to accept(), getsockname(),
 * getpeername() et al.  Consequently many unixes took an int * for that
 * parameter.  The POSIX committee decided that "int" was just too generic and
 * had to be replaced with size_t almost everywhere.  There's no problem with
 * that when you're passing by value.  But when you're passing by reference
 * this creates a gross source incompatibility with existing programs.  On
 * 32-bit architectures it creates only a warning.  On 64-bit architectures it
 * creates broken code -- because "int *" is a pointer to a 64-bit quantity and
 * "size_t *" is frequently a pointer to a 32-bit quantity.
 *
 * Some Unixes adopted "size_t *" for the sake of POSIX compliance.  Others
 * ignored it because it was such a broken interface.  Chaos ensued.  POSIX
 * finally woke up and decided that it was wrong and created a new type
 * socklen_t.  The only useful value for socklen_t is int, and that's how
 * everyone who has a clue implements it.  It is almost always the case that
 * NET_SIZE_T should be defined to be an int, unless the system being compiled
 * for was created in the window of POSIX madness.
 */
#ifndef NET_SIZE_T
#define NET_SIZE_T int
#endif

/* Linux defines __WCOREDUMP, but doesn't define WCOREDUMP unless __USE_BSD
 * is in use... we'd prefer to just use WCOREDUMP everywhere.
 */
#if defined(__WCOREDUMP) && !defined(WCOREDUMP)
#define WCOREDUMP __WCOREDUMP
#endif

#ifdef SUNOS_LIB_PROTOTYPES
/* Prototypes needed to get a clean compile with gcc -Wall.
 * Believe it or not, these do have to be declared, at least on SunOS,
 * because they aren't mentioned in the relevant system headers.
 * Sun Quality Software.  Gotta love it.  This section is not 
 * currently (13Nov97) used.
 */

int getopt(int, char **, char *);

int strcasecmp(const char *, const char *);
int strncasecmp(const char *, const char *, int);
int toupper(int);
int tolower(int);

int printf(char *,...);
int fprintf(FILE *, char *,...);
int fputs(char *, FILE *);
int fread(char *, int, int, FILE *);
int fwrite(char *, int, int, FILE *);
int fgetc(FILE *);
char *fgets(char *s, int, FILE*);
int fflush(FILE *);
int fclose(FILE *);
int ungetc(int, FILE *);
int _filbuf(FILE *);	/* !!! */
int _flsbuf(unsigned char, FILE *);	/* !!! */
int sscanf(char *, char *,...);
void setbuf(FILE *, char *);
void perror(char *);

time_t time(time_t *);
int strftime(char *, int, const char *, struct tm *);

int initgroups(char *, int);
int wait3(int *, int, void *);	/* Close enough for us... */
int lstat(const char *, struct stat *);
int stat(const char *, struct stat *);
int flock(int, int);
#ifndef NO_KILLPG
int killpg(int, int);
#endif
int socket(int, int, int);
int setsockopt(int, int, int, const char *, int);
int listen(int, int);
int bind(int, struct sockaddr *, int);
int connect(int, struct sockaddr *, int);
int accept(int, struct sockaddr *, int *);
int shutdown(int, int);

int getsockname(int s, struct sockaddr *name, int *namelen);
int getpeername(int s, struct sockaddr *name, int *namelen);
int gethostname(char *name, int namelen);
void syslog(int, char *,...);
char *mktemp(char *);

int vfprintf(FILE *, const char *, va_list);

#endif /* SUNOS_LIB_PROTOTYPES */

/* The assumption is that when the functions are missing,
 * then there's no matching prototype available either.
 * Declare what is needed exactly as the replacement routines implement it.
 */
#ifdef NEED_STRDUP
extern char *strdup (const char *str);
#endif
#ifdef NEED_STRCASECMP
extern int strcasecmp (const char *a, const char *b);
#endif
#ifdef NEED_STRNCASECMP
extern int strncasecmp (const char *a, const char *b, int n);
#endif
#ifdef NEED_INITGROUPS
extern int initgroups(const char *name, gid_t basegid);
#endif
#ifdef NEED_WAITPID
extern int waitpid(pid_t pid, int *statusp, int options);
#endif
#ifdef NEED_STRERROR
extern char *strerror (int err);
#endif
#ifdef NEED_DIFFTIME
extern double difftime(time_t time1, time_t time0);
#endif

#ifndef ap_wait_t
#define ap_wait_t int
#endif

#ifdef __cplusplus
}
#endif

#endif /* !AP_CONFIG_H */
