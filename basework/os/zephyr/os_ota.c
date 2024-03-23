/*
 * Copyright 2023 wtcat
 */
#define CONFIG_LOGLEVEL  LOGLEVEL_DEBUG
#define pr_fmt(fmt) "os_ota: " fmt
#include <assert.h>
#include <string.h>
#include <init.h>
#include <partition/partition.h>
#include <sdfs.h>

#include "basework/dev/blkdev.h"
#include "basework/dev/disk.h"
#include "basework/dev/partition.h"
#include "basework/utils/ota_fstream.h"
#include "basework/log.h"
#include "basework/boot/boot.h"

struct file_partition {
    struct disk_device *dd;
    uint32_t offset;
    uint32_t len;
    const char *name;
};

static bool res_first = true;
static struct file_partition file_dev;
static struct partition_entry resext_entry = {
    .name = {"res_ext"},
    .type = 0,
    .file_id = 11,
    .mirror_id = 0,
    .storage_id = 0,
    .flag = 0,
    .size = 0x30000,
};

//Picture resource extension partition
const struct partition_entry *partition_get_stf_part_ext(
	u8_t stor_id, u8_t file_id) {
  //16MB nor
    if (resext_entry.storage_id == stor_id && 
        resext_entry.file_id == file_id &&
        res_first) {
        return &resext_entry;
    }
    return NULL;
}

void resource_ext_check(void) {
#define PARTITION_OFS(x) (fw->offset + (x))
    const struct disk_partition *fw; 
    const struct partition_entry *part;

    fw = disk_partition_find("firmware");
    assert(fw != NULL);

    part = partition_get_stf_part(STORAGE_ID_NOR, 
        PARTITION_FILE_ID_SYSTEM);
    assert(part != NULL);

    if (part->size < 0x1c2000)
        resext_entry.offset = PARTITION_OFS(0x1c2000);
    else
        resext_entry.offset = PARTITION_OFS(part->size);
    resext_entry.file_offset = resext_entry.offset;
    pr_notice("ext-resource offset(0x%x) size(0x%x)\n", 
		part->offset, part->size);
    if (sdfs_verify("/NOR:B")) {
        res_first = false;
		sdfs_verify("/NOR:B");
	}
}

static int partition_get_fid(const char *fname) {
    int fid;

    if (!strcmp(fname, "res+.bin"))
        fid = 14;
    else if (!strcmp(fname, "fonts+.bin"))
        fid = 15;
    else
        fid = -1;
    return fid;
}

static void *partition_get_fd(struct file_partition *dev, const char *name) {
    const struct partition_entry *pe;
    const struct disk_partition *dp;
    int fid;

    fid = partition_get_fid(name);
    if (fid < 0)
        return NULL;

    pe = partition_get_stf_part(STORAGE_ID_NOR, fid);
    if (pe == NULL || pe->offset == UINT32_MAX)
        return NULL;

    dp = disk_partition_find("firmware");
    assert(dp != NULL);
    dev->offset = dp->offset;
    dev->len = pe->size;
    dev->name = name;
    pr_dbg("open %s partition: start(0x%x) size(%d)\n", 
        name, dp->offset, dp->len);
    return dev;
}

