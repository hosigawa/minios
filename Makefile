CC = gcc
AS = gas
LD = ld
OBJCOPY = objcopy
OBJDUMP = objdump
CFLAGS = -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
ASFLAGS = -m32 -gdwarf-2 -Wa,-divide
LDFLAGS = -m $(shell $(LD) -V | grep elf_i386 2>/dev/null | head -n 1)

OBJDIR = .obj/

SRCDIR =\
		./kernel/

SRCS = $(wildcard $(addsuffix *.c, $(SRCDIR)))

OBJS = $(addprefix $(OBJDIR), $(subst ./,,$(SRCS:.c=.o)))

.PHONY: all mkobjdir makeproject q qemu

makeproject: mkobjdir minios.img

all: makeproject

q: qemu

g: qemu-gdb

qemu: makeproject
	qemu-system-x86_64 -serial mon:stdio -drive file=minios.img,index=0,media=disk,format=raw -smp 1

qemu-gdb: makeproject
	qemu-system-x86_64 -serial mon:stdio -drive file=minios.img,index=0,media=disk,format=raw -S -gdb tcp::1111

minios.img:	$(OBJDIR)bootblock $(OBJDIR)kernelblock
	dd if=/dev/zero of=$@ count=10000
	dd if=$(OBJDIR)bootblock of=$@ conv=notrunc
	dd if=$(OBJDIR)kernelblock of=minios.img seek=1 conv=notrunc

$(OBJDIR)bootblock: kernel/boot/bootasm.S kernel/boot/bootmain.c
	$(CC) $(CFLAGS) -O -nostdinc -I $(SRCDIR) -c kernel/boot/bootmain.c -o $(OBJDIR)bootmain.o
	$(CC) $(CFLAGS) -nostdinc -I $(SRCDIR) -c kernel/boot/bootasm.S -o $(OBJDIR)bootasm.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7c00 -o $(OBJDIR)bootblock.o $(OBJDIR)bootasm.o $(OBJDIR)bootmain.o
	$(OBJCOPY) -S -O binary -j .text $(OBJDIR)bootblock.o $@
	./kernel/boot/sign.pl $@

$(OBJDIR)kernelblock: $(OBJS) $(OBJDIR)entry.o $(OBJDIR)vectors.o $(OBJDIR)trapasm.o kernel/kernel.ld
	$(LD) $(LDFLAGS) -T kernel/kernel.ld -o $@ $^

kernel/vectors.S: kernel/vectors.pl
	perl kernel/vectors.pl > kernel/vectors.S

$(OBJDIR)%.o: kernel/%.S
	$(CC) $(CFLAGS) -O -nostdinc -I $(SRCDIR) -c -o $@ $<
	
$(OBJDIR)%.o: %.c
	$(CC) $(CFLAGS) -O -nostdinc -I $(SRCDIR) -c -o $@ $<

mkfs: tools/mkfs.c fs.h
	gcc -Werror -Wall -I $(SRCDIR) -o tools/mkfs $<

mkobjdir:
	@test -d $(OBJDIR) || (mkdir $(OBJDIR) && mkdir $(addprefix $(OBJDIR), $(subst ./,,$(SRCDIR))))

clean:
	rm -f minios.img
	rm -rf $(OBJDIR)

