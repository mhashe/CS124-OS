    #=====================================================================
    # A collection of handlers used by the system. Handlers included here
    # operate by saving state and then calling a corresponding c function.
    #=================================================================

    #    Handlers generally must follow this pattern:
    #
    #    .align 4
    #    .globl irqN_handler
    #    irqN_handler:
    #            pusha   # Save registers from the interrupted code!
    #            cld     # If calling C code, clear direction flag.
    #
    #            ...     # Do whatever you want to do here.
    #
    #            # Acknowledge the interrupt so that the PIC will send more
    #            # interrupts!  We do this by sending 0x20 to port 0x20.
    #            # See http://wiki.osdev.org/8259_PIC#End_of_Interrupt for
    #            # details.
    #            mov     $0x20, %al
    #            out     %al, $0x20
    #
    #            popa    # Restore all registers.
    #            iret    # Go back to interrupted code.


    # Handler for timer interrupts. 
    .align 4
    .globl irq0_handler
    irq0_handler:
            pusha   # Save registers from the interrupted code!
            cld     # If calling C code, clear direction flag.

            call timer_handler # Actually handle the interrupt.

            # Acknowledge the interrupt.
            mov     $0x20, %al
            out     %al, $0x20

            popa    # Restore all registers.
            iret    # Go back to interrupted code.


    # Handler for keyboard interrupts. 
    .align 4
    .globl irq1_handler
    irq1_handler:
            pusha   # Save registers from the interrupted code!
            cld     # If calling C code, clear direction flag.
 
            call key_handler # Actually handle the interrupt.
 
            # Acknowledge the interrupt.
            mov     $0x20, %al
            out     %al, $0x20
 
            popa    # Restore all registers.
            iret    # Go back to interrupted code.

