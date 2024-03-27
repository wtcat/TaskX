MEMORY {
  /* ITCM boot address */
  ITCM  (rwx) : ORIGIN = 0x00000100, LENGTH = 64K - 0x100

  /* 2048Kb FLASH */
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 128K

  /* DTCM SRAM */
  DTCM (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
 
  /* 512Kb of contiguous AXI SRAM */
  SRAM_AXI (rwx)  : ORIGIN = 0x24000000, LENGTH = 512K
  
  /* SRAM123 */
  SRAM_1      : ORIGIN = 0x30000000, LENGTH = 128K
  SRAM_2      : ORIGIN = 0x30020000, LENGTH = 128K
  SRAM_3      : ORIGIN = 0x30040000, LENGTH = 32K
  SRAM_4      : ORIGIN = 0x38000000, LENGTH = 64K
  SRAM_BACKUP : ORIGIN = 0x38800000, LENGTH = 4K
  
  /* SDRAM_1 */
  SDRAM_1     : ORIGIN = 0xC0000000, LENGTH = 32768K
}

_system_stack_size = 2048;

REGION_ALIAS("REGION_START",          SDRAM_1);
REGION_ALIAS("REGION_VECTOR",         SDRAM_1);
REGION_ALIAS("REGION_TEXT",           SDRAM_1);
REGION_ALIAS("REGION_TEXT_LOAD",      SDRAM_1);
REGION_ALIAS("REGION_RODATA",         SDRAM_1);
REGION_ALIAS("REGION_RODATA_LOAD",    SDRAM_1);
REGION_ALIAS("REGION_DATA",           SRAM_4);
REGION_ALIAS("REGION_DATA_LOAD",      SDRAM_1);
REGION_ALIAS("REGION_FAST_TEXT",      ITCM);
REGION_ALIAS("REGION_FAST_TEXT_LOAD", SDRAM_1);
REGION_ALIAS("REGION_FAST_DATA",      DTCM);
REGION_ALIAS("REGION_FAST_DATA_LOAD", SDRAM_1);
REGION_ALIAS("REGION_BSS",            SRAM_AXI);
REGION_ALIAS("REGION_STACK",          SRAM_4);
REGION_ALIAS("REGION_WORK",           SDRAM_1);
REGION_ALIAS("REGION_NOCACHE",        SDRAM_1);
REGION_ALIAS("REGION_NOCACHE_LOAD",   SDRAM_1);


OUTPUT_ARCH(arm)
EXTERN(_vectors)
ENTRY(_start)
SECTIONS
{
    .vectors : 
    {
        *(.vectors)
    } > REGION_TEXT

    .itcm :
    {
        _itcm_text_start = .;
        *(.itcm*)
        threadx/CMakeFiles/threadx.dir/common/src/tx_event_flags_get.c.obj (.text*)
        threadx/CMakeFiles/threadx.dir/common/src/tx_event_flags_set.c.obj (.text*)
        threadx/CMakeFiles/threadx.dir/common/src/tx_semaphore_get.c.obj (.text*)
        threadx/CMakeFiles/threadx.dir/common/src/tx_semaphore_put.c.obj (.text*)
        threadx/CMakeFiles/threadx.dir/common/src/tx_queue_receive.c.obj (.text*)
        threadx/CMakeFiles/threadx.dir/common/src/tx_queue_send.c.obj (.text*)
        threadx/CMakeFiles/threadx.dir/common/src/tx_mutex_put.c.obj (.text*)
        threadx/CMakeFiles/threadx.dir/common/src/tx_mutex_get.c.obj (.text*)
        threadx/CMakeFiles/threadx.dir/common/src/tx_timer_system_activate.c.obj (.text*)
        threadx/CMakeFiles/threadx.dir/common/src/tx_timer_expiration_process.c.obj (.text*)
        _itcm_text_end = .;
    } > REGION_FAST_TEXT AT > REGION_TEXT
    _itcm_text_load = LOADADDR(.itcm);

    .text :
    {
        _stext = ABSOLUTE(.);
        *(.text .text.*)
        *(.fixup)
        *(.gnu.warning)
        *(.rodata .rodata.*)
        *(.gnu.linkonce.t.*)
        *(.glue_7)
        *(.glue_7t)
        *(.got)
        *(.gcc_except_table)
        *(.gnu.linkonce.r.*)
        _etext = ABSOLUTE(.);
    } > REGION_TEXT

    .init_section :
    {
        _sinit = ABSOLUTE(.);
        *(.init_array .init_array.*)
        _einit = ABSOLUTE(.);
    } > REGION_TEXT

    .ARM.extab :
    {
        *(.ARM.extab*)
    } > REGION_TEXT

    _exidx_start = ABSOLUTE(.);
    .ARM.exidx :
    {
        *(.ARM.exidx*)
    } > REGION_TEXT
    _exidx_end = ABSOLUTE(.);

    _eronly = ABSOLUTE(.);
	
	.dtcm (NOLOAD):
	{
        _dtcm_bss_start = .;
		*(.ram_vector)
		*(.dtcm*)
        threadx/CMakeFiles/threadx.dir/common/src/tx_thread_initialize.c.obj (.bss*)
        threadx/CMakeFiles/threadx.dir/common/src/tx_timer_initialize.c.obj (.bss*)
        _dtcm_bss_end = .;

        . = ALIGN(4);
        _system_stack_bottom = .;
        . += _system_stack_size;
        _system_stack_top = .;
	} > REGION_FAST_DATA

    .data :
    {
        _sdata_start = .;
		*(.data .data.* .gnu.linkonce.d.*)
		SORT(CONSTRUCTORS)
        _sdata_end = .;
    } > REGION_FAST_DATA AT > REGION_TEXT
	_sdata_load_begin = LOADADDR(.data);

    .bss (NOLOAD):
    {
        _bss_start = ABSOLUTE(.);
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = ABSOLUTE(.);
    } > REGION_BSS
	
	.stack ALIGN(4) (NOLOAD): 
	{
        *(.stack)
		_isr_stack_begin = .;
        . += ORIGIN(REGION_STACK) + LENGTH(REGION_STACK) - ABSOLUTE(.) - 4;
		_isr_stack_end = .;
	} > REGION_STACK

	/* .heap ALIGN(4) (NOLOAD): 
	{
		_heap_begin = .;
		. += ORIGIN(REGION_WORK) + LENGTH(REGION_WORK) - ABSOLUTE(.) - 4;
		_heap_end = .;
	} > REGION_WORK */
	
	_sram_heap_begin  = ORIGIN(SRAM_1);
	_sram_heap_end    = _sram_heap_begin + LENGTH(SRAM_1) + LENGTH(SRAM_2) + LENGTH(SRAM_3);

    /* Stabs debugging sections. */
    .stab 0 : { *(.stab) }
    .stabstr 0 : { *(.stabstr) }
    .stab.excl 0 : { *(.stab.excl) }
    .stab.exclstr 0 : { *(.stab.exclstr) }
    .stab.index 0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment 0 : { *(.comment) }
    .debug_abbrev 0 : { *(.debug_abbrev) }
    .debug_info 0 : { *(.debug_info) }
    .debug_line 0 : { *(.debug_line) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    .debug_aranges 0 : { *(.debug_aranges) }
}

