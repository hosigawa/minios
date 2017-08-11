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
		./\

SRCS = $(wildcard $(addsuffix *.c, $(SRCDIR)))

OBJS = $(addprefix $(OBJDIR), $(subst ./,,$(SRCS:.c=.o)))

APPS =\
	init.c

.PHONY: all mkobjdir mkfs

all: mkobjdir fs.img

c: clean

fs.img: $(OBJDIR)mkfs $(APPS)
	$< $@ $(APPS)
	@cp -f $@ ../

$(OBJDIR)%.o: %.c
	$(CC) $(CFLAGS) -O -nostdinc -I $(SRCDIR) -c -o $@ $<

$(OBJDIR)mkfs: ../tools/mkfs.c ../kernel/fs.h
	gcc -Werror -Wall -I ../kernel -o $@ $<

mkobjdir:
	@test -d $(OBJDIR) || (mkdir $(OBJDIR) && mkdir $(addprefix $(OBJDIR), $(subst ./,,$(SRCDIR))))

clean:
	rm -f fs.img
	rm -rf $(OBJDIR)

