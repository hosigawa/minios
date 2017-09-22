#include "elf.h"
#include "type.h"
#include "x86.h"

#define KERNEL_IMG_OFF (62*2)

void wait_disk() {
	while((inb(0x1f7) & 0xc0) != 0x40);
}

void read_disk(uchar *va, int offset) {
	wait_disk();
	outb(0x1F2, 1);
	outb(0x1F3, offset);
	outb(0x1F4, offset >> 8); 
	outb(0x1F5, offset >> 16);
	outb(0x1F6, (offset >> 24) | 0xE0);
	outb(0x1F7, 0x20);

	wait_disk();
	insl(0x1F0, va, 512/4);
}

void read_seg(uchar *va, int size, int offset) {
	uchar *end = va + size;
	va -= offset % 512;
	offset = offset / 512 + KERNEL_IMG_OFF;
	for(; va < end; va += 512, offset++) {
		if(offset - KERNEL_IMG_OFF >= 24)
			read_disk(va, offset + 2);
		else
			read_disk(va, offset);
	}
}

void bootmain() {
	struct elfhdr *elf_hdr;
	struct proghdr *ph;

	elf_hdr = (struct elfhdr *)0x10000;

	read_seg((uchar *)elf_hdr, 4096, 0);

	if(elf_hdr->magic != ELF_MAGIC) {
		return;
	}

	ph = (struct proghdr *)((uchar *)elf_hdr + elf_hdr->phoff);
	struct proghdr *ph_end = ph + elf_hdr->phnum;

	for(; ph < ph_end; ph++) {
		uchar *pa = (uchar *)ph->pa;
		read_seg(pa, ph->filesz, ph->offset);
		if(ph->memsz > ph->filesz) {
			stosb(pa+ ph->filesz, 0, ph->memsz - ph->filesz);
		}
	}
	
	void (*entry)(void) = (void(*)(void))elf_hdr->entry;
	entry();
}

