/*
 * System call switch table.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * created from	OpenBSD: syscalls.master,v 1.13 1997/08/29 18:45:52 kstailey Exp 
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/signal.h>
#include <sys/mount.h>
#include <sys/syscallargs.h>
#include <compat/svr4/svr4_types.h>
#include <compat/svr4/svr4_time.h>
#include <compat/svr4/svr4_signal.h>
#include <compat/svr4/svr4_ucontext.h>
#include <compat/svr4/svr4_syscallargs.h>
#include <compat/svr4/svr4_statvfs.h>
#include <compat/svr4/svr4_acl.h>

#ifdef COMPAT_43
#define compat_43(func) __CONCAT(compat_43_,func)
#else
#define compat_43(func) sys_nosys
#endif

#ifdef COMPAT_09
#define compat_09(func) __CONCAT(compat_09_,func)
#else
#define compat_09(func) sys_nosys
#endif

#ifdef COMPAT_10
#define compat_10(func) __CONCAT(compat_10_,func)
#else
#define compat_10(func) sys_nosys
#endif

#define	s(type)	sizeof(type)

struct sysent svr4_sysent[] = {
	{ 0, 0,
	    sys_nosys },			/* 0 = syscall */
	{ 1, s(struct sys_exit_args),
	    sys_exit },				/* 1 = exit */
	{ 0, 0,
	    sys_fork },				/* 2 = fork */
	{ 3, s(struct sys_read_args),
	    sys_read },				/* 3 = read */
	{ 3, s(struct sys_write_args),
	    sys_write },			/* 4 = write */
	{ 3, s(struct svr4_sys_open_args),
	    svr4_sys_open },			/* 5 = open */
	{ 1, s(struct sys_close_args),
	    sys_close },			/* 6 = close */
	{ 1, s(struct svr4_sys_wait_args),
	    svr4_sys_wait },			/* 7 = wait */
	{ 2, s(struct svr4_sys_creat_args),
	    svr4_sys_creat },			/* 8 = creat */
	{ 2, s(struct sys_link_args),
	    sys_link },				/* 9 = link */
	{ 1, s(struct sys_unlink_args),
	    sys_unlink },			/* 10 = unlink */
	{ 2, s(struct svr4_sys_execv_args),
	    svr4_sys_execv },			/* 11 = execv */
	{ 1, s(struct sys_chdir_args),
	    sys_chdir },			/* 12 = chdir */
	{ 1, s(struct svr4_sys_time_args),
	    svr4_sys_time },			/* 13 = time */
	{ 3, s(struct svr4_sys_mknod_args),
	    svr4_sys_mknod },			/* 14 = mknod */
	{ 2, s(struct sys_chmod_args),
	    sys_chmod },			/* 15 = chmod */
	{ 3, s(struct sys_chown_args),
	    sys_chown },			/* 16 = chown */
	{ 1, s(struct svr4_sys_break_args),
	    svr4_sys_break },			/* 17 = break */
	{ 2, s(struct svr4_sys_stat_args),
	    svr4_sys_stat },			/* 18 = stat */
	{ 3, s(struct compat_43_sys_lseek_args),
	    compat_43_sys_lseek },		/* 19 = lseek */
	{ 0, 0,
	    sys_getpid },			/* 20 = getpid */
	{ 0, 0,
	    sys_nosys },			/* 21 = unimplemented old_mount */
	{ 0, 0,
	    sys_nosys },			/* 22 = unimplemented System V umount */
	{ 1, s(struct sys_setuid_args),
	    sys_setuid },			/* 23 = setuid */
	{ 0, 0,
	    sys_getuid },			/* 24 = getuid */
	{ 0, 0,
	    sys_nosys },			/* 25 = unimplemented stime */
	{ 0, 0,
	    sys_nosys },			/* 26 = unimplemented ptrace */
	{ 1, s(struct svr4_sys_alarm_args),
	    svr4_sys_alarm },			/* 27 = alarm */
	{ 2, s(struct svr4_sys_fstat_args),
	    svr4_sys_fstat },			/* 28 = fstat */
	{ 0, 0,
	    svr4_sys_pause },			/* 29 = pause */
	{ 2, s(struct svr4_sys_utime_args),
	    svr4_sys_utime },			/* 30 = utime */
	{ 0, 0,
	    sys_nosys },			/* 31 = unimplemented was stty */
	{ 0, 0,
	    sys_nosys },			/* 32 = unimplemented was gtty */
	{ 2, s(struct svr4_sys_access_args),
	    svr4_sys_access },			/* 33 = access */
	{ 1, s(struct svr4_sys_nice_args),
	    svr4_sys_nice },			/* 34 = nice */
	{ 0, 0,
	    sys_nosys },			/* 35 = unimplemented statfs */
	{ 0, 0,
	    sys_sync },				/* 36 = sync */
	{ 2, s(struct svr4_sys_kill_args),
	    svr4_sys_kill },			/* 37 = kill */
	{ 0, 0,
	    sys_nosys },			/* 38 = unimplemented fstatfs */
	{ 3, s(struct svr4_sys_pgrpsys_args),
	    svr4_sys_pgrpsys },			/* 39 = pgrpsys */
	{ 0, 0,
	    sys_nosys },			/* 40 = unimplemented xenix */
	{ 1, s(struct sys_dup_args),
	    sys_dup },				/* 41 = dup */
	{ 0, 0,
	    sys_pipe },				/* 42 = pipe */
	{ 1, s(struct svr4_sys_times_args),
	    svr4_sys_times },			/* 43 = times */
	{ 0, 0,
	    sys_nosys },			/* 44 = unimplemented profil */
	{ 0, 0,
	    sys_nosys },			/* 45 = unimplemented plock */
	{ 1, s(struct sys_setgid_args),
	    sys_setgid },			/* 46 = setgid */
	{ 0, 0,
	    sys_getgid },			/* 47 = getgid */
	{ 2, s(struct svr4_sys_signal_args),
	    svr4_sys_signal },			/* 48 = signal */
