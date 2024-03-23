/*
 * Copyright 2022 wtcat
 */
#define pr_fmt(fmt) "<vfs>: "fmt
#include <assert.h>
#include <string.h>

#include "basework/os/osapi_config.h"
#include "basework/os/osapi_fs.h"
#include "basework/os/osapi.h"
#include "basework/log.h"

static struct vfs_node *vfs_list;

static struct file_class *vfs_match(const char *path) {
    struct vfs_node *vn = vfs_list;
    struct file_class *best_matched = NULL;

#if !defined(_WIN32)
    const char* pend;
    if (path[0] != '/' || !(pend = strchr(path + 1, '/'))) {
        pr_err("invalid file path(%s)\n", path);
        return NULL;
    }
#endif
    while (vn) {
        if (!vn->mntpoint) {
            best_matched = vn->vfs;
        } 
#if !defined(_WIN32)
        else if (!strncmp(vn->mntpoint, path, pend-path)) {
            best_matched = vn->vfs;
            break;
        }
#endif
        vn = vn->next;
    }
    return best_matched;
}

int vfs_open(os_file_t *fd, const char *path, int flags) {
    struct file_class *f;
    struct file_base *fp;
    int err;

    if (fd == NULL || path == NULL)
        return -EINVAL;

    if (!(f = vfs_match(path)))
        return -EINVAL;

    fp = os_obj_allocate(&f->robj);
    assert(fp != NULL);

    fp->f_class = f;
    err = fp->f_class->open(fp, path, flags);
    if (err) {
        pr_err("Open file failed(%d)\n", err);
        os_obj_free(&f->robj, fp);
        return err;
    }

    *fd = fp;
    return 0;
}

int vfs_close(os_file_t fd) {
    struct file_base *fp = (struct file_base *)fd;
    int err;

    assert(fp != NULL);
    err = fp->f_class->close(fd);
    if (err) {
        pr_err("File close failed (%d)\n", err);
        return err;
    }
    os_obj_free(&fp->f_class->robj, fd);
    return 0;
}

int vfs_ioctl(os_file_t fd, int cmd, void *args) {
    struct file_base *fp = (struct file_base *)fd;

    assert(fp != NULL);
    assert(fp->f_class != NULL);
    assert(fp->f_class->ioctl != NULL);
    return fp->f_class->ioctl(fd, cmd, args);
}

ssize_t vfs_read(os_file_t fd, void *buf, size_t len) {
    struct file_base *fp = (struct file_base *)fd;
    assert(fp != NULL);

    if (buf == NULL)
        return -EINVAL;
    if (len == 0)
        return 0;
    return fp->f_class->read(fd, buf, len);
}

ssize_t vfs_write(os_file_t fd, const void *buf, size_t len) {
    struct file_base *fp = (struct file_base *)fd;
    assert(fp != NULL);
    assert(fp->f_class->write != NULL);
    if (buf == NULL)
        return -EINVAL;
    if (len == 0)
        return 0;
    return fp->f_class->write(fd, buf, len);
}

off_t vfs_tell(os_file_t fd) {
    struct file_base *fp = (struct file_base *)fd;
    assert(fp != NULL);
    assert(fp->f_class != NULL);
    assert(fp->f_class->tell != NULL);

    return fp->f_class->tell(fd);
}

int vfs_flush(os_file_t fd) {
    struct file_base *fp = (struct file_base *)fd;
    assert(fp != NULL);
    assert(fp->f_class != NULL);
    assert(fp->f_class->flush != NULL);

    return fp->f_class->flush(fd);
}

int vfs_lseek(os_file_t fd, off_t offset, int whence) {
    struct file_base *fp = (struct file_base *)fd;
    assert(fp != NULL);
    return fp->f_class->lseek(fd, offset, whence);
}

int vfs_getdents(os_file_t fd, struct dirent *dirp, size_t nbytes) {
    struct file_base *fp = (struct file_base *)fd;
    assert(fp != NULL);
    if (dirp == NULL)
        return -EINVAL;

    assert(fp->f_class != NULL);
    assert(fp->f_class->getdents != NULL);

    return fp->f_class->getdents(fd, dirp, nbytes);
}

int vfs_ftruncate(os_file_t fd, off_t length) {
    struct file_base *fp = (struct file_base *)fd;
    assert(fp != NULL);
    assert(fp->f_class != NULL);
    assert(fp->f_class->truncate != NULL);

    return fp->f_class->truncate(fd, length);
}

