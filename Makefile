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
		./kernel/\
		./kernel/kernel/\
		./kernel/IO/\
		./kernel/mm/\
		./kernel/fs/\
		./kernel/fs/minios/\
		./kernel/fs/proc/\

INCDIR =\
		-I ./libs/\
		-I ./kernel/include/\

SRCS = $(wildcard $(addsuffix *.c, $(SRCDIR)))
ASM_SRCS = $(wildcard $(addsuffix *.S, $(SRCDIR)))

OBJS = $(addprefix $(OBJDIR), $(subst ./,,$(SRCS:.c=.o)))
ASM_OBJS = $(addprefix $(OBJDIR), $(subst ./,,$(ASM_SRCS:.S=.o)))

COM_DIR = libs/.obj/
COM_LIBS = $(COM_DIR)libc.o $(COM_DIR)time.o

VERSION = 0.2.0

include app/applist.dep

.PHONY: all mkobjdir makeproject q qemu r m fs

makeproject: mkobjdir  minios.img

all: makeproject

r: qemu

g: qemu-gdb

c: clean

qemu: makeproject
	qemu-system-x86_64 -m 512 -serial mon:stdio -drive file=minios.img,index=0,media=disk,format=raw -smp 1

qemu-fs: makeproject
	qemu-system-x86_64 -m 512 -smp 1 -serial mon:stdio \
	-drive file=minios.img,index=0,media=disk,format=raw \
	-drive file=fs.img,index=0,media=disk,format=raw

qemu-gdb: makeproject
	qemu-system-x86_64 -m 512 -serial mon:stdio \
	-drive file=minios.img,index=0,media=disk,format=raw \
	-S -gdb tcp::1111

minios.img: $(OBJDIR)mkfs $(OBJDIR)bootblock $(OBJDIR)kernelblock
	@cd app && make && cd ../
	$< $@ $(VERSION) $(addprefix app/, $(APPS))

$(OBJDIR)bootblock: kernel/boot/bootasm.S kernel/boot/bootmain.c
	$(CC) $(CFLAGS) -O -nostdinc $(INCDIR) -c kernel/boot/bootmain.c -o $(OBJDIR)bootmain.o
	$(CC) $(CFLAGS) -nostdinc $(INCDIR) -c kernel/boot/bootasm.S -o $(OBJDIR)bootasm.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7c00 -o $(OBJDIR)bootblock.o $(OBJDIR)bootasm.o $(OBJDIR)bootmain.o
	$(OBJCOPY) -S -O binary -j .text $(OBJDIR)bootblock.o $@

$(OBJDIR)kernelblock: $(OBJS) $(ASM_OBJS) $(OBJDIR)initcode $(COM_LIBS) kernel/kernel/kernel.ld
	$(LD) $(LDFLAGS) -T kernel/kernel/kernel.ld -o $@ $(OBJS) $(ASM_OBJS) $(COM_LIBS) -b binary $(OBJDIR)initcode
	$(OBJDUMP) -S $@ > $@.asm

$(OBJDIR)%.o: %.S
	$(CC) $(CFLAGS) -O -nostdinc $(INCDIR) -c -o $@ $<
	
$(OBJDIR)%.o: %.c
	$(CC) $(CFLAGS) -O -nostdinc $(INCDIR) -c -o $@ $<
	$(OBJDUMP) -S $@ > $@.asm

$(OBJDIR)initcode: kernel/boot/initcode.S
	$(CC) $(CFLAGS) -nostdinc $(INCDIR) -c $< -o $(OBJDIR)initcode.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o $(OBJDIR)initcode.out $(OBJDIR)initcode.o
	$(OBJCOPY) -S -O binary $(OBJDIR)initcode.out $@

$(OBJDIR)app/mkfs: tools/mkfs.c kernel/include/mkfs.h
	gcc -Werror -Wall -o $@ $<

$(COM_DIR)%.o: libs/%.c
	$(CC) $(CFLAGS) -O -nostdinc $(INCDIR) -c -o $@ $<
	$(OBJDUMP) -S $@ > $@.asm

$(OBJDIR)mkfs: tools/mkfs.c
	gcc -Werror -Wall -o $@ $<

mkobjdir:
	@test -d $(COM_DIR) || mkdir $(COM_DIR)
	@test -d $(OBJDIR) || (mkdir $(OBJDIR) && mkdir $(addprefix $(OBJDIR), $(subst ./,,$(SRCDIR))))

clean:
	rm -f minios.img
	rm -rf $(OBJDIR)
	rm -rf $(COM_DIR)
	@cd app && make c && cd ../

