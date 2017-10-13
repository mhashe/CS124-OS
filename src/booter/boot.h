/* The BIOS always loads the boot sector at address 0x07c00 (almost always as
 * the real address 0000:7c00).  Being 512 bytes, the sector ends at address
 * 0x07e00.
 */
#define BOOT_BASE_ADDR      0x7c00
#define BOOT_END_ADDR       0x7e00


/* This is where the bootloader signature goes.  It's 2 bytes, 0xaa55. */
#define BOOT_SIG_ADDR       (BOOT_END_ADDR - 2)


/* The main program expects to be loaded at address 0x00020000. */
#define PROGRAM_BASE_ADDR   0x00020000
#define PROGRAM_SEGMENT     0x2000
#define PROGRAM_OFFSET      0x0
#define PROGRAM_NUM_SECT    20


/* Flags in control register 0. */
#define CR0_PE              0x00000001      /* Protection Enable. */
#define CR0_EM              0x00000004      /* (Floating-point) Emulation. */
#define CR0_PG              0x80000000      /* Paging. */
#define CR0_WP              0x00010000      /* Write-Protect in kernel mode. */


/* Global Descriptor Table selectors. */
#define SEL_NULL            0x00    /* Null selector. */
#define SEL_CODESEG         0x08    /* Code selector. */
#define SEL_DATASEG         0x10    /* Data selector. */


/* Where we initialize the stack. */
#define STACK_BASE          0xf000


/* Cursor end-row. */
#define CURSOR_END          0x2607


/* E_entry offset. */
#define E_OFFSET            0x18


/* Interrupts, interrupt conditions. */
#define DISPLAY_INT         0x10
#define DISK_INT            0x13
#define EXT_READ            0x42


/* Different video display modes. */
#define TEXT_40_25_M_MODE      0x00     /* text 40*25 16 color (mono)*/
#define TEXT_40_25_16_MODE     0x01     /* text 40*25 16 color*/
#define TEXT_80_25_M_MODE      0x02     /* text 80*25 16 color (mono)*/
#define TEXT_80_25_16_MODE     0x03     /* text 80*25 16 color*/
#define CGA_320_200_4_MODE     0x04     /* CGA 320*200 4 color*/
#define CGA_320_200_M_MODE     0x05     /* CGA 320*200 4 color (m)*/
#define CGA_640_200_2_MODE     0x06     /* CGA 640*200 2 color*/
#define MDA_80_25_M_MODE       0x07     /* MDA monochrome text 80*25*/
#define EGA_320_200_16_MODE    0x0D     /* EGA 320*200 16 color*/
#define EGA_640_200_16_MODE    0x0E     /* EGA 640*200 16 color*/
#define EGA_640_350_M_MODE     0x0F     /* EGA 640*350 mono*/
#define EGA_640_350_16_MODE    0x10     /* EGA 640*350 16 color*/
#define VGA_640_480_M_MODE     0x11     /* VGA 640*480 mono*/
#define VGA_640_480_16_MODE    0x12     /* VGA 640*480 16 color*/
#define VGA_640_480_256_MODE   0x13     /* VGA 320*200 256 color*/
