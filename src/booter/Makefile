# IMPORTANT NOTE:
#     Most files in this project are run through the C preprocessor
#     before they are processed - even the assembly language boot.S
#     file is processed this way.  This allows us to use header files
#     to hold widely used constants.  The consequence of this is that
#     we invoke gcc to assemble boot.S, and we use gcc to process the
#     linker script, and so forth.

CFLAGS=-g

BOOT_BIN = boot.bin

OBJS = game.o video.o interrupts.o handlers.o timer.o keyboard.o
PROGRAM_OBJ = program.o
PROGRAM_BIN = program.bin

FLOPPY_IMG = floppy.img


all : clean $(FLOPPY_IMG)

debug: clean $(FLOPPY_IMG) debug-qemu


# Use gcc to build our assembly code so that we can use the C preprocessor
# to #define constants for our bootloader.  The "-Wa,--gstabs" argument tells
# gcc to pass "--gstabs" to the assembler.
boot.o: boot.S boot.h
	gcc -c boot.S -Wa,--gstabs

# Build the bootloader binary file.
$(BOOT_BIN): boot.o
	$(LD) -N -e 0 -Ttext 0x7c00 --oformat binary -o $@ $^

# Build the main program binary file.  We take the final .o file and strip out
# all the stuff we don't care about, so we end up with a simple ELF file.

# Run the C preprocessor on the linker script, to apply definitions in boot.h
# Note:  CPP is the C compiler, not "cpp", so we need to specify the input and
#        output files differently from what cpp requires.
prog.lds.s: CPPFLAGS += -P
prog.lds.s: program.lds.S boot.h
	$(CPP) $(CPPFLAGS) -o $@ $<

# $< is the first prerequisite, so that has to be the linker script.
$(PROGRAM_OBJ): prog.lds.s $(OBJS)
	$(LD) -T $< -o $@ $(OBJS)

$(PROGRAM_BIN): $(PROGRAM_OBJ)
	objcopy -R .note -R .comment -S $< $@

$(FLOPPY_IMG): $(BOOT_BIN) $(PROGRAM_BIN)
	cat $(BOOT_BIN) $(PROGRAM_BIN) > $@
	truncate -s 1474560 $@

run-qemu: #$(FLOPPY_IMG)
	qemu-system-i386 -soundhw pcspk -drive file=$(FLOPPY_IMG),format=raw

debug-qemu: #$(FLOPPY_IMG)
	qemu-system-i386 -soundhw pcspk -s -S -drive file=$(FLOPPY_IMG),format=raw

# Currently compiling floppy.img on VM, running natively.
clean:
	rm -f *.o prog.lds.s $(BOOT_BIN) $(PROGRAM_BIN) $(FLOPPY_IMG)

.PHONY: all clean run-qemu debug-qemu