static void* partition_open(const char *name) {
    const struct partition_entry *parti;

    if (!file_dev.dd) {
        disk_device_open("spi_flash", &file_dev.dd);
        assert(file_dev.dd != NULL);
    }
    /* Picture resource partition */
    if (!strcmp(name, "res.bin")) {
        parti = partition_get_stf_part(STORAGE_ID_NOR, 
            PARTITION_FILE_ID_SDFS_PART0);
        assert(parti != NULL);
        file_dev.offset = parti->offset;
        file_dev.len = parti->size;
        file_dev.name = "picture";
        pr_dbg("open picture partition: start(0x%x) size(%d)\n", 
            file_dev.offset, file_dev.len);
        return &file_dev;
    }
	
    /* Picture resource ext-partition */
    if (!strcmp(name, "res_ext.bin")) {
        parti = partition_get_stf_part(STORAGE_ID_NOR, 
            PARTITION_FILE_ID_SDFS_PART1);
        assert(parti != NULL);
		file_dev.offset = parti->offset;
		file_dev.len = parti->size;
        file_dev.name = "picture-ext";
        pr_dbg("open picture extension partition: start(0x%x) size(%d)\n", 
            file_dev.offset, file_dev.len);
        return &file_dev;
    }

    /* Font resource partition */
    if (!strcmp(name, "fonts.bin")) {
        parti = partition_get_stf_part(STORAGE_ID_NOR, 
            PARTITION_FILE_ID_SDFS_PART2);
        assert(parti != NULL);
        file_dev.offset = parti->offset;
        file_dev.len = parti->size;
        file_dev.name = "font";
        pr_dbg("open fonts partition: start(0x%x) size(%d)\n", 
            file_dev.offset, file_dev.len);
        return &file_dev;
    }

    /* System configuration */
    if (!strcmp(name, "sdfs.bin")) {
        parti = partition_get_stf_part(STORAGE_ID_NOR, 
            PARTITION_FILE_ID_SDFS);
        assert(parti != NULL);
        file_dev.offset = parti->offset;
        file_dev.len = parti->size;
        file_dev.name = "sdfs";
        pr_dbg("open sdfs partition: start(0x%x) size(%d)\n", 
            file_dev.offset, file_dev.len);
        return &file_dev;
    }

    if (!strcmp(name, "sdfs_k.bin")) {
        parti = partition_get_stf_part(STORAGE_ID_NOR, 
            20);
        assert(parti != NULL);
        file_dev.offset = parti->offset;
        file_dev.len = parti->size;
        file_dev.name = "sdfs_k";
        pr_dbg("open sdfs_k partition: start(0x%x) size(%d)\n", 
            file_dev.offset, file_dev.len);
        return &file_dev;
    }

    /* Default Dial */
    if (!strcmp(name, "dial.bin")) {
        parti = partition_get_stf_part(STORAGE_ID_NOR, 
            PARTITION_FILE_ID_UDISK);
        assert(parti != NULL);
        file_dev.offset = parti->offset;
        file_dev.len = parti->size;
        file_dev.name = "dial";
        pr_dbg("open udisk partition: start(0x%x) size(%d)\n", 
            file_dev.offset, file_dev.len);
        return &file_dev;
    }
    
    /* Firmware partition */
#ifndef CONFIG_BOARD_ATS3085L_HG_BEANS_EXT_NOR
    if (!strcmp(name, "zephyr.bin")) {
        const struct disk_partition *dp;
        dp = disk_partition_find("firmware");
        assert(dp != NULL);
        file_dev.offset = dp->offset;
        file_dev.len = dp->len;
        file_dev.name = "firmware";
        pr_dbg("open firmware partition: start(0x%x) size(%d)\n", 
            dp->offset, dp->len);
        
        struct firmware_pointer fwinfo = {0};
        dp = disk_partition_find("firmware_cur");
        assert(dp != NULL);

        /* Record firmware address information */
        disk_partition_read(dp, 0, &fwinfo, sizeof(fwinfo));
        if (fwinfo.addr.fw_offset != file_dev.offset) {
            pr_info("Firmware information record address(0x%08x)\n", dp->offset);
            fwinfo.addr.fw_offset = file_dev.offset;
            fwinfo.addr.fw_size = file_dev.len;

            /* Calculate check sum */
            const uint8_t *p = (const uint8_t *)&fwinfo.addr;
            uint8_t chksum = 0;
            for (size_t i = 0; i < offsetof(struct firmware_addr, chksum); i++) 
                chksum ^= p[i];
            fwinfo.addr.chksum = chksum;

            disk_partition_erase_all(dp);
            disk_partition_write(dp, 0, &fwinfo, sizeof(fwinfo));

            //TODO:
            struct firmware_pointer fwchk;
            disk_partition_read(dp, 0, &fwchk, sizeof(fwchk));
            if (fwchk.addr.fw_offset != fwinfo.addr.fw_offset)
                assert(0);
        }

        return &file_dev;
    }
#else
    if (!strcmp(name, "zephyr.bin")) {
        parti = partition_get_stf_part(STORAGE_ID_NOR, 50);
        assert(parti != NULL);
        file_dev.offset = parti->offset + 0x1000;
        file_dev.len = parti->size - 0x1000;
        file_dev.name = "firmware";
        pr_dbg("open fonts partition: start(0x%x) size(%d)\n", 
            file_dev.offset, file_dev.len);
        return &file_dev;
    }
#endif /* CONFIG_BOARD_ATS3085L_HG_BEANS_EXT_NOR */

    if (!strcmp(name, "wtm_app.bin")) {
        parti = partition_get_stf_part(STORAGE_ID_NOR, 51);
        assert(parti != NULL);
        file_dev.offset = parti->offset;
        file_dev.len = parti->size;
        file_dev.name = "wtm_app";
        pr_dbg("open wtm-app partition: start(0x%x) size(%d)\n", 
            parti->offset, parti->size);
        return &file_dev;
    }

    if (!strcmp(name, "wtm_boot.bin")) {
        parti = partition_get_stf_part(STORAGE_ID_NOR, 50);
        assert(parti != NULL);
        file_dev.offset = parti->offset;
        file_dev.len = parti->size;
        file_dev.name = "wtm_boot";
        pr_dbg("open wtm_boot partition: start(0x%x) size(%d)\n", 
            parti->offset, parti->size);
        return &file_dev;
    }
		
    if (!strcmp(name, "msgfile.bin")) {
        const struct disk_partition *dp;
        dp = disk_partition_find("msgfile.db");
        assert(dp != NULL);
        file_dev.offset = dp->offset;
        file_dev.len = dp->len;
        file_dev.name = "msgfile";
        pr_dbg("open msgfile.db partition: start(0x%x) size(%d)\n", 
            dp->offset, dp->len);
        return &file_dev;
    }

    if (!strcmp(name, "recovery.bin")) {
        parti = partition_get_stf_part(STORAGE_ID_NOR, 3);
        assert(parti != NULL);
        file_dev.offset = parti->offset;
        file_dev.len = parti->size;
        file_dev.name = "recovery";
        pr_dbg("open bootloader partition: start(0x%x) size(%d)\n", 
            parti->offset, parti->size);
        return &file_dev;
    }
	
    if (!strcmp(name, "mbrec.bin")) {
        parti = partition_get_stf_part(STORAGE_ID_NOR, 1);
        assert(parti != NULL);
        file_dev.offset = parti->offset;
        file_dev.len = parti->size;
        file_dev.name = "mbrec";
        pr_dbg("open mbrec partition: start(0x%x) size(%d)\n", 
            parti->offset, parti->size);
        return &file_dev;
    }

    if (!strcmp(name, "res+.bin") ||
        !strcmp(name, "fonts+.bin"))
        return partition_get_fd(&file_dev, name);

    return NULL;
}

