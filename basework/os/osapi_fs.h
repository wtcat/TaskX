/*
 * Copyright 2022 wtcat
 *
 * Borrowed from rt-thread
 */
#ifndef BASEWORK_OS_OSAPI_FS_H_
#define BASEWORK_OS_OSAPI_FS_H_

#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>
#include <sys/types.h>

#include "basework/os/osapi_obj.h"

#ifdef __cplusplus
extern "C"{
#endif

#ifdef _MSC_VER
	#define __ssize_t_defined
    typedef long int ssize_t;
#endif

#ifndef MAX_FILE_NAME
#define MAX_FILE_NAME 256
#endif

typedef void *os_filesystem_t;
typedef void *os_file_t;

struct stat;
struct dirent;

#define VFS_O_RDONLY 0
#define VFS_O_WRONLY 1
#define VFS_O_RDWR   2
#define VFS_O_MASK   3

#define VFS_O_CREAT  0x0200
#define VFS_O_APPEND 0x0008


#define VFS_SEEK_SET        0  /* Seek from beginning of file.  */
#define VFS_SEEK_CUR        1  /* Seek from current position.  */
#define VFS_SEEK_END        2  /* Set file pointer to EOF plus "offset" */

/* 
 * File I/O control type 
 */
#define VFS_IOSET_FILESIZE  0x0100

struct vfs_stat {
	unsigned long st_size; /* File size */
	unsigned long st_blksize;
	unsigned long st_blocks;
};

struct vfs_dirent {
  uint8_t d_type; /* Type of file */
#define DT_REG 0  /* file type */
#define DT_DIR 1  /* directory type*/
  char d_name[MAX_FILE_NAME + 1];  /* File name */
};
#undef MAX_FILE_NAME

typedef struct {
  os_file_t fd;
  struct vfs_dirent entry;
  void *data;
} VFS_DIR;

struct file_class {
    /* File operations */
    int (*open)     (os_file_t fd, const char *path, int flags);
    int (*close)    (os_file_t fd);
    int (*ioctl)    (os_file_t fd, int cmd, void *args);
    ssize_t (*read) (os_file_t fd, void *buf, size_t count);
    ssize_t (*write)(os_file_t fd, const void *buf, size_t count);
    int (*flush)    (os_file_t fd);
    int (*lseek)    (os_file_t fd, off_t offset, int whence);
    int (*getdents) (os_file_t fd, struct dirent *dirp, uint32_t count);
    int (*truncate) (os_file_t fd, off_t length);
    off_t (*tell)   (os_file_t fd);
    // int (*poll)     (os_file_t *fd, struct rt_pollreq *req);

    /* Directory operations */
    int (*opendir)(const char *path, VFS_DIR *dirp);
    int (*readdir)(VFS_DIR *dirp, struct vfs_dirent *entry);
    int (*closedir)(VFS_DIR *dirp);

    /* XIP map*/
    void *(*mmap)(os_file_t fd, size_t *size);

    /* filesystem operations */
    int (*fssync)   (os_filesystem_t fs);
    int (*mkdir)    (os_filesystem_t fs, const char *pathname);
    int (*unlink)   (os_filesystem_t fs, const char *pathname);
    int (*stat)     (os_filesystem_t fs, const char *filename, struct vfs_stat *buf);
    int (*rename)   (os_filesystem_t fs, const char *oldpath, const char *newpath);

    int (*reset)   (os_filesystem_t fs);

    /* Resource object */
    struct os_robj robj;
};

struct file_base {
    struct file_class *f_class;
    // unsigned int flags;
};

struct vfs_node {
    struct file_class *vfs;
    struct vfs_node *next;
    const char *mntpoint;
};

/*
 * vfs_open - Open or create a regular file
 *
 * @fd: file descriptor
 * @path: file path
 * @flags: open mode (VFS_O_XXXXXX)
 * return 0 if success
 */
int vfs_open(os_file_t *fd, const char *path, int flags);

/*
 * vfs_close - Close a regular file
 *
 * @fd: file descritpor
 * return 0 if success
 */
int vfs_close(os_file_t fd);

/**/
int vfs_ioctl(os_file_t fd, int cmd, void *args);

/*
 * vfs_read - Read data from file
 *
 * @fd: file descriptor
 * @buf: buffer pointer
 * @len: request size
 * return read bytes if success
 */
ssize_t vfs_read(os_file_t fd, void *buf, size_t len);

/*
 * vfs_write - Write data to file
 *
 * @fd: file descriptor
 * @buf: buffer pointer
 * @len: buffer size
 * return writen bytes if success
 */
ssize_t vfs_write(os_file_t fd, const void *buf, size_t len);

/*
 * vfs_tell - Obtain the position of file pointer 
 *
 * @fd: file descriptor
 * return the offset of file pointer 
 */
off_t vfs_tell(os_file_t fd);

/*
 * vfs_flush - Sync file cache to disk
 *
 * @fd: file descriptor
 * return 0 if success
 */
int vfs_flush(os_file_t fd);

/*
 * vfs_lseek - Set offset for file pointer 
 *
 * @fd: file descriptor
 * @offset: file offset
 * @whence: base position
 * return 0 if success
 */
int vfs_lseek(os_file_t fd, off_t offset, int whence);


/*
 * vfs_ftruncate - Change file size
 *
 * @fd: file descriptor
 * @length: the new size for file
 * return 0 if success
 */
int vfs_ftruncate(os_file_t fd, off_t length);

/*
 * vfs_opendir - Open a directory
 *
 * @path: directory path
 * @dirp: directory object pointer
 * return 0 if success
 */
int vfs_opendir(const char *path, VFS_DIR *dirp);

/*
 * vfs_readdir - Read a directory
 *
 * @dirp: directory object pointer
 * @entry: directory content pointer
 * return 0 if success
 */
int vfs_readdir(VFS_DIR *dirp, struct vfs_dirent *entry);

/*
 * vfs_closedir - Close a directory
 *
 * @dirp: directory object pointer
 * return 0 if success
 */
int vfs_closedir(VFS_DIR *dirp);

/*
 * vfs_mkdir - Create a directory
 *
 * @path: directory path
 * return 0 if success
 */
int vfs_mkdir(const char *path);

/*
 * vfs_unlink - Delete a file or directory
 *
 * @path: directory path
 * return 0 if success
 */
int vfs_unlink(const char *path);

/*
 * vfs_rename - Rename a file
 *
 * @oldpath: current name
 * @newpath: new name
 * return 0 if success
 */
int vfs_rename(const char *oldpath, const char *newpath);

/*
 * vfs_stat - Get file status
 *
 * @path: file path
 * @buf: status handle
 * return 0 if success
 */
int vfs_stat(const char *path, struct vfs_stat *buf);

/*
 * vfs_dir_foreach - Iterate through the file directory
 *
 * @path: file path
 * @iterator: user callback (if need break and return true)
 * @arg: user parameter
 * return 0 if success
 */
int vfs_dir_foreach(const char *path, 
    bool (*iterator)(struct vfs_dirent *dirent, void *), 
    void *arg);

/*
 * vfs_sync - Flush filesystem cache to disk
 *
 * return 0 if success
 */
int vfs_sync(void);

/*
 * vfs_reset - Reset filesystem data 
 * Warnning: This operation will cause all files to be deleted
 *
 * @mpt: mount point
 * return 0 if success
 */
int vfs_reset(const char *mpt);

/*
 * vfs_mmap - Mapping file to memory address
 *
 * @fd: file descriptor
 * @size: mapped size
 * return memory address if success
 */
void *vfs_mmap(os_file_t fd, size_t *size);

/*
 * vfs_register - Register a filesystem
 */
int vfs_register(struct vfs_node *vnode);

#ifdef __cplusplus
}
#endif
#endif /* BASEWORK_OS_OSAPI_FS_H_ */