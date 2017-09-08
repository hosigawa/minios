#include "kernel.h"

__attribute__((__aligned__(PG_SIZE)))
pde_t super_pgdir[1024] = {
  [0] = (0) | PTE_P | PTE_W | PTE_PS,
  [KERN_BASE >> 22] = (0) | PTE_P | PTE_W | PTE_PS,
  [(KERN_BASE >> 22) + 1] = (0x400000) | PTE_P | PTE_W | PTE_PS,
};

extern struct CPU cpu;
struct gdt_desc gdt[8];
struct taskstate ts;
pde_t *k_dir = NULL;

extern char data[];

struct kernel_map kmap[] = {
	{ (void*)KERN_BASE, 0, IO_END, PTE_W}, // I/O space
	{ (void*)KERN_LINK, V2P(KERN_LINK), V2P(data), 0},     // kern text+rodata
	{ (void*)data, V2P(data), min(PHYSICAL_END, LOW_MEM), PTE_W},     // kern text+rodata
	{ (void*)DEV_SPACE, DEV_SPACE, 0, PTE_W}, // more devices
};

void init_gdt() 
{
	gdt[SEG_KCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, DPL_KERN);
	gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, DPL_KERN);
	gdt[SEG_UCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, DPL_USER);
	gdt[SEG_UDATA] = SEG(STA_W, 0, 0xffffffff, DPL_USER);

	lgdt(gdt, sizeof(gdt));
}

void init_kvm() 
{
	k_dir = (pde_t *)kalloc();
	if(!k_dir)
		panic("init_kvm error\n");
	memset(k_dir, 0, PG_SIZE);
	int size = sizeof(kmap) / sizeof(kmap[0]);
	int i = 0;
	struct kernel_map *km;
	for(; i < size; i++) {
		km = kmap + i;
		map_page(k_dir, km->va, km->pa_start, km->pa_end-km->pa_start, km->perm);
	}
	swtch_kvm();
}

void swtch_kvm() 
{
	lcr3(V2P(k_dir));
}

pde_t *set_kvm()
{
	pde_t *pdir = (pde_t *)kalloc();
	if(!pdir)
		panic("set_kvm error\n");
	memset(pdir, 0, PG_SIZE);
	memcpy(pdir + 0x300, k_dir + 0x300, 0x400);
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
			panic("map_page PTE doesn't exists\n");
		if(*pte & PTE_P)
			panic("map_page PTE %p has mapped %p\n", va_s, *pte & ~0xfff);
		*pte = la | PTE_P | perm;
		if(va_s == va_e)
			break;
	}
}

uint unmap(pde_t *pdir, void *va)
{
	uint addr;
	pte_t *pte = get_pte(pdir, va, false);
	if(pte && *pte & PTE_P) {
		addr = *pte & ~0xfff;
		*pte = 0;
		return addr;
	}
	return 0;
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
		pttab = (pte_t *)kalloc();
		if(!pttab)
			panic("create PTE error\n");
		memset(pttab, 0, PG_SIZE);
		*pde = (uint)V2P(pttab) | PTE_P | PTE_W | PTE_U;
	}
	return (pte_t *)(pttab + (((uint)va >> 12) & 0x3ff));
}

void swtch_uvm(struct proc *p)
{
	if(!p)
		panic("swtch_uvm\n");
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
	if(size > PG_SIZE)
		panic("init_uvm: binary size is too big\n");
	char *pg = kalloc();
	if(!pg)
		panic("init_uvm: kalloc error\n");
	memset(pg, 0, PG_SIZE);
	map_page(pdir, 0, V2P(pg), PG_SIZE, PTE_W|PTE_U);
	memcpy(pg, start, size);
	return 0;
}

pde_t *cp_uvm(pde_t *pgdir, int mem_size) 
{
	uint new_mem, old_mem;
	uint flag;
	pde_t *new_pg = set_kvm();
	if(!new_pg)
		return NULL;
	uint i = USER_LINK;
	for(; i < (uint)mem_size; i += PG_SIZE) {
		pte_t *pte = get_pte(pgdir, (char *)i, false);
		if(!pte) 
			panic("cp_uvm pde not exist, va:%p\n", i);
		if(!(*pte & PTE_P))
			panic("cp_uvm pte not exist, va:%p\n", i);
		new_mem = valloc();
		if(!new_mem) {
			panic("free uvm\n");
			free_uvm(new_pg);
			return NULL;
		}
		old_mem = *pte & ~0xfff;
		flag = *pte & 0xfff;
		copy_user_page(new_mem, old_mem);
		map_page(new_pg, (char *)i, new_mem, PG_SIZE, flag);
	}
	return new_pg;
}