static void partition_close(void *fp) {
    (void) fp;
    blkdev_sync();
}

static int partition_write(void *fp, const void *buf, 
    size_t size, uint32_t offset) {
    struct file_partition *filp = fp;
    uint32_t base;

    base = filp->offset + offset;
    if (offset + size > filp->len) {
        pr_err("write address is out of range(offset:0x%x size:%d "
                "partition(name:%s ofs:0x%x, size:%d))\n",
            offset, size, filp->name, filp->offset, filp->len);
        return -EINVAL;
    }
    return blkdev_write(filp->dd, buf, size, base);
}

static int partition_data_copy(struct disk_device *dd, uint32_t dst, 
    uint32_t src, size_t size, size_t blksz) {
	size_t len = size;
	uint32_t offset = 0;
    char buffer[1024];
    int ret;

    /* Flush disk cache */
    blkdev_sync();

    /* 
     * Erase destination address. The parititon size 
     * must be a multiple of the block size 
     */
    pr_dbg("erasing address(0x%x) size(0x%x)...\n", dst, rte_roundup(len, blksz));
    disk_device_erase(dd, dst, rte_roundup(len, blksz));

    /* Copy data */
    while (len > 0) {
        size_t bytes = rte_min(len, sizeof(buffer));

        ret = disk_device_read(dd, buffer, bytes, src + offset);
        if (ret < 0) {
            pr_err("read 0x%x failed\n", src + offset);
            return ret;
        }
        ret = disk_device_write(dd, buffer, bytes, dst + offset);
        if (ret) {
            pr_err("write 0x%x failed\n", dst + offset);
            return ret;
        }
        offset += bytes;
        len   -= bytes;
    }

    pr_info("Copy %d bytes finished (from 0x%x to 0x%x )\n", size, src, dst);
    return 0;
}

static void partition_completed(int err, void *fp, const char *fname, 
    size_t size) {
    int fid;

    if (err)
        return;

    fid = partition_get_fid(fname);
    if (fid > 0) {
        const struct partition_entry *pe;
        const struct file_partition *filp;

        filp = (struct file_partition *)fp;
        pe = partition_get_stf_part(STORAGE_ID_NOR, fid);
        if (pe != NULL) {
            pr_dbg("increase partiton(%s): offset(0x%08x) size(0x%x)\n", 
				pe->name, pe->offset, pe->size);
            size_t blksz = disk_device_get_block_size(filp->dd);
            if (pe->offset != UINT32_MAX && 
                pe->size >= rte_roundup(size, blksz)) {
                partition_data_copy(filp->dd, pe->offset, filp->offset, size, blksz);
                return;
            }
        }
        assert(0);
    }
}

static int __rte_notrace ota_fstream_init(const struct device *dev) {
    (void) dev;
    static const struct ota_fstream_ops fstream_ops = {
        .open  = partition_open,
        .close = partition_close,
        .write = partition_write,
        .completed = partition_completed
    };
    ota_fstream_set_ops(&fstream_ops);
    return 0;
}

SYS_INIT(ota_fstream_init, APPLICATION, 
    CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
