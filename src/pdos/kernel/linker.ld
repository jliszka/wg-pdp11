MEMORY
{
    IROM (rx) : ORIGIN = 0x400, LENGTH = 63K   /* I space: for instructions (read/execute) */
    DRAM (rw) : ORIGIN = 0x0, LENGTH = 64K     /* D space: for data (read/write) */
}

SECTIONS
{
    /* Place the .text section (code) in I space (IROM) */
    .text :
    {
        *(.text)
    } > IROM

    /* Place the .data section (initialized data) in D space (DRAM) */
    .data :
    {
        *(.data)
    } > DRAM

    /* Place the .bss section (uninitialized data) in D space (DRAM) */
    .bss :
    {
        *(.bss)
    } > DRAM
}