int vfs_opendir(const char *path, VFS_DIR *dirp) {
    struct file_class *f;
    struct file_base *fp;
    int err;

    if (path == NULL || dirp == NULL)
        return -EINVAL;

    if (!(f = vfs_match(path)))
        return -EINVAL;

    fp = os_obj_allocate(&f->robj);
    assert(fp != NULL);
    fp->f_class = f;
    dirp->fd = fp;
    err = fp->f_class->opendir(path, dirp);
    if (err) {
        pr_err("Open directory failed(%d)\n", err);
        os_obj_free(&f->robj, fp);
        return err;
    }
    
    return 0;
}

int vfs_readdir(VFS_DIR *dirp, struct vfs_dirent *dirent) {
    struct file_base *fp;

    if (dirp == NULL || dirent == NULL)
        return -EINVAL;

    fp = (struct file_base *)dirp->fd;
    assert(fp->f_class);
    assert(fp->f_class->readdir);
    return fp->f_class->readdir(dirp, dirent);
}

int vfs_closedir(VFS_DIR *dirp) {
    struct file_base *fp;
    int err;

    if (dirp == NULL)
        return -EINVAL;

    fp = (struct file_base *)dirp->fd;
    assert(fp->f_class);
    assert(fp->f_class->closedir);
    err = fp->f_class->closedir(dirp);
    if (err) {
        pr_err("Directory close failed (%d)\n", err);
        return err;
    }
    os_obj_free(&fp->f_class->robj, dirp->fd);
    return 0;
}

int vfs_rename(const char *oldpath, const char *newpath) {
    struct file_class *fp;
    if (oldpath == NULL || newpath == NULL)
        return -EINVAL;

    //TODO: check newpath
    if (!(fp = vfs_match(oldpath)))
        return -EINVAL;
    assert(fp->rename != NULL);
    return fp->rename(NULL, oldpath, newpath);
}

int vfs_mkdir(const char *path) {
    struct file_class *fp;
    if (path == NULL)
        return -EINVAL;
    if (!(fp = vfs_match(path)))
        return -EINVAL;
    assert(fp->mkdir != NULL);
    return fp->mkdir(NULL, path);
}

int vfs_unlink(const char *path) {
    struct file_class *fp;
    if (path == NULL)
        return -EINVAL;

    if (!(fp = vfs_match(path)))
        return -EINVAL;
    assert(fp->unlink != NULL);
    return fp->unlink(NULL, path);
}

int vfs_stat(const char *path, struct vfs_stat *buf) {
    struct file_class *fp;
    if (path == NULL || buf == NULL)
        return -EINVAL;
    if (!(fp = vfs_match(path)))
        return -EINVAL;
    assert(fp->stat != NULL);
    return fp->stat(NULL, path, buf);
}

int vfs_sync(void) {
    struct vfs_node *vn = vfs_list;
    int err = 0;

    while (vn) {
        if (vn->vfs->fssync)
            err |= vn->vfs->fssync(NULL);
        vn = vn->next;
    }
    return err;
}

int vfs_dir_foreach(const char *path, 
    bool (*iterator)(struct vfs_dirent *dirent, void *), 
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
    struct file_base *fp = (struct file_base *)fd;
    assert(fp != NULL);
    assert(fp->f_class != NULL);

    if (fp->f_class->mmap)
        return fp->f_class->mmap(fd, size);
    return NULL;
}

int vfs_reset(const char *mpt) {
    struct file_class *f;

    if (mpt == NULL)
        return -EINVAL;

    if (!(f = vfs_match(mpt)))
        return -EINVAL;

    if (f->reset)
        return f->reset(NULL);
    return -ENOSYS;
}

int vfs_register(struct vfs_node *vnode) {
    struct vfs_node *vn = vfs_list;

    if (!vnode || !vnode->vfs) {
        pr_err("invalid vfs node\n");
        return -EINVAL;
    }
    if (vnode->mntpoint) {
        if (vnode->mntpoint[0] != '/' ||
            strchr(vnode->mntpoint + 1, '/')) {
            pr_err("invalid vfs mount point(%s)\n", vnode->mntpoint);
            return -EINVAL;
        }
    }
    while (vn) {
        if (vnode->mntpoint == vn->mntpoint ||
            !strcmp(vn->mntpoint, vnode->mntpoint)) {
            pr_err("vfs node has exist\n");
            return -EEXIST;
        }
        vn = vn->next;
    }
    assert(os_obj_ready(&vnode->vfs->robj));
    assert(vnode->vfs->read != NULL);
    assert(vnode->vfs->open != NULL);
    assert(vnode->vfs->close != NULL);
    assert(vnode->vfs->lseek != NULL);
    vnode->next = vfs_list;
    vfs_list = vnode;
    return 0;
}