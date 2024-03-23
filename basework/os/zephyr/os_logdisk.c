/*
 * Copyright 2023 wtcat
 */
#define CONFIG_LOGLEVEL LOGLEVEL_DEBUG
#define pr_fmt(fmt) "<os_logdisk>: "fmt
#include <errno.h>
#include <string.h>

#include <drivers/disk.h>
#include <board_cfg.h>
#include <fs_manager.h>
#include <partition/partition.h>

#include "basework/dev/partition.h"
#include "basework/malloc.h"
#include "basework/log.h"

struct mp_info {
    const char *name;
    const char *dev;
    const char *mp;
    size_t limit_size;
};

#define NOR_LOG_SIZE 9 
#define NOR_SECTOR_SIZE (1 << NOR_LOG_SIZE)
#if !defined(CONFIG_PTFS) && !defined(CONFIG_PAGEFS)
#define MP_TBSIZE 3
#else
#define MP_TBSIZE 2
#endif

static const struct mp_info mp_table[] = {
    {"filesystem", "UD0", "/UD0:", UINT32_MAX},
    {"firmware",   "UD1", "/UD1:", 0x1c2000},  /* Limit to 1.8MB */
#if !defined(CONFIG_PTFS) && !defined(CONFIG_PAGEFS)
    {"udisk",      "UD2", "/UD2:", UINT32_MAX},  /* just only for  */
#endif
};
_Static_assert(MP_TBSIZE == ARRAY_SIZE(mp_table), "");

static struct fs_mount_t mount_entry[MP_TBSIZE];

#ifdef CONFIG_FILE_SYSTEM_LITTLEFS
#include <fs/littlefs.h>

#if MP_TBSIZE > 0
FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(liitefs_storage_1);
#endif
#if MP_TBSIZE > 1
FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(liitefs_storage_2);
#endif
#if MP_TBSIZE > 2
FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(liitefs_storage_3);
#endif

static struct fs_littlefs *littlefs_dat[] = {
#if MP_TBSIZE > 0
    &liitefs_storage_1,
#endif
#if MP_TBSIZE > 1
    &liitefs_storage_2,
#endif
#if MP_TBSIZE > 2
    &liitefs_storage_3,
#endif
};
static struct flash_area fs_flash_map[MP_TBSIZE];
const struct flash_area *flash_map = fs_flash_map;
const int flash_map_entries = MP_TBSIZE;

#else /* !CONFIG_FILE_SYSTEM_LITTLEFS */
extern struct disk_operations disk_nor_operation;
static FATFS fat_data[MP_TBSIZE];

/*
 * Create logic device base on norflash device
 */
static int platform_nor_logdisk_create(const char *name, 
    uint32_t offset, uint32_t size) {
    struct disk_info *di;

    if (!name) {
        pr_err("Invalid physic device(null)\n");
        return -EINVAL;
    }
    if (offset + size > CONFIG_SPI_FLASH_CHIP_SIZE) {
        pr_err("Invalid offset or size\n");
        return -EINVAL;
    }
    if (size < NOR_SECTOR_SIZE) {
        pr_err("Invalid size\n");
        return -EINVAL;
    }

    di = general_calloc(1, sizeof(*di) + strlen(name) + 1);
    if (!di) {
        pr_err("No more memory\n");
        return -ENOMEM;
    }

    pr_dbg("Create logic device(%s) offset(0x%x) size(0x%x)\n", name, offset, size);
    di->name = (char *)(di + 1);
    strcpy(di->name, name);
    di->sector_size = NOR_SECTOR_SIZE;
    di->sector_offset = offset >> NOR_LOG_SIZE;
    di->sector_cnt = size >> NOR_LOG_SIZE;
    di->ops = &disk_nor_operation;
    return disk_access_register(di);
}
#endif /* CONFIG_FILE_SYSTEM_LITTLEFS */

/*
 * Mount fatfs to filesystem parition
 */
int __rte_notrace platform_extend_filesystem_init(int fs_type) {
    const struct partition_entry *parti;
    struct fs_mount_t *mp;
    const struct disk_partition *fpt;
    struct disk_partition dpobj;
		size_t limit;
    int err = 0;

    for (int i = 0; i < ARRAY_SIZE(mp_table); i++) {
        fpt = disk_partition_find(mp_table[i].name);
        if (!fpt) { 
            if (strcmp(mp_table[i].name, "udisk")) {
                pr_err("Not found filesystem partition(%s)\n", mp_table[i].name);
                return -ENODATA;
            }
            parti = partition_get_stf_part(STORAGE_ID_NOR, 
                PARTITION_FILE_ID_UDISK);
            dpobj.parent = CONFIG_SPI_FLASH_NAME;
            dpobj.offset = parti->offset;
            dpobj.len = parti->size;
            fpt = &dpobj;
        }

        mp = &mount_entry[i];
        mp->mnt_point = mp_table[i].mp;
				limit = mp_table[i].limit_size;
    #ifdef CONFIG_FILE_SYSTEM_LITTLEFS
        if (fs_type == FS_LITTLEFS) {
            fs_flash_map[i].fa_id = (uint8_t)i;
            fs_flash_map[i].fa_device_id = (uint8_t)i;
            fs_flash_map[i].fa_off = fpt->offset;
            fs_flash_map[i].fa_size = rte_min(fpt->len, limit);
            fs_flash_map[i].fa_dev_name =fpt->parent;

            mp->type = FS_LITTLEFS;
            mp->fs_data = littlefs_dat[i];
            mp->storage_dev = (void *)i;
            err |= fs_mount(mp);
            continue;
        }

    #else /* !CONFIG_FILE_SYSTEM_LITTLEFS */
        if (fs_type == FS_FATFS) {
            err = platform_nor_logdisk_create(mp_table[i].dev, 
                fpt->offset, fpt->len);
            if (err)
                return err;
            if (disk_access_init(mp_table[i].dev) != 0) 
                return -ENODEV;

            mp->type = FS_FATFS;
            mp->fs_data = &fat_data[i];
            err |= fs_mount(mp);
            continue;
        }
    #endif /* CONFIG_FILE_SYSTEM_LITTLEFS */
    }
    return err;
}
