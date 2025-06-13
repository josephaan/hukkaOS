

ENTRY(main)

SECTIONS
{
    // . = ; // Must assign virtual address here where the process gets loaded at
    .text : {
        KEEP(*(.text.boot)) *(.text .text.*)   /* code */
        *(.rodata .rodata.*)                   /* data */
        *(.data .data.*)
    } 
    
    .bss (NOLOAD) : {                          /* bss */
        . = ALIGN(16);
        *(.bss .bss.*)
        *(COMMON)
    } 


    /DISCARD/ : { *(.eh_frame) *(.comment) }
}
