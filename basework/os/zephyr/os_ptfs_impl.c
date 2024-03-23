/*
 * Copyright 2022 wtcat
 */
#define pr_fmt(fmt) "<os_ptfs>: "fmt

#include <string.h>
#include <errno.h>
#include <assert.h>
#include <init.h>
#include <board_cfg.h>
#include <partition/partition.h>
#include <drivers/nvram_config.h>

#include "basework/dev/disk.h"
#include "basework/dev/ptfs.h"
#include "basework/generic.h"
#include "basework/os/osapi_config.h"
#include "basework/os/osapi_fs.h"
#include "basework/log.h"
#include "basework/malloc.h"

#define READ_BUFFER_SIZE  4096
#define FLASH_ERASE_BLKSZ 4096
#define FLASH_ALIGNED_UP(x) \
    (((x) + FLASH_ERASE_BLKSZ - 1) & ~(FLASH_ERASE_BLKSZ - 1))

struct file {
    struct file_base base;
    struct ptfs_file *fp;
};

#define PTFS &ptfs_context

extern size_t strlcpy (char *dst, const char *src, size_t dsize);

static struct ptfs_context ptfs_context __rte_section(".ram.noinit");;

const char *ptfs_get_filename(const char *name) {
    if (name[0] != '/')
        return NULL;
    const char *p = strchr(name, ':');
    if (!p || p[1] != '/')
        return NULL;
    return p + 2;
}

static int ptfs_impl_open(os_file_t fd, const char *path, int flags) {
    struct file *fp = (struct file *)fd;
    const char *fname = ptfs_get_filename(path);
    if (!fname)
        return -EINVAL;
    fp->fp = ptfile_ll_open(PTFS, fname, flags);
    if (!fp->fp)
        return -1;
    return 0;
}

static int ptfs_impl_close(os_file_t fd) {
    struct file *fp = (struct file *)fd;
    return ptfile_ll_close(PTFS, fp->fp);
}

static int ptfs_impl_ioctl(os_file_t fd, int cmd, void *args) {
    (void) fd;
    (void) cmd;
    (void) args;
    return -ENOSYS;
}

static ssize_t ptfs_impl_read(os_file_t fd, void *buf, size_t len) {
    struct file *fp = (struct file *)fd;
    return ptfile_ll_read(PTFS, fp->fp, buf, len);
}

static ssize_t ptfs_impl_write(os_file_t fd, const void *buf, size_t len) {
    struct file *fp = (struct file *)fd;
    return ptfile_ll_write(PTFS, fp->fp, buf, len);    
}

static int ptfs_impl_flush(os_file_t fd) {
    (void) fd;
    return -ENOSYS;
}

static int ptfs_impl_lseek(os_file_t fd, off_t offset, int whence) {
    struct file *fp = (struct file *)fd;
    return ptfile_ll_seek(PTFS, fp->fp, offset, whence); 
}


static int ptfs_impl_opendir(const char *path, VFS_DIR *dirp) {
    const char *dir = ptfs_get_filename(path);

    if (dir[-1] == '/' && dir[0] == '\0') {
        dirp->data = (void *)0;
        return 0;
    }
    return -EINVAL;
}

static int ptfs_impl_readir(VFS_DIR *dirp, struct vfs_dirent *dirent) {
    const char *pname;
    int idx;

    idx = (int)dirp->data;
    pname = ptfile_get_fname(PTFS, &idx);
    if (pname) {
        dirp->data = (void *)idx;
        dirent->d_type = DT_REG;
        strlcpy(dirent->d_name, pname, sizeof(dirent->d_name));
        return 0;
    }
    return -ENODATA;
}

static int ptfs_impl_closedir(VFS_DIR *dirp) {
    (void) dirp;
    return 0;
}

static int ptfs_impl_rename(os_filesystem_t fs, const char *oldpath, 
    const char *newpath) {
    (void) fs;
    (void) oldpath;
    (void) newpath;
    return -ENOSYS;
}

