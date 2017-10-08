#include "interrupts.h"
#include "boot.h"
#include "ports.h"

#include <stdint.h>


/*============================================================================
 * INTERRUPT DESCRIPTOR TABLE
 *
 * This section declares the interrupt descriptor table and related types.
 */

#define NUM_INTERRUPTS 256


/* This is the structure of a single interrupt descriptor.  For 32-bit
 * interrupt addresses, the address is split across two 16-bit fields.
 */
typedef struct IDT_Descriptor {
    uint16_t offset_15_0;      // offset bits 0..15
    uint16_t selector;         // a code segment selector in GDT or LDT
    uint8_t zero;              // unused, set to 0
    uint8_t type_attr;         // descriptor type and attributes
    uint16_t offset_31_16;     // offset bits 16..31
} IDT_Descriptor;


/* The interrupt descriptor table we will use. */
static IDT_Descriptor interrupt_descriptor_table[NUM_INTERRUPTS];


/* Load a new interrupt descriptor table.  The function takes two arguments:
 *
 *     base = starting address of the interrupt descriptor table
 *     size = the size of the interrupt descriptor table *IN BYTES*
 *
 * (Specifying the IDT size in bytes seems dumb, but it's what the "lidt"
 * instruction wants...  See IA32 manual for details.)
 */
static inline void lidt(void* base, uint16_t size) {
    // This function works in 32 and 64bit mode
    struct {
        uint16_t length;
        void*    base;
    } __attribute__((packed)) IDTR = { size, base };
 
    // let the compiler choose an addressing mode
    asm ( "lidt %0" : : "m"(IDTR) );
}


/*============================================================================
 * 8259 PROGRAMMABLE INTERRUPT CONTROLLER
 *
 * We need to do some basic configuration on the programmable interrupt
 * controller (PIC) in order to get our interrupts delivered properly.
 * See http://wiki.osdev.org/PIC for details.
 *
 * Note that the PIC is not the same as the APIC (Advanced Programmable
 * Interrupt Controller) - this is the original, simple PIC that was included
 * on 8086/8088-based Personal Computers.
 *
 * For more details on the 8259 PIC, see:  http://wiki.osdev.org/8259_PIC
 */

#define PIC1         0x20           /* IO base address for master PIC */
#define PIC2         0xA0           /* IO base address for slave PIC */
#define PIC1_COMMAND PIC1
#define PIC1_DATA    (PIC1+1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA    (PIC2+1)

/* These definitions are used to reinitialize the PIC controllers, giving
 * them the specified vector offsets rather than 8h and 70h, as configured
 * by default.  (The master PIC interrupt IRQs fall within the first 32
 * protected-mode interrupts, which are reserved by the processor.)
 */
#define ICW1_ICW4       0x01        /* ICW4 (not) needed */
#define ICW1_SINGLE     0x02        /* Single (cascade) mode */
#define ICW1_INTERVAL4  0x04        /* Call address interval 4 (8) */
#define ICW1_LEVEL      0x08        /* Level triggered (edge) mode */
#define ICW1_INIT       0x10        /* Initialization - required! */
 
#define ICW4_8086       0x01        /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO       0x02        /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE  0x08        /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C        /* Buffered mode/master */
#define ICW4_SFNM       0x10        /* Special fully nested (not) */
 
/* Remap the interrupts that the PIC generates.  The default interrupt
 * mapping conflicts with the IA32 protected-mode interrupts for indicating
 * hardware/software exceptions, so we need to map them elsewhere.
 *
 * Arguments:
 *     offset1 - vector offset for master PIC
 *               vectors on the master become offset1..offset1+7
 *     offset2 - same for slave PIC: offset2..offset2+7
 */
void PIC_remap(int offset1, int offset2) {
    unsigned char a1, a2;
 
    a1 = inb(PIC1_DATA);                        // save masks
    a2 = inb(PIC2_DATA);
 
    // starts the initialization sequence (in cascade mode)
    outb(PIC1_COMMAND, ICW1_INIT+ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT+ICW1_ICW4);
    io_wait();
    outb(PIC1_DATA, offset1);       // ICW2: Master PIC vector offset
    io_wait();
    outb(PIC2_DATA, offset2);       // ICW2: Slave PIC vector offset
    io_wait();
    outb(PIC1_DATA, 4);             // ICW3: tell Master PIC that there is a
    io_wait();                      //       slave PIC at IRQ2 (0000 0100)
    outb(PIC2_DATA, 2);             // ICW3: tell Slave PIC its cascade
    io_wait();                      // identity (0000 0010)

    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    outb(PIC1_DATA, a1);   // restore saved masks.
    outb(PIC2_DATA, a2);
}


/* Mask out a particular interrupt from being delivered. */
void IRQ_set_mask(unsigned char IRQline) {
    uint16_t port;
    uint8_t value;
 
    if(IRQline < 8) {
        port = PIC1_DATA;
    }
    else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) | (1 << IRQline);
    outb(port, value);        
}


/* Unmask a particular interrupt so that it will be delivered. */
void IRQ_clear_mask(unsigned char IRQline) {
    uint16_t port;
    uint8_t value;

    if(IRQline < 8) {
        port = PIC1_DATA;
    }
    else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) & ~(1 << IRQline);
    outb(port, value);
}


/*============================================================================
 * GENERAL INTERRUPT-HANDLING OPERATIONS
 */


/* Initialize interrupts */
void init_interrupts(void) {
    /* TODO:  INITIALIZE AND LOAD THE INTERRUPT DESCRIPTOR TABLE.
     *
     *        The entire Interrupt Descriptor Table should be zeroed out.
     *        (Unfortunately you have to do this yourself since you don't
     *        have the C Standard Library to use...)
     *
     *        Once the entire IDT has been cleared, use the lidt() function
     *        defined above to install our IDT.
     */

    /* Remap the Programmable Interrupt Controller to deliver its interrupts
     * to 0x20-0x33 (32-45), so that they don't conflict with the IA32 built-
     * in protected-mode interrupts.  (Each PIC services 7 interrupts, and
     * there is a Master PIC and a Slave PIC; they are chained together.
     * The first number says where to map the Master PIC's IRQs, and the
     * second number says where to map the Slave PIC's IRQs.)
     */
    PIC_remap(0x20, 0x27);
}


/* Installs an interrupt handler into the Interrupt Descriptor Table.
 * The handler is expected to be an assembly language handler function,
 * not a C function, although the handler might call a C function.
 */
void install_interrupt_handler(int num, void *handler) {
    /* TODO:  IMPLEMENT.  See IA32 Manual, Volume 3A, Section 5.11 for an
     *        overview of the contents of IDT Descriptors.  These are
     *        Interrupt Gates.
     *
     *        The handler address must be split into two halves, so that it
     *        can be stored into the IDT descriptor.
     *
     *        The segment selector should be the code-segment selector
     *        that was set up in the bootloader.  (See boot.h for the
     *        appropriate definition.)
     *
     *        The DPL component of the "type_attr" field specifies the
     *        required privilege level to invoke the interrupt.  You can
     *        set this to 0 (which allows anything to invoke the interrupt),
     *        but its value isn't really relevant to us.
     *
     *        REMOVE THIS COMMENT WHEN YOU WRITE THE CODE.  (FEEL FREE TO
     *        INCORPORATE THE ABOVE COMMENTS IF YOU WISH.)
     */
}


