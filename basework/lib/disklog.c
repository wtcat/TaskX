/*
 * Copyright 2022 wtcat
 */
#include <assert.h>
#include <stdbool.h>
#include <errno.h>
#ifdef __ZEPHYR__
#include <posix/pthread.h>
#else
#include <pthread.h>
#endif

#include "basework/dev/disk.h"
#include "basework/dev/partition.h"
#include "basework/dev/buffer_io.h"
#include "basework/lib/printer.h"
#include "basework/lib/disklog.h"
#include "basework/log.h"
#include "basework/minmax.h"
#include "basework/malloc.h"
#include "basework/system.h"

struct disk_log {
    uint32_t magic;
#define DISKLOG_MAGIC 0xabdcebcf
    uint32_t start;
    uint32_t end;
    uint32_t size;
    uint32_t wr_ofs;
    uint32_t rd_ofs;
    uint32_t d_size;
};

struct disklog_ctx {
    struct disk_log file;
    pthread_mutex_t mtx;
    os_timer_t timer;
    struct buffered_io *bio;
    uint32_t offset;
    uint32_t len;
    size_t blksz;
    struct observer_base obs;
    bool upload_pending;
    bool read_locked;
    bool log_dirty;
    bool panic;
};


#ifndef MIN
#define MIN(a, b) ((a) < (b)? (a): (b))
#endif

#define DISLOG_SWAP_PERIOD (3600 * 1000) //1 hour

#define MTX_TRYLOCK() pthread_mutex_trylock(&logctx.mtx)
#define MTX_LOCK()    pthread_mutex_lock(&logctx.mtx)
#define MTX_UNLOCK()  pthread_mutex_unlock(&logctx.mtx)
#define MTX_INIT()    pthread_mutex_init(&logctx.mtx, NULL)
#define MTX_UNINIT()  pthread_mutex_destroy(&logctx.mtx)

static struct disklog_ctx logctx;

static void disklog_reset_locked(void) {
    struct disk_log *filp = &logctx.file;
    size_t blksz = logctx.blksz;
    
    filp->magic  = DISKLOG_MAGIC;
    filp->start  = blksz;
    filp->size   = ((logctx.len - blksz) / blksz) * blksz;
    filp->end    = filp->start + filp->size;
    filp->rd_ofs = filp->start;
    filp->wr_ofs = filp->start;
    filp->d_size = 0;
}

static void disklog_sync(void) {
    if (logctx.log_dirty) {
        MTX_LOCK();
        if (logctx.log_dirty) {
            logctx.log_dirty = false;
            disklog_flush();
        }
        MTX_UNLOCK();
    }
}

static int disklog_sync_listen(struct observer_base *nb,
	unsigned long action, void *data) {
    (void) data;
    (void) nb;
    (void) action;
    disklog_sync();
    return 0;
}

static void disklog_swap(os_timer_t timer, void *arg) {
    (void) arg;
    disklog_sync();
    os_timer_mod(timer, DISLOG_SWAP_PERIOD);
}

void disklog_reset(void) {
    MTX_LOCK();
    logctx.read_locked = false;
    disklog_reset_locked();
    MTX_UNLOCK();
}

void disklog_flush(void) {
	assert(logctx.bio != NULL);
	if (logctx.bio->panic) {
		disk_device_erase(logctx.bio->dd, logctx.offset, logctx.blksz);
		disk_device_write(logctx.bio->dd, &logctx.file, 
			sizeof(logctx.file), logctx.offset);
		buffered_flush_locked(logctx.bio);
	} else {
	    int ret = buffered_write(logctx.bio, &logctx.file, 
	        sizeof(logctx.file), logctx.offset);
	    assert(ret > 0);
	    (void) ret;
		buffered_ioflush(logctx.bio, false);
	}
}

int disklog_init(void) {
    const struct disk_partition *dp_dev;
    struct disk_device *dd;
    static bool initialized;
    int ret;

    if (initialized)
        return 0;

    MTX_INIT();
    dp_dev = disk_partition_find("syslog");
    assert(dp_dev != NULL);
    dd = disk_device_find_by_part(dp_dev);

    MTX_LOCK();
    ret = buffered_iocreate(dd, dd->blk_size, false, &logctx.bio);
    if (ret)
        goto _unlock;

    lgpt_get_block_size(dp_dev, &logctx.blksz);
    logctx.offset = dp_dev->offset;
    logctx.len = dp_dev->len;
    ret = buffered_read(logctx.bio, &logctx.file, 
        sizeof(logctx.file), logctx.offset);
    if (ret < 0) {
        MTX_UNLOCK();
        pr_err("read disklog file failed(%d)\n", ret);
        return ret;
    }
    if (logctx.file.magic != DISKLOG_MAGIC)
        disklog_reset_locked();

    os_timer_create(&logctx.timer, disklog_swap, 
        NULL, false);
    
    /* Register observer for system shutdown/reoot */
    logctx.obs.update = disklog_sync_listen;
    logctx.obs.priority = 10;
    system_add_observer(&logctx.obs);

	logctx.log_dirty = false;
    initialized = true;
    ret = 0;
    os_timer_add(logctx.timer, DISLOG_SWAP_PERIOD);

_unlock:
    MTX_UNLOCK();
    return ret;
}

