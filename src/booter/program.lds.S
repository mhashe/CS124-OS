#include "boot.h"

OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH("i386")

/* The game code itself will start with the game_main() function. */
ENTRY(c_start)
SECTIONS
{
    /* Specify the base address of the main program. */
    _start = PROGRAM_BASE_ADDR;

    /* Make room for the ELF headers. */
    . = _start + SIZEOF_HEADERS;

    /* Start the program with code, followed by read-only data and
     * then finally  writable data. */
    .text : { *(.start) *(.text) } = 0x90
    .rodata : { *(.rodata) *(.rodata.*) 
                . = ALIGN(0x1000); 
                _end_kernel_text = .; }
    .data : { *(.data) 
              _signature = .; LONG(0xaa55aa55) }

    /* BSS (zero-initialized data) is after everything else. */
    _start_bss = .;
    .bss : { *(.bss) }
    _end_bss = .;

    _end = .;

    ASSERT(_end - _start <= 512K, "Image is too big.")
}

