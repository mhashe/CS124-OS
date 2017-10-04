/* The BIOS always loads the boot sector at address 0x07c00 (almost always as
 * the real address 0000:7c00).  Being 512 bytes, the sector ends at address
 * 0x07e00.
 */
#define BOOT_BASE_ADDR 0x7c00
#define BOOT_END_ADDR  0x7e00

/* This is where the bootloader signature goes.  It's 2 bytes, 0xaa55. */
#define BOOT_SIG_ADDR (BOOT_END_ADDR - 2)

/* The main program expects to be loaded at address 0x00020000 */
#define PROGRAM_BASE_ADDR 0x00020000

/* Flags in control register 0. */
#define CR0_PE 0x00000001      /* Protection Enable. */
#define CR0_EM 0x00000004      /* (Floating-point) Emulation. */
#define CR0_PG 0x80000000      /* Paging. */
#define CR0_WP 0x00010000      /* Write-Protect enable in kernel mode. */

/* Global Descriptor Table selectors. */
#define SEL_NULL          0x00    /* Null selector. */
#define SEL_CODESEG       0x08    /* Code selector. */
#define SEL_DATASEG       0x10    /* Data selector. */