int disklog_input(const char *buf, size_t size) {
    struct buffered_io *bio;
    size_t remain = size;
    uint32_t wr_ofs, bytes;
    int ret;

    if (buf == NULL || size == 0)
        return -EINVAL;

    if (logctx.read_locked){
        return -EBUSY;
	}

	bio = logctx.bio;
	if (rte_likely(!bio->panic)) {
	    ret = MTX_TRYLOCK();
	    if (ret) {
	        if (logctx.upload_pending)
	            return -EBUSY;
	        MTX_LOCK();
	    }
	}

    struct disk_log *filp = &logctx.file;
    wr_ofs = filp->wr_ofs;
    while (remain > 0) {
        bytes = MIN(filp->end - wr_ofs, remain);
        ret = buffered_write(bio, buf, bytes, 
            logctx.offset + wr_ofs);
        if (ret <= 0) 
            goto _next;
        
        remain -= bytes;
        wr_ofs += bytes;
        buf += bytes;
        if (wr_ofs >= filp->end)
            wr_ofs = filp->start;
    }

    filp->d_size += size - remain;
    if (filp->d_size > filp->size) {
        filp->d_size = filp->size;
        filp->rd_ofs = wr_ofs;
    }
    ret = 0;
_next:
    filp->wr_ofs = wr_ofs;
	logctx.log_dirty = true;
	if (rte_likely(!bio->panic))
    	MTX_UNLOCK();
    return ret;
}

int disklog_ouput(bool (*output)(void *ctx, char *buf, size_t size), 
    void *ctx, size_t maxsize) {
#define LOG_SIZE 1024
    uint32_t rd_ofs, bytes;
    char *buffer;
    int ret;
	
    if (output == NULL)
        return -EINVAL;
		
    if (logctx.upload_pending)
        return -EBUSY;
		
    if (maxsize == 0)
        maxsize = LOG_SIZE;
		
    buffer = general_malloc(maxsize+1);
    if (buffer == NULL)
        return -ENOMEM;

    logctx.upload_pending = true;
    MTX_LOCK();
    struct disk_log *filp = &logctx.file;
    size_t size = filp->d_size;
    size_t remain = size;
    rd_ofs = filp->rd_ofs;
    while (remain > 0) {
        bytes = MIN(filp->end - rd_ofs, remain);
        bytes = MIN(bytes, maxsize);
        ret = buffered_read(logctx.bio, buffer, bytes, 
            logctx.offset + rd_ofs);
        if (ret <= 0)
            goto _next;
		
        pr_dbg("%s\n", buffer);
        if (!output(ctx, buffer, bytes)) {
            ret = -EIO;
            goto _next;
        }

        remain -= bytes;
        rd_ofs += bytes;
        if (rd_ofs >= filp->end)
            rd_ofs = filp->start;
    }
    ret = 0;
_next:
    MTX_UNLOCK();
	logctx.upload_pending = false;
    general_free(buffer);
    return ret;    
}

ssize_t disklog_read(char *buffer, size_t maxlen, bool first) {
    static struct disk_log rdlog;
    uint32_t rd_ofs, bytes;
    int ret = 0;

    if (!buffer || !maxlen)
        return -EINVAL;

    ret = MTX_TRYLOCK();
    if (ret) {
        if (logctx.upload_pending)
            return -EBUSY;
        MTX_LOCK();
    }

    if (first)
        rdlog = logctx.file;

    size_t size = rdlog.d_size;
    size_t remain = MIN(maxlen, size);
    rd_ofs = rdlog.rd_ofs;
    while (remain > 0) {
        bytes = MIN(rdlog.end - rd_ofs, remain);
        ret = buffered_read(logctx.bio, buffer, bytes, 
            logctx.offset + rd_ofs);
        if (ret <= 0)
            goto _next;

        remain -= bytes;
        rd_ofs += bytes;
        size -= bytes;
        if (rd_ofs >= rdlog.end)
            rd_ofs = rdlog.start;
    }
    rdlog.rd_ofs = rd_ofs;
    rdlog.d_size = size;
_next:
    MTX_UNLOCK();
    return ret;   
}

void disklog_upload_cb(struct log_upload_class *luc) {
    if (!luc || !luc->upload)
        return;
    if (luc->begin)
        luc->begin(luc->ctx);

    int err = disklog_ouput(luc->upload, luc->ctx, luc->maxsize);

    if (luc->end)
        luc->end(luc->ctx, err);
}

int disklog_read_lock(bool enable) {
    MTX_LOCK();
    logctx.read_locked = enable;
    MTX_UNLOCK();
    return 0;
}