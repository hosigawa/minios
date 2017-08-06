#include "kernel.h"

struct gdt_desc gdt[8];
struct taskstate ts;
pde_t *k_dir;

extern char data[];

struct kernel_map kmap[] = {
	{ (void*)KERN_BASE, 0,             IO_END,    PTE_W}, // I/O space
	{ (void*)KERN_LINK, V2P(KERN_LINK), V2P(data), 0},     // kern text+rodata
	{ (void*)data,     V2P(data),     PYSICAL_END,   PTE_W}, // kern data+memory
	{ (void*)DEV_SPACE, DEV_SPACE,      0,         PTE_W}, // more devices
};

void init_gdt() 
{
	gdt[SEG_KCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, 0);
	gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0);
	gdt[SEG_UCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, DPL_USER);
	gdt[SEG_UDATA] = SEG(STA_W, 0, 0xffffffff, DPL_USER);

	lgdt(gdt, sizeof(gdt));
}

void init_kvm() 
{
	k_dir = (pde_t *)mem_alloc();
	if(k_dir == 0)
		panic("init_kvm error\n");
	memset(k_dir, 0, PG_SIZE);
	set_kvm(k_dir);
	swtch_kvm();
}

void swtch_kvm() 
{
	lcr3(V2P(k_dir));
}

void set_kvm(pde_t *pdir)
{
	int size = sizeof(kmap) / sizeof(kmap[0]);
	int i = 0;
	struct kernel_map *km;
	for(; i < size; i++) {
		km = kmap + i;
		map_page(pdir, km->va, km->pa_start, km->pa_end-km->pa_start, km->perm);
	}
}

void map_page(pde_t *pdir, void *va, uint la, uint size, int perm)
{
	char *va_s = (char *)PG_ROUNDDOWN((uint)va);
	char *va_e = (char *)PG_ROUNDDOWN(((uint)va) + size - 1);
	pte_t *pte;
	for(;va_s <= va_e; va_s += PG_SIZE, la += PG_SIZE) {
		pte = get_pte(pdir, va_s, true);
		if(!pte)
			panic("PTE doesn't exists\n");
		if(*pte & PTE_P)
			panic("PTE %p has mapped\n", va_s);
		*pte = la | PTE_P | perm;
		if(va_s == va_e)
			break;
	}
}

pte_t *get_pte(pde_t *pdir, void *va, bool bcreate)
{
	pde_t *pde;
	pte_t *pttab;
	pde = (pde_t *)(pdir + (((uint)va >> 22) & 0x3ff));
	if(*pde & PTE_P) {
		pttab = (pte_t *)P2V((*pde) & ~0xfff);
	}
	else {
		if(!bcreate)
			return 0;
		pttab = (pte_t *)mem_alloc();
		if(!pttab)
			panic("create PTE error\n");
		memset(pttab, 0, PG_SIZE);
		*pde = (uint)V2P(pttab) | PTE_P | PTE_W | PTE_U;
	}
	return (pte_t *)(pttab + (((uint)va >> 12) & 0x3ff));
}

void swtch_uvm(struct proc *p)
{
	cli();
	gdt[SEG_TSS] = SEG16(STS_T32A, &ts, sizeof(ts)-1, 0);
	gdt[SEG_TSS].s = 0;
	ts.ss0 = SEG_KDATA << 3;
	ts.esp0 = p->kstack + KSTACKSIZE;
	ts.iomb = (ushort) 0xFFFF;
	ltr(SEG_TSS << 3);	
	
	lcr3(V2P(p->pgdir));
	sti();
}

