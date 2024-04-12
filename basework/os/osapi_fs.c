/*
 * Copyright 2022 wtcat
 */
#define pr_fmt(fmt) "<vfs>: " fmt
#define VFS_CHECKER_ENABLE 0
#include <assert.h>
#include <string.h>

#include "basework/log.h"
#include "basework/os/osapi.h"
#include "basework/os/osapi_config.h"
#include "basework/os/osapi_fs.h"

#if VFS_CHECKER_ENABLE
#define FD2VFS(fd, vfs)                                                                  \
	do {                                                                                 \
		assert(fd != NULL);                                                              \
		(vfs) = *(void **)(fd);                                                          \
        assert(vfs != NULL);                                                             \
	} while (0)
#else 
#define FD2VFS(fd, vfs) (vfs) = *(void **)(fd)
#endif /* VFS_CHECKER_ENABLE */

#define VFSSETFD(fd, vfs) *(void **)(fd) = (void *)vfs

static struct file_class *vfs_list;

static struct file_class *vfs_match(const char *path) {
	struct file_class *vfs = vfs_list;
	struct file_class *best_matched = NULL;

#if !defined(_WIN32)
	const char *pend;
	if (path[0] != '/' || !(pend = strchr(path + 1, '/'))) {
		pr_err("invalid file path(%s)\n", path);
		return NULL;
	}
#endif
	while (vfs != NULL) {
		if (!vfs->mntpoint) {
			best_matched = vfs;
		}
#if !defined(_WIN32)
		else if (!strncmp(vfs->mntpoint, path, pend - path)) {
			best_matched = vfs;
			break;
		}
#endif
		vfs = vfs->next;
	}

	return best_matched;
}

int vfs_open(os_file_t *fd, const char *path, int flags) {
	struct file_class *vfs;
	os_file_t fp;
	int err;

	if (fd == NULL || path == NULL)
		return -EINVAL;

	if (!(vfs = vfs_match(path)))
		return -EINVAL;

	fp = os_obj_allocate(&vfs->avalible_fds);
	assert(fp != NULL);

	VFSSETFD(fp, vfs);
	err = vfs->open(fp, path, flags);
	if (err) {
		pr_err("Open file failed(%d)\n", err);
		os_obj_free(&vfs->avalible_fds, fp);
		return err;
	}

	*fd = fp;
	return 0;
}

int vfs_close(os_file_t fd) {
	struct file_class *vfs;
	int err;

    FD2VFS(fd, vfs);
	err = vfs->close(fd);
	if (err) {
		pr_err("File close failed (%d)\n", err);
		return err;
	}
	os_obj_free(&vfs->avalible_fds, fd);
	return 0;
}

int vfs_ioctl(os_file_t fd, int cmd, void *args) {
	struct file_class *vfs;

    FD2VFS(fd, vfs);
	if (vfs->ioctl)
		return vfs->ioctl(fd, cmd, args);

	return -ENOSYS;
}

ssize_t vfs_read(os_file_t fd, void *buf, size_t len) {
	struct file_class *vfs;

	if (buf == NULL)
		return -EINVAL;

	if (len == 0)
		return 0;

    FD2VFS(fd, vfs);
	return vfs->read(fd, buf, len);
}

ssize_t vfs_write(os_file_t fd, const void *buf, size_t len) {
	struct file_class *vfs;

	if (buf == NULL)
		return -EINVAL;

	if (len == 0)
		return 0;

    FD2VFS(fd, vfs);
	return vfs->write(fd, buf, len);
}

off_t vfs_tell(os_file_t fd) {
	struct file_class *vfs;

    FD2VFS(fd, vfs);
    if (vfs->tell)
	    return vfs->tell(fd);
    
    return -ENOSYS;
}

int vfs_flush(os_file_t fd) {
	struct file_class *vfs;

    FD2VFS(fd, vfs);
	if (vfs->flush)
		return vfs->flush(fd);

	return -ENOSYS;
}

int vfs_lseek(os_file_t fd, off_t offset, int whence) {
	struct file_class *vfs;

    FD2VFS(fd, vfs);
	return vfs->lseek(fd, offset, whence);
}

int vfs_getdents(os_file_t fd, struct dirent *dirp, size_t nbytes) {
	struct file_class *vfs;

	if (dirp == NULL)
		return -EINVAL;

    FD2VFS(fd, vfs);
	if (vfs->getdents)
		return vfs->getdents(fd, dirp, nbytes);

	return -ENOSYS;
}

int vfs_ftruncate(os_file_t fd, off_t length) {
	struct file_class *vfs;

    FD2VFS(fd, vfs);
	if (vfs->truncate)
		return vfs->truncate(fd, length);

	return -ENOSYS;
}

int vfs_opendir(const char *path, VFS_DIR *dirp) {
	struct file_class *vfs;
	os_file_t fp;
	int err;

	if (path == NULL || dirp == NULL)
		return -EINVAL;

	if (!(vfs = vfs_match(path)))
		return -EINVAL;

	fp = os_obj_allocate(&vfs->avalible_fds);
	assert(fp != NULL);

	VFSSETFD(fp, vfs);
	dirp->fd = fp;

	err = vfs->opendir(path, dirp);
	if (err) {
		pr_err("Open directory failed(%d)\n", err);
		os_obj_free(&vfs->avalible_fds, fp);
		return err;
	}

	return 0;
}

