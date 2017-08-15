#include "kernel.h"

struct gdt_desc gdt[8];
struct taskstate ts;
pde_t *k_dir;

extern char data[];

struct kernel_map kmap[] = {
	{ (void*)KERN_BASE, 0,             IO_END,    PTE_W}, // I/O space
	{ (void*)KERN_LINK, V2P(KERN_LINK), V2P(data), 0},     // kern text+rodata
	{ (void*)data,     V2P(data),     PHYSICAL_END,   PTE_W}, // kern data+memory
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
	k_dir = set_kvm();
	swtch_kvm();
}

void swtch_kvm() 
{
	lcr3(V2P(k_dir));
}

pde_t *set_kvm()
{
	pde_t *pdir = (pde_t *)mem_alloc();
	if(pdir == 0)
		panic("set_kvm error\n");
	memset(pdir, 0, PG_SIZE);
	int size = sizeof(kmap) / sizeof(kmap[0]);
	int i = 0;
	struct kernel_map *km;
	for(; i < size; i++) {
		km = kmap + i;
		map_page(pdir, km->va, km->pa_start, km->pa_end-km->pa_start, km->perm);
	}
	return pdir;
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
			return NULL;
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
	pushcli();
	gdt[SEG_TSS] = SEG16(STS_T32A, &ts, sizeof(ts)-1, 0);
	gdt[SEG_TSS].s = 0;
	ts.ss0 = SEG_KDATA << 3;
	ts.esp0 = p->kstack + KSTACKSIZE;
	ts.iomb = (ushort) 0xFFFF;
	ltr(SEG_TSS << 3);	
	
	lcr3(V2P(p->pgdir));
	popsti();
}

int init_uvm(pde_t *pdir, char *start, int size)
{
	char *pg = mem_alloc();
	memset(pg, 0, PG_SIZE);
	map_page(pdir, 0, V2P(pg), PG_SIZE, PTE_W|PTE_U);
	memmove(pg, start, size);
	return 0;
}

pde_t *cp_uvm(pde_t *pgdir, int mem_size) 
{
	char *new_mem, *old_mem;
	uint flag;
	pde_t *new_pg = set_kvm();
	if(!new_pg)
		return NULL;
	uint i = 0;
	for(; i < (uint)mem_size; i += PG_SIZE) {
		pte_t *pte = get_pte(pgdir, (char *)i, false);
		if(!pte) 
			panic("cp_uvm pde not exist\n");
		if(!(*pte & PTE_P))
			panic("cp_uvm pte not exist");
		new_mem = mem_alloc();
		if(!new_mem) {
			panic("free uvm\n");
			free_uvm(new_pg);
			return NULL;
		}
		old_mem = (char *)P2V(*pte & ~0xfff);
		flag = (uint)(*pte & 0xfff);
		memmove(new_mem, old_mem, PG_SIZE);
		map_page(new_pg, (char *)i, V2P(new_mem), PG_SIZE, flag);
	}
	return new_pg;
}

void free_uvm(pde_t *pgdir)
{
	resize_uvm(pgdir, KERN_BASE, 0);
	int i = 0;
	for(; i < 1024; i++) {
		if(pgdir[i] & PTE_P)
			mem_free(P2V(pgdir[i] & ~0xfff));
	}
	mem_free(pgdir);
}

int resize_uvm(pde_t *pgdir, uint oldsz, uint newsz)
{
	if(newsz > KERN_BASE)
		return -1;
	char *i = 0;
	char *old_addr = (char *)PG_ROUNDUP(oldsz);
	char *new_addr = (char *)PG_ROUNDUP(newsz);
	char *mem;
	pte_t *pte;
	if(old_addr <= new_addr) {
		for(i = old_addr; i < new_addr; i += PG_SIZE) {
			mem = mem_alloc();
			if(!mem) {
				return -1;
			}
			map_page(pgdir, i, V2P(mem), PG_SIZE, PTE_W|PTE_U);
		}
	}
	else {
		for(i = new_addr; i < old_addr; i += PG_SIZE) {
			pte = get_pte(pgdir, i, false);
			if(pte && *pte & PTE_P) {
				mem_free(P2V(*pte & ~0xfff));
				*pte = 0;
			}
			else {
				break;
			}
		}
	}
	return newsz;
}

int load_uvm(pde_t *pdir, struct inode *ip, char *va, int off, int len)
{
	if((uint)va % PG_SIZE != 0) {
		panic("va not page aligned\n");
		return -1;
	}
	uint addr;
	pte_t *pte;
	int i;
	for(i = 0; i < len; i += PG_SIZE) {
		pte = get_pte(pdir, va + i, false);
		if(!pte)
			panic("load_uvm pde not exists\n");
		if(!(*pte & PTE_P))
			panic("load_uvm pte not exists\n");
		addr = *pte & ~0xfff;
		if(len - i < PG_SIZE)
			readi(ip, P2V(addr), off + i, len - i);
		else
			readi(ip, P2V(addr), off + i, PG_SIZE);
	}
	return 0;
}

