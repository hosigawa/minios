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

LIBDIR = ./ulibs/\

SRCDIR = ./src/\
		$(LIBDIR)

INCDIR = -I./src\
		-I./ulibs\
		 -I../kernel/include\
		 -I../libs\

SRCS = $(wildcard $(addsuffix *.c, $(SRCDIR)))

LIB_C_SRCS = $(wildcard $(addsuffix *.c, $(LIBDIR)))
LIB_ASM_SRCS = $(wildcard $(addsuffix *.S, $(LIBDIR)))

OBJS = $(addprefix $(OBJDIR), $(subst ./,,$(SRCS:.c=.o)))

LIB_C_OBJS = $(addprefix $(OBJDIR), $(subst ./,,$(LIB_C_SRCS:.c=.o)))
LIB_ASM_OBJS = $(addprefix $(OBJDIR), $(subst ./,,$(LIB_ASM_SRCS:.S=.o)))

COM_DIR = ../libs/.obj/
COM_LIBS = $(COM_DIR)libc.o $(COM_DIR)time.o

include applist.dep

.PHONY: all mkobjdir

all: mkobjdir $(APPS)

c: clean

$(OBJDIR)%.o: %.c
	$(CC) $(CFLAGS) -O -nostdinc $(INCDIR) -c -o $@ $<
	$(OBJDUMP) -S $@ > $@.asm

$(OBJDIR)%.o: %.S
	$(CC) $(CFLAGS) -O -nostdinc $(INCDIR) -c -o $@ $<

_%: $(OBJDIR)src/%.o $(LIB_C_OBJS) $(LIB_ASM_OBJS) $(COM_LIBS)
	$(LD) $(LDFLAGS) -N -e _start -Ttext 0x10000 -o $@ $^
	$(OBJDUMP) -S $@ > $(OBJDIR)$@.asm

$(COM_DIR)%.o: ../libs/%.c
	$(CC) $(CFLAGS) -O -nostdinc $(INCDIR) -c -o $@ $<
	$(OBJDUMP) -S $@ > $@.asm

mkobjdir:
	@test -d $(COM_DIR) || mkdir $(COM_DIR)
	@test -d $(OBJDIR) || (mkdir $(OBJDIR) && mkdir $(addprefix $(OBJDIR), $(subst ./,,$(SRCDIR))))

clean:
	rm -f fs.img
	rm -f _*
	rm -rf $(OBJDIR)