int vfs_readdir(VFS_DIR *dirp, struct vfs_dirent *dirent) {
	struct file_class *vfs;

	if (dirp == NULL || dirent == NULL)
		return -EINVAL;

    FD2VFS(dirp->fd, vfs);
	if (vfs->readdir)
		return vfs->readdir(dirp, dirent);

	return -ENOSYS;
}

int vfs_closedir(VFS_DIR *dirp) {
	struct file_class *vfs;
	int err;

	if (dirp == NULL)
		return -EINVAL;

	FD2VFS(dirp->fd, vfs);
	err = vfs->closedir(dirp);
	if (err) {
		pr_err("Directory close failed (%d)\n", err);
		return err;
	}

	os_obj_free(&vfs->avalible_fds, dirp->fd);
	return 0;
}

int vfs_rename(const char *oldpath, const char *newpath) {
	struct file_class *vfs;

	if (oldpath == NULL || newpath == NULL)
		return -EINVAL;

	// TODO: check newpath
	if (!(vfs = vfs_match(oldpath)))
		return -EINVAL;

	if (vfs->rename)
	    return vfs->rename(NULL, oldpath, newpath);

    return -ENOSYS;
}

int vfs_mkdir(const char *path) {
	struct file_class *vfs;

	if (path == NULL)
		return -EINVAL;

	if (!(vfs = vfs_match(path)))
		return -EINVAL;

	if (vfs->mkdir)
	    return vfs->mkdir(NULL, path);

    return -ENOSYS;
}

int vfs_unlink(const char *path) {
	struct file_class *fp;

	if (path == NULL)
		return -EINVAL;

	if (!(fp = vfs_match(path)))
		return -EINVAL;

	if (fp->unlink)
	    return fp->unlink(NULL, path);

    return -ENOSYS;
}

int vfs_stat(const char *path, struct vfs_stat *buf) {
	struct file_class *fp;

	if (path == NULL || buf == NULL)
		return -EINVAL;

	if (!(fp = vfs_match(path)))
		return -EINVAL;

	if (fp->stat)
	    return fp->stat(NULL, path, buf);

    return -ENOSYS;
}

int vfs_sync(void) {
	struct file_class *vfs = vfs_list;
	int err = 0;

	while (vfs != NULL) {
		if (vfs->fssync)
			err |= vfs->fssync(NULL);
		vfs = vfs->next;
	}
	return err;
}

int vfs_dir_foreach(const char *path, bool (*iterator)(struct vfs_dirent *dirent, void *),
					void *arg) {
	struct vfs_dirent dirent;
	VFS_DIR dirp;
	int err;

	if (!iterator)
		return -EINVAL;

	err = vfs_opendir(path, &dirp);
	if (err)
		return err;

	while (!(err = vfs_readdir(&dirp, &dirent))) {
		if (iterator(&dirent, arg))
			break;
	}

	return vfs_closedir(&dirp);
}

void *vfs_mmap(os_file_t fd, size_t *size) {
	struct file_class *vfs;

    FD2VFS(fd, vfs);
	if (vfs->mmap)
		return vfs->mmap(fd, size);

	return NULL;
}

int vfs_reset(const char *mpt) {
	struct file_class *vfs;

	if (mpt == NULL)
		return -EINVAL;

	if (!(vfs = vfs_match(mpt)))
		return -EINVAL;

	if (vfs->reset)
		return vfs->reset(NULL);

	return -ENOSYS;
}

int vfs_register(struct file_class *cls) {
	struct file_class *vfs = vfs_list;
	int err;

	if (cls == NULL || cls->read == NULL || cls->open == NULL || cls->close == NULL ||
		cls->lseek == NULL) {
		pr_err("invalid vfs node\n");
		err = -EINVAL;
		goto _out;
	}

	if (cls->fds_buffer == NULL) {
		pr_err("No file descriptor buffer\n");
		err = -EINVAL;
		goto _out;
	}

	if (cls->fds_size < cls->fd_size || cls->fd_size == 0 || cls->fds_size == 0) {
		pr_err("Invalid file descriptor parameters\n");
		err = -EINVAL;
		goto _out;
	}

	if (cls->mntpoint) {
		if (cls->mntpoint[0] != '/' || strchr(cls->mntpoint + 1, '/')) {
			pr_err("invalid vfs mount point(%s)\n", cls->mntpoint);
			err = -EINVAL;
			goto _out;
		}
	}

	while (vfs != NULL) {
		if (cls->mntpoint == vfs->mntpoint || !strcmp(cls->mntpoint, vfs->mntpoint)) {
			pr_err("vfs node has exist\n");
			err = -EEXIST;
			goto _out;
		}
		vfs = vfs->next;
	}

	err = os_obj_initialize(&cls->avalible_fds, cls->fds_buffer, cls->fds_size,
							cls->fd_size);
	if (!err) {
		cls->next = vfs_list;
		vfs_list = cls;
	}

_out:
	assert(err == 0);
	return err;
}