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

SRCDIR = ./src/\
		./ulibs/

INCDIR = -I./src\
		-I./ulibs\
		-I../kernel\

SRCS = $(wildcard $(addsuffix *.c, $(SRCDIR)))
ASM_SRCS = $(wildcard $(addsuffix *.S, $(SRCDIR)))

OBJS = $(addprefix $(OBJDIR), $(subst ./,,$(SRCS:.c=.o)))
ASM_OBJS = $(addprefix $(OBJDIR), $(subst ./,,$(ASM_SRCS:.S=.o)))

APPS =\
	_init

.PHONY: all mkobjdir mkfs

all: mkobjdir fs.img

c: clean

fs.img: $(OBJDIR)mkfs $(APPS)
	$< $@ $(APPS)
	@cp -f $@ ../

$(OBJDIR)%.o: %.c
	$(CC) $(CFLAGS) -O -nostdinc $(INCDIR) -c -o $@ $<

$(OBJDIR)%.o: %.S
	$(CC) $(CFLAGS) -O -nostdinc $(INCDIR) -c -o $@ $<

_%: $(OBJS) $(ASM_OBJS)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^

$(OBJDIR)mkfs: ../tools/mkfs.c ../kernel/fs.h
	gcc -Werror -Wall -I ../kernel -o $@ $<

mkobjdir:
	@test -d $(OBJDIR) || (mkdir $(OBJDIR) && mkdir $(addprefix $(OBJDIR), $(subst ./,,$(SRCDIR))))

clean:
	rm -f fs.img
	rm -f _*
	rm -rf $(OBJDIR)

