#ifndef INTERRUPTS_H
#define INTERRUPTS_H


/* These are the interrupt numbers that the timer and keyboard will
 * generate when they fire.  Confusingly, the timer interrupt is IRQ0
 * on the Programmable Interrupt Controller, and the keyboard interrupt
 * is IRQ1.  But, during setup, they are mapped to interrupts 0x20 and
 * 0x21, correspondingly.
 */
#define TIMER_INTERRUPT 0x20
#define KEYBOARD_INTERRUPT 0x21


#define LOWER_16_MASK 0x0000FFFF


/* Initialize interrupts. */
void init_interrupts(void);


/* Installs an interrupt handler into the Interrupt Descriptor Table.
 * The handler is expected to be an assembly language handler function,
 * not a C function, although the handler might call a C function.
 */
void install_interrupt_handler(int num, void *handler);


/* Returns true if interrupts are currently enabled, false otherwise. */
static inline int are_interrupts_enabled() {
    unsigned long flags;
    asm volatile ( "pushf\n\t"
                   "pop %0"
                   : "=g"(flags) );
    return flags & (1 << 9);
}


/* Disable interrupt delivery.  Returns a flag indicating whether interrupts
 * were previously enabled.
 */
static inline int disable_interrupts(void) {
    int enabled = are_interrupts_enabled();
    asm ( "cli" );
}


/* Enables interrupt delivery. */
static inline void enable_interrupts(void) {
    asm ( "sti" );
}


#endif /* INTERRUPTS_H */

