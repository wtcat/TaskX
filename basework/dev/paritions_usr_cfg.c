/*
 * Copyright 2022 wtcat
 */
#include <errno.h>
#include "basework/generic.h"
#include "basework/dev/partition.h"
#include "basework/dev/partition_file.h"

#define BYTE(n) SFILE_SIZE(n)
#define KB(n) SFILE_SIZE(n * 1024)

#define PT_ENTRY(name, size) \
    PARTITION_ENTRY(name, NULL, -1, size)

/*
 * Partition configure table
 */
PARTITION_TABLE_DEFINE(partitions_usr_configure) {
#ifdef CONFIG_CUSTOM_PT_TABLE
    #include CONFIG_MINOR_PT_FILE
#else
    #include "config/minor_ptcfg_default.h"
#endif
    PARTITION_TERMINAL
};

/*
 * This partition can not be erased when the system reset factory settings
 */
PARTITION_TABLE_DEFINE(partitions_usr2_configure) {
#ifdef CONFIG_CUSTOM_PT_TABLE
    #include CONFIG_MINOR_PT2_FILE
#else
    #include "config/minor_ptcfg2_default.h"
#endif
    PARTITION_TERMINAL
};

int usr_partition_init(void) {
    int err;
    err = logic_partitions_create("usrdata", partitions_usr_configure);
    if (!err)
        err = logic_partitions_create("usrdata2", partitions_usr2_configure);
    return err;
}