#ifdef SYSVMSG
	{ 5, s(struct svr4_sys_msgsys_args),
	    svr4_sys_msgsys },			/* 49 = msgsys */
#else
	{ 0, 0,
	    sys_nosys },			/* 49 = unimplemented msgsys */
#endif
	{ 2, s(struct svr4_sys_sysarch_args),
	    svr4_sys_sysarch },			/* 50 = sysarch */
	{ 0, 0,
	    sys_nosys },			/* 51 = unimplemented acct */
#ifdef SYSVSHM
	{ 4, s(struct svr4_sys_shmsys_args),
	    svr4_sys_shmsys },			/* 52 = shmsys */
#else
	{ 0, 0,
	    sys_nosys },			/* 52 = unimplemented shmsys */
#endif
#ifdef SYSVSEM
	{ 5, s(struct svr4_sys_semsys_args),
	    svr4_sys_semsys },			/* 53 = semsys */
#else
	{ 0, 0,
	    sys_nosys },			/* 53 = unimplemented semsys */
#endif
	{ 3, s(struct svr4_sys_ioctl_args),
	    svr4_sys_ioctl },			/* 54 = ioctl */
	{ 0, 0,
	    sys_nosys },			/* 55 = unimplemented uadmin */
	{ 0, 0,
	    sys_nosys },			/* 56 = unimplemented exch */
	{ 4, s(struct svr4_sys_utssys_args),
	    svr4_sys_utssys },			/* 57 = utssys */
	{ 1, s(struct sys_fsync_args),
	    sys_fsync },			/* 58 = fsync */
	{ 3, s(struct svr4_sys_execve_args),
	    svr4_sys_execve },			/* 59 = execve */
	{ 1, s(struct sys_umask_args),
	    sys_umask },			/* 60 = umask */
	{ 1, s(struct sys_chroot_args),
	    sys_chroot },			/* 61 = chroot */
	{ 3, s(struct svr4_sys_fcntl_args),
	    svr4_sys_fcntl },			/* 62 = fcntl */
	{ 2, s(struct svr4_sys_ulimit_args),
	    svr4_sys_ulimit },			/* 63 = ulimit */
	{ 0, 0,
	    sys_nosys },			/* 64 = unimplemented reserved for unix/pc */
	{ 0, 0,
	    sys_nosys },			/* 65 = unimplemented reserved for unix/pc */
	{ 0, 0,
	    sys_nosys },			/* 66 = unimplemented reserved for unix/pc */
	{ 0, 0,
	    sys_nosys },			/* 67 = unimplemented reserved for unix/pc */
	{ 0, 0,
	    sys_nosys },			/* 68 = unimplemented reserved for unix/pc */
	{ 0, 0,
	    sys_nosys },			/* 69 = unimplemented reserved for unix/pc */
	{ 0, 0,
	    sys_nosys },			/* 70 = obsolete advfs */
	{ 0, 0,
	    sys_nosys },			/* 71 = obsolete unadvfs */
	{ 0, 0,
	    sys_nosys },			/* 72 = obsolete rmount */
	{ 0, 0,
	    sys_nosys },			/* 73 = obsolete rumount */
	{ 0, 0,
	    sys_nosys },			/* 74 = obsolete rfstart */
	{ 0, 0,
	    sys_nosys },			/* 75 = obsolete sigret */
#ifdef COMPAT_SVR4_NCR
	{ 0, 0,
	    svr4_ncr_sys_rdebug },		/* 76 = rdebug */
#else
	{ 0, 0,
	    sys_nosys },			/* 76 = obsolete rdebug */
#endif
	{ 0, 0,
	    sys_nosys },			/* 77 = obsolete rfstop */
	{ 0, 0,
	    sys_nosys },			/* 78 = unimplemented rfsys */
	{ 1, s(struct sys_rmdir_args),
	    sys_rmdir },			/* 79 = rmdir */
	{ 2, s(struct sys_mkdir_args),
	    sys_mkdir },			/* 80 = mkdir */
	{ 3, s(struct svr4_sys_getdents_args),
	    svr4_sys_getdents },		/* 81 = getdents */
	{ 0, 0,
	    sys_nosys },			/* 82 = obsolete libattach */
	{ 0, 0,
	    sys_nosys },			/* 83 = obsolete libdetach */
	{ 0, 0,
	    sys_nosys },			/* 84 = unimplemented sysfs */
	{ 4, s(struct svr4_sys_getmsg_args),
	    svr4_sys_getmsg },			/* 85 = getmsg */
	{ 4, s(struct svr4_sys_putmsg_args),
	    svr4_sys_putmsg },			/* 86 = putmsg */
	{ 3, s(struct sys_poll_args),
	    sys_poll },				/* 87 = poll */
	{ 2, s(struct svr4_sys_lstat_args),
	    svr4_sys_lstat },			/* 88 = lstat */
	{ 2, s(struct sys_symlink_args),
	    sys_symlink },			/* 89 = symlink */
	{ 3, s(struct sys_readlink_args),
	    sys_readlink },			/* 90 = readlink */
	{ 2, s(struct sys_getgroups_args),
	    sys_getgroups },			/* 91 = getgroups */
	{ 2, s(struct sys_setgroups_args),
	    sys_setgroups },			/* 92 = setgroups */
	{ 2, s(struct sys_fchmod_args),
	    sys_fchmod },			/* 93 = fchmod */
	{ 3, s(struct sys_fchown_args),
	    sys_fchown },			/* 94 = fchown */
	{ 3, s(struct svr4_sys_sigprocmask_args),
	    svr4_sys_sigprocmask },		/* 95 = sigprocmask */
	{ 1, s(struct svr4_sys_sigsuspend_args),
	    svr4_sys_sigsuspend },		/* 96 = sigsuspend */
	{ 2, s(struct svr4_sys_sigaltstack_args),
	    svr4_sys_sigaltstack },		/* 97 = sigaltstack */
	{ 3, s(struct svr4_sys_sigaction_args),
	    svr4_sys_sigaction },		/* 98 = sigaction */
	{ 2, s(struct svr4_sys_sigpending_args),
	    svr4_sys_sigpending },		/* 99 = sigpending */
	{ 2, s(struct svr4_sys_context_args),
	    svr4_sys_context },			/* 100 = context */
	{ 0, 0,
	    sys_nosys },			/* 101 = unimplemented evsys */
	{ 0, 0,
	    sys_nosys },			/* 102 = unimplemented evtrapret */
	{ 2, s(struct svr4_sys_statvfs_args),
	    svr4_sys_statvfs },			/* 103 = statvfs */
	{ 2, s(struct svr4_sys_fstatvfs_args),
	    svr4_sys_fstatvfs },		/* 104 = fstatvfs */
	{ 0, 0,
	    sys_nosys },			/* 105 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 106 = unimplemented nfssvc */
	{ 4, s(struct svr4_sys_waitsys_args),
	    svr4_sys_waitsys },			/* 107 = waitsys */
	{ 0, 0,
	    sys_nosys },			/* 108 = unimplemented sigsendsys */
	{ 5, s(struct svr4_sys_hrtsys_args),
	    svr4_sys_hrtsys },			/* 109 = hrtsys */
	{ 0, 0,
	    sys_nosys },			/* 110 = unimplemented acancel */
	{ 0, 0,
	    sys_nosys },			/* 111 = unimplemented async */
	{ 0, 0,
	    sys_nosys },			/* 112 = unimplemented priocntlsys */
	{ 2, s(struct svr4_sys_pathconf_args),
	    svr4_sys_pathconf },		/* 113 = pathconf */
	{ 3, s(struct sys_mincore_args),
	    sys_mincore },			/* 114 = mincore */
	{ 6, s(struct svr4_sys_mmap_args),
	    svr4_sys_mmap },			/* 115 = mmap */
	{ 3, s(struct sys_mprotect_args),
	    sys_mprotect },			/* 116 = mprotect */
	{ 2, s(struct sys_munmap_args),
	    sys_munmap },			/* 117 = munmap */
	{ 2, s(struct svr4_sys_fpathconf_args),
	    svr4_sys_fpathconf },		/* 118 = fpathconf */
	{ 0, 0,
	    sys_vfork },			/* 119 = vfork */
	{ 1, s(struct sys_fchdir_args),
	    sys_fchdir },			/* 120 = fchdir */
	{ 3, s(struct sys_readv_args),
	    sys_readv },			/* 121 = readv */
	{ 3, s(struct sys_writev_args),
	    sys_writev },			/* 122 = writev */
	{ 3, s(struct svr4_sys_xstat_args),
	    svr4_sys_xstat },			/* 123 = xstat */
	{ 3, s(struct svr4_sys_lxstat_args),
	    svr4_sys_lxstat },			/* 124 = lxstat */
	{ 3, s(struct svr4_sys_fxstat_args),
	    svr4_sys_fxstat },			/* 125 = fxstat */
	{ 4, s(struct svr4_sys_xmknod_args),
	    svr4_sys_xmknod },			/* 126 = xmknod */
	{ 0, 0,
	    sys_nosys },			/* 127 = unimplemented clocal */
	{ 2, s(struct svr4_sys_setrlimit_args),
	    svr4_sys_setrlimit },		/* 128 = setrlimit */
	{ 2, s(struct svr4_sys_getrlimit_args),
	    svr4_sys_getrlimit },		/* 129 = getrlimit */
	{ 3, s(struct sys_lchown_args),
	    sys_lchown },			/* 130 = lchown */
	{ 6, s(struct svr4_sys_memcntl_args),
	    svr4_sys_memcntl },			/* 131 = memcntl */
	{ 0, 0,
	    sys_nosys },			/* 132 = unimplemented getpmsg */
	{ 0, 0,
	    sys_nosys },			/* 133 = unimplemented putpmsg */
	{ 2, s(struct sys_rename_args),
	    sys_rename },			/* 134 = rename */
	{ 2, s(struct svr4_sys_uname_args),
	    svr4_sys_uname },			/* 135 = uname */
	{ 1, s(struct sys_setegid_args),
	    sys_setegid },			/* 136 = setegid */
	{ 1, s(struct svr4_sys_sysconfig_args),
	    svr4_sys_sysconfig },		/* 137 = sysconfig */
	{ 2, s(struct sys_adjtime_args),
	    sys_adjtime },			/* 138 = adjtime */
	{ 3, s(struct svr4_sys_systeminfo_args),
	    svr4_sys_systeminfo },		/* 139 = systeminfo */
	{ 0, 0,
	    sys_nosys },			/* 140 = unimplemented */
	{ 1, s(struct sys_seteuid_args),
	    sys_seteuid },			/* 141 = seteuid */
	{ 0, 0,
	    sys_nosys },			/* 142 = unimplemented vtrace */
	{ 0, 0,
	    sys_nosys },			/* 143 = unimplemented fork1 */
	{ 0, 0,
	    sys_nosys },			/* 144 = unimplemented sigtimedwait */
	{ 0, 0,
	    sys_nosys },			/* 145 = unimplemented lwp_info */
	{ 0, 0,
	    sys_nosys },			/* 146 = unimplemented yield */
	{ 0, 0,
	    sys_nosys },			/* 147 = unimplemented lwp_sema_wait */
	{ 0, 0,
	    sys_nosys },			/* 148 = unimplemented lwp_sema_post */
	{ 0, 0,
	    sys_nosys },			/* 149 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 150 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 151 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 152 = unimplemented modctl */
	{ 1, s(struct svr4_sys_fchroot_args),
	    svr4_sys_fchroot },			/* 153 = fchroot */
	{ 2, s(struct svr4_sys_utimes_args),
	    svr4_sys_utimes },			/* 154 = utimes */
	{ 0, 0,
	    svr4_sys_vhangup },			/* 155 = vhangup */
	{ 1, s(struct svr4_sys_gettimeofday_args),
	    svr4_sys_gettimeofday },		/* 156 = gettimeofday */
	{ 2, s(struct sys_getitimer_args),
	    sys_getitimer },			/* 157 = getitimer */
	{ 3, s(struct sys_setitimer_args),
	    sys_setitimer },			/* 158 = setitimer */
	{ 0, 0,
	    sys_nosys },			/* 159 = unimplemented lwp_create */
	{ 0, 0,
	    sys_nosys },			/* 160 = unimplemented lwp_exit */
	{ 0, 0,
	    sys_nosys },			/* 161 = unimplemented lwp_suspend */
	{ 0, 0,
	    sys_nosys },			/* 162 = unimplemented lwp_continue */
	{ 0, 0,
	    sys_nosys },			/* 163 = unimplemented lwp_kill */
	{ 0, 0,
	    sys_nosys },			/* 164 = unimplemented lwp_self */
	{ 0, 0,
	    sys_nosys },			/* 165 = unimplemented lwp_getprivate */
	{ 0, 0,
	    sys_nosys },			/* 166 = unimplemented lwp_setprivate */
	{ 0, 0,
	    sys_nosys },			/* 167 = unimplemented lwp_wait */
	{ 0, 0,
	    sys_nosys },			/* 168 = unimplemented lwp_mutex_unlock */
	{ 0, 0,
	    sys_nosys },			/* 169 = unimplemented lwp_mutex_lock */
	{ 0, 0,
	    sys_nosys },			/* 170 = unimplemented lwp_cond_wait */
	{ 0, 0,
	    sys_nosys },			/* 171 = unimplemented lwp_cond_signal */
	{ 0, 0,
	    sys_nosys },			/* 172 = unimplemented lwp_cond_broadcast */
	{ 4, s(struct svr4_sys_pread_args),
	    svr4_sys_pread },			/* 173 = pread */
	{ 4, s(struct svr4_sys_pwrite_args),
	    svr4_sys_pwrite },			/* 174 = pwrite */
	{ 0, 0,
	    sys_nosys },			/* 175 = unimplemented llseek */
	{ 0, 0,
	    sys_nosys },			/* 176 = unimplemented inst_sync */
	{ 0, 0,
	    sys_nosys },			/* 177 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 178 = unimplemented kaio */
	{ 0, 0,
	    sys_nosys },			/* 179 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 180 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 181 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 182 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 183 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 184 = unimplemented tsolsys */
	{ 4, s(struct svr4_sys_acl_args),
	    svr4_sys_acl },			/* 185 = acl */
	{ 0, 0,
	    sys_nosys },			/* 186 = unimplemented auditsys */
	{ 0, 0,
	    sys_nosys },			/* 187 = unimplemented processor_bind */
	{ 0, 0,
	    sys_nosys },			/* 188 = unimplemented processor_info */
	{ 0, 0,
	    sys_nosys },			/* 189 = unimplemented p_online */
	{ 0, 0,
	    sys_nosys },			/* 190 = unimplemented sigqueue */
	{ 0, 0,
	    sys_nosys },			/* 191 = unimplemented clock_gettime */
	{ 0, 0,
	    sys_nosys },			/* 192 = unimplemented clock_settime */
	{ 0, 0,
	    sys_nosys },			/* 193 = unimplemented clock_getres */
	{ 0, 0,
	    sys_nosys },			/* 194 = unimplemented timer_create */
	{ 0, 0,
	    sys_nosys },			/* 195 = unimplemented timer_delete */
	{ 0, 0,
	    sys_nosys },			/* 196 = unimplemented timer_settime */
	{ 0, 0,
	    sys_nosys },			/* 197 = unimplemented timer_gettime */
	{ 0, 0,
	    sys_nosys },			/* 198 = unimplemented timer_getoverrun */
	{ 0, 0,
	    sys_nosys },			/* 199 = unimplemented nanosleep */
	{ 4, s(struct svr4_sys_facl_args),
	    svr4_sys_facl },			/* 200 = facl */
	{ 0, 0,
	    sys_nosys },			/* 201 = unimplemented door */
	{ 2, s(struct compat_43_sys_setreuid_args),
	    compat_43_sys_setreuid },		/* 202 = setreuid */
	{ 2, s(struct compat_43_sys_setregid_args),
	    compat_43_sys_setregid },		/* 203 = setregid */
	{ 0, 0,
	    sys_nosys },			/* 204 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 205 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 206 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 207 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 208 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 209 = unimplemented */
	{ 0, 0,
	    sys_nosys },			/* 210 = unimplemented signotifywait */
	{ 0, 0,
	    sys_nosys },			/* 211 = unimplemented lwp_sigredirect */
	{ 0, 0,
	    sys_nosys },			/* 212 = unimplemented lwp_alarm */
};