void free_uvm(pde_t *pgdir)
{
	resize_uvm(pgdir, KERN_BASE, 0);
	int i = 0;
	for(; i < 768; i++) {
		if(pgdir[i] & PTE_P)
			kfree(P2V(pgdir[i] & ~0xfff));
	}
	kfree(pgdir);
}

int resize_uvm(pde_t *pgdir, uint oldsz, uint newsz)
{
	if(oldsz < USER_LINK)
		panic("resize_uvm: oldsz < USER_LINK\n");
	if(newsz > KERN_BASE)
		return -1;
	char *i = 0;
	char *old_addr = (char *)PG_ROUNDUP(oldsz);
	char *new_addr = (char *)PG_ROUNDUP(newsz);
	uint mem;
	if(old_addr <= new_addr) {
		for(i = old_addr; i < new_addr; i += PG_SIZE) {
			mem = valloc();
			if(!mem) {
				return -1;
			}
			map_page(pgdir, i, mem, PG_SIZE, PTE_W|PTE_U);
		}
	}
	else {
		for(i = new_addr; i < old_addr; i += PG_SIZE) {
			if((mem = unmap(pgdir, i)) == 0) {
				if(i < (char *)USER_LINK)
					continue;
				else
					break;
			}
			else {
				vfree(mem);
			}
		}
	}
	return (int)new_addr;
}

int load_uvm(pde_t *pdir, struct inode *ip, char *va, int off, int len)
{
	if((uint)va % PG_SIZE != 0) {
		panic("va not page aligned\n");
		return -1;
	}
	pte_t *pte;
	uint addr;
	int i;
	for(i = 0; i < len; i += PG_SIZE) {
		pte = get_pte(pdir, va + i, false);
		if(!pte)
			panic("load_uvm pde not exists\n");
		if(!(*pte & PTE_P))
			panic("load_uvm pte not exists\n");
		addr = *pte & ~0xfff;
		kmap_atomic(USER0, addr);
		if(len - i < PG_SIZE)
			readi(ip, USER0, off + i, len - i);
		else
			readi(ip, USER0, off + i, PG_SIZE);
		unkmap_atomic(USER0);
	}
	return 0;
}

void clear_pte(pde_t *pdir, void *va)
{
	pte_t *pte = get_pte(pdir, va, false);
	if(pte && *pte & PTE_P) {
		*pte &= ~PTE_U;
	}
}

int copy_out(pde_t *pdir, char *dst, char *src, int len)
{
	int i;
	pte_t *pte;
	char *va;
	uint pa;
	int intr = 0;
	int off;
	for(i = intr; i < len; i += intr, dst += intr, src += intr) {
		va = PG_ROUNDDOWN(dst);
		off = (uint)dst - (uint)va;
		intr = min(len - i, PG_SIZE - off);
		pte = get_pte(pdir, va, false);
		if(!pte || !(*pte & PTE_P) || !(*pte & PTE_U)) {
			return -1;
		}
		pa = *pte & ~0xfff;
		copy_to_user(pa, off, src, intr);
	}
	return 0;
}

void do_page_fault()
{
	uint p = valloc();
	if(!p)
		panic("physical memory not enough\n");
	uint va = read_cr2();
	map_page(cpu.cur_proc->pgdir, (void *)va, p, PG_SIZE, PTE_W);
}

void copy_user_page(uint dst, uint src)
{
	pushcli();
	kmap_atomic(USER0, dst);
	kmap_atomic(USER1, src);
	memcpy(USER0, USER1, PG_SIZE);
	unkmap_atomic(USER0);
	unkmap_atomic(USER1);
	popsti();
}

void copy_to_user(uint dst, int off, char *src, int len)
{
	pushcli();
	if(len > PG_SIZE)
		panic("copy_user_page len > PG_SIZE\n");
	kmap_atomic(USER0, dst);
	memcpy(USER0 + off, src, len);
	unkmap_atomic(USER0);
	popsti();
}

void kmap_atomic(char *va, uint mem)
{
	pde_t *pgdir = cpu.cur_proc ? cpu.cur_proc->pgdir : k_dir;
	map_page(pgdir, va, mem, PG_SIZE, PTE_W);
	invlpg(va);
}

void unkmap_atomic(char *va)
{
	pde_t *pgdir = cpu.cur_proc ? cpu.cur_proc->pgdir : k_dir;
	unmap(pgdir, va);
	invlpg(va);
}

