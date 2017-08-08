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
ASM_SRCS = $(wildcard $(addsuffix *.S, $(SRCDIR)))

OBJS = $(addprefix $(OBJDIR), $(subst ./,,$(SRCS:.c=.o)))
ASM_OBJS = $(addprefix $(OBJDIR), $(subst ./,,$(ASM_SRCS:.S=.o))) $(OBJDIR)kernel/vectors.o

.PHONY: all mkobjdir makeproject q qemu

makeproject: mkobjdir kernel/vectors.S minios.img

all: makeproject

q: qemu

g: qemu-gdb

c: clean

qemu: makeproject
	qemu-system-x86_64 -m 512 -serial mon:stdio -drive file=minios.img,index=0,media=disk,format=raw -smp 1

qemu-gdb: makeproject
	qemu-system-x86_64 -m 512 -serial mon:stdio -drive file=minios.img,index=0,media=disk,format=raw -S -gdb tcp::1111

minios.img:	$(OBJDIR)bootblock $(OBJDIR)kernelblock
	dd if=/dev/zero of=$@ count=10000
	dd if=$(OBJDIR)bootblock of=$@ conv=notrunc
	dd if=$(OBJDIR)kernelblock of=minios.img seek=1 conv=notrunc

$(OBJDIR)bootblock: kernel/boot/bootasm.S kernel/boot/bootmain.c
	$(CC) $(CFLAGS) -O -nostdinc -I $(SRCDIR) -c kernel/boot/bootmain.c -o $(OBJDIR)bootmain.o
	$(CC) $(CFLAGS) -nostdinc -I $(SRCDIR) -c kernel/boot/bootasm.S -o $(OBJDIR)bootasm.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7c00 -o $(OBJDIR)bootblock.o $(OBJDIR)bootasm.o $(OBJDIR)bootmain.o
	$(OBJCOPY) -S -O binary -j .text $(OBJDIR)bootblock.o $@
	perl ./tools/sign.pl $@

$(OBJDIR)kernelblock: $(OBJS) $(ASM_OBJS) $(OBJDIR)initcode kernel/kernel.ld
	$(LD) $(LDFLAGS) -T kernel/kernel.ld -o $@ $(OBJS) $(ASM_OBJS) -b binary $(OBJDIR)initcode

kernel/vectors.S: tools/vectors.pl
	perl tools/vectors.pl > kernel/vectors.S

$(OBJDIR)%.o: %.S
	$(CC) $(CFLAGS) -O -nostdinc -I $(SRCDIR) -c -o $@ $<
	
$(OBJDIR)%.o: %.c
	$(CC) $(CFLAGS) -O -nostdinc -I $(SRCDIR) -c -o $@ $<

$(OBJDIR)initcode: kernel/boot/initcode.S
	$(CC) $(CFLAGS) -nostdinc -I$(SRCDIR) -c $< -o $(OBJDIR)initcode.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o $(OBJDIR)initcode.out $(OBJDIR)initcode.o
	$(OBJCOPY) -S -O binary $(OBJDIR)initcode.out $@

mkfs: tools/mkfs.c fs.h
	gcc -Werror -Wall -I $(SRCDIR) -o tools/mkfs $<

mkobjdir:
	@test -d $(OBJDIR) || (mkdir $(OBJDIR) && mkdir $(addprefix $(OBJDIR), $(subst ./,,$(SRCDIR))))

clean:
	rm -f minios.img
	rm -rf $(OBJDIR)