static int ptfs_impl_ftruncate(os_file_t fd, off_t length) {
    (void) fd;
    (void) length;
    return -ENOSYS;
}

static int ptfs_impl_unlink(os_filesystem_t fs, const char *path) {
    (void) fs;
    (void) path;
    const char *fname = ptfs_get_filename(path);
    if (!fname)
        return -EINVAL;
    return ptfile_ll_unlink(PTFS, fname);
}

static int ptfs_impl_stat(os_filesystem_t fs, const char *name, 
    struct vfs_stat *buf) {
    const char *fname = ptfs_get_filename(name);
    (void) fs;
    return ptfile_ll_stat(PTFS, fname, buf); 
}

static int ptfs_impl_reset(os_filesystem_t fs) {
    (void) fs;
    ptfile_ll_reset(PTFS);
    return 0;
}

static struct file os_files[CONFIG_OS_MAX_FILES] __rte_section(".ram.noinit");
static struct file_class ptfs_class = {
    .open = ptfs_impl_open,
    .close = ptfs_impl_close,
    .ioctl = ptfs_impl_ioctl,
    .read = ptfs_impl_read,
    .write = ptfs_impl_write,
    .flush = ptfs_impl_flush,
    .lseek = ptfs_impl_lseek,
    .truncate = ptfs_impl_ftruncate,
    .opendir = ptfs_impl_opendir,
    .readdir = ptfs_impl_readir,
    .closedir = ptfs_impl_closedir,
    .mkdir = NULL,
    .unlink = ptfs_impl_unlink,
    .stat = ptfs_impl_stat,
    .rename = ptfs_impl_rename,
    .reset = ptfs_impl_reset,
};

#ifdef CONFIG_FW_UPDATE_CHECK
static bool dev_address_updated(void) {
    char bak_mac[16], mac[16];
    int ret;

    ret = nvram_config_get("BT_MAC", mac, 12);
    if (ret <= 0) {
        pr_err("Not found device MAC address\n");
        goto _exit;
    }

    ret = nvram_config_get("BT_MAC_BAK", bak_mac, 12);
    if (ret <= 0) {
        pr_err("Not found MAC address backup\n");
        goto _update;
    }

    if (!strncmp(mac, bak_mac, 12)) {
        pr_notice("The firmware package not updated\n");
        return false;
    }

_update:
    nvram_config_set("BT_MAC_BAK", mac, 12);
_exit:
    return true;
}
#endif /* CONFIG_FW_UPDATE_CHECK */

static int __rte_unused ptfs_impl_register(const struct device *dev) {
    static struct vfs_node ptfs_vfs = {
        .mntpoint = "/PTFS:",
        .vfs = &ptfs_class
    };
    uint32_t offset;
    int err;
    (void) dev;

    const struct partition_entry *parti;
    parti = partition_get_stf_part(STORAGE_ID_NOR, PARTITION_FILE_ID_UDISK);
    if (!parti) {
        pr_err("Not found udisk parition\n");
        return -ENOENT;
    }

    if (parti->size < CONFIG_PTFS_SIZE) {
        pr_err("ptfs partition is too large (%d)\n", CONFIG_PTFS_SIZE);
        return -EINVAL;
    }

    offset = parti->offset;

    memset(os_files, 0, sizeof(os_files));
    memset(&ptfs_context, 0, sizeof(ptfs_context));
    err = ptfile_ll_init(PTFS, CONFIG_SPI_FLASH_NAME, offset);
    if (err)
        return err;
    pr_notice("## partition filesystem registed start(0x%x) size(%d)\n", 
        offset, CONFIG_PTFS_SIZE);
    err = os_obj_initialize(&ptfs_class.robj, os_files, 
        sizeof(os_files), sizeof(os_files[0]));
    assert(err == 0);
    return vfs_register(&ptfs_vfs);
}

#ifndef CONFIG_CARD_READER_APP
SYS_INIT(ptfs_impl_register, APPLICATION, 
    CONFIG_APPLICATION_INIT_PRIORITY);
#endif
