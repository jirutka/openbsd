/*	$OpenBSD: src/sys/miscfs/procfs/procfs_vfsops.c,v 1.2 1996/02/27 08:03:38 niklas Exp $	*/
/*	$NetBSD: procfs_vfsops.c,v 1.25 1996/02/09 22:40:53 christos Exp $	*/

/*
 * Copyright (c) 1993 Jan-Simon Pendry
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Jan-Simon Pendry.
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
 *
 *	@(#)procfs_vfsops.c	8.5 (Berkeley) 6/15/94
 */

/*
 * procfs VFS interface
 */

#include <sys/param.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/syslog.h>
#include <sys/mount.h>
#include <sys/signalvar.h>
#include <sys/vnode.h>
#include <miscfs/procfs/procfs.h>
#include <vm/vm.h>			/* for PAGE_SIZE */

int	procfs_mount __P((struct mount *, char *, caddr_t,
			  struct nameidata *, struct proc *));
int	procfs_start __P((struct mount *, int, struct proc *));
int	procfs_unmount __P((struct mount *, int, struct proc *));
int	procfs_quotactl __P((struct mount *, int, uid_t, caddr_t,
			     struct proc *));
int	procfs_statfs __P((struct mount *, struct statfs *, struct proc *));
int	procfs_sync __P((struct mount *, int, struct ucred *, struct proc *));
int	procfs_vget __P((struct mount *, ino_t, struct vnode **));
int	procfs_fhtovp __P((struct mount *, struct fid *, struct mbuf *,
			   struct vnode **, int *, struct ucred **));
int	procfs_vptofh __P((struct vnode *, struct fid *));
/*
 * VFS Operations.
 *
 * mount system call
 */
/* ARGSUSED */
int
procfs_mount(mp, path, data, ndp, p)
	struct mount *mp;
	char *path;
	caddr_t data;
	struct nameidata *ndp;
	struct proc *p;
{
	size_t size;

	if (UIO_MX & (UIO_MX-1)) {
		log(LOG_ERR, "procfs: invalid directory entry size");
		return (EINVAL);
	}

	if (mp->mnt_flag & MNT_UPDATE)
		return (EOPNOTSUPP);

	mp->mnt_flag |= MNT_LOCAL;
	mp->mnt_data = 0;
	getnewfsid(mp, makefstype(MOUNT_PROCFS));

	(void) copyinstr(path, mp->mnt_stat.f_mntonname, MNAMELEN, &size);
	bzero(mp->mnt_stat.f_mntonname + size, MNAMELEN - size);
	bzero(mp->mnt_stat.f_mntfromname, MNAMELEN);
	bcopy("procfs", mp->mnt_stat.f_mntfromname, sizeof("procfs"));
	return (0);
}

/*
 * unmount system call
 */
int
procfs_unmount(mp, mntflags, p)
	struct mount *mp;
	int mntflags;
	struct proc *p;
{
	int error;
	extern int doforce;
	int flags = 0;

	if (mntflags & MNT_FORCE) {
		/* procfs can never be rootfs so don't check for it */
		if (!doforce)
			return (EINVAL);
		flags |= FORCECLOSE;
	}

	if ((error = vflush(mp, 0, flags)) != 0)
		return (error);

	return (0);
}

int
procfs_root(mp, vpp)
	struct mount *mp;
	struct vnode **vpp;
{

	return (procfs_allocvp(mp, vpp, 0, Proot));
}

/* ARGSUSED */
int
procfs_start(mp, flags, p)
	struct mount *mp;
	int flags;
	struct proc *p;
{

	return (0);
}

/*
 * Get file system statistics.
 */
int
procfs_statfs(mp, sbp, p)
	struct mount *mp;
	struct statfs *sbp;
	struct proc *p;
{

#ifdef COMPAT_09
	sbp->f_type = 10;
#else
	sbp->f_type = 0;
#endif
	sbp->f_bsize = PAGE_SIZE;
	sbp->f_iosize = PAGE_SIZE;
	sbp->f_blocks = 1;	/* avoid divide by zero in some df's */
	sbp->f_bfree = 0;
	sbp->f_bavail = 0;
	sbp->f_files = maxproc;			/* approx */
	sbp->f_ffree = maxproc - nprocs;	/* approx */
	if (sbp != &mp->mnt_stat) {
		bcopy(&mp->mnt_stat.f_fsid, &sbp->f_fsid, sizeof(sbp->f_fsid));
		bcopy(mp->mnt_stat.f_mntonname, sbp->f_mntonname, MNAMELEN);
		bcopy(mp->mnt_stat.f_mntfromname, sbp->f_mntfromname, MNAMELEN);
	}
	strncpy(sbp->f_fstypename, mp->mnt_op->vfs_name, MFSNAMELEN);
	return (0);
}

/*ARGSUSED*/
int
procfs_quotactl(mp, cmds, uid, arg, p)
	struct mount *mp;
	int cmds;
	uid_t uid;
	caddr_t arg;
	struct proc *p;
{

	return (EOPNOTSUPP);
}

/*ARGSUSED*/
int
procfs_sync(mp, waitfor, uc, p)
	struct mount *mp;
	int waitfor;
	struct ucred *uc;
	struct proc *p;
{

	return (0);
}

/*ARGSUSED*/
int
procfs_vget(mp, ino, vpp)
	struct mount *mp;
	ino_t ino;
	struct vnode **vpp;
{

	return (EOPNOTSUPP);
}

/*ARGSUSED*/
int
procfs_fhtovp(mp, fhp, mb, vpp, what, anon)
	struct mount *mp;
	struct fid *fhp;
	struct mbuf *mb;
	struct vnode **vpp;
	int *what;
	struct ucred **anon;
{

	return (EINVAL);
}

/*ARGSUSED*/
int
procfs_vptofh(vp, fhp)
	struct vnode *vp;
	struct fid *fhp;
{

	return (EINVAL);
}

void
procfs_init()
{
}

struct vfsops procfs_vfsops = {
	MOUNT_PROCFS,
	procfs_mount,
	procfs_start,
	procfs_unmount,
	procfs_root,
	procfs_quotactl,
	procfs_statfs,
	procfs_sync,
	procfs_vget,
	procfs_fhtovp,
	procfs_vptofh,
	procfs_init,
};
