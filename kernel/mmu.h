#ifndef __MMU_H__
#define __MMU_H__

#define DPL_USER    0x3     // User DPL

// Application segment type bits
#define STA_X       0x8     // Executable segment
#define STA_E       0x4     // Expand down (non-executable segments)
#define STA_C       0x4     // Conforming code segment (executable only)
#define STA_W       0x2     // Writeable (non-executable segments)
#define STA_R       0x2     // Readable (executable segments)
#define STA_A       0x1     // Accessed

// System segment type bits
#define STS_T16A    0x1     // Available 16-bit TSS
#define STS_LDT     0x2     // Local Descriptor Table
#define STS_T16B    0x3     // Busy 16-bit TSS
#define STS_CG16    0x4     // 16-bit Call Gate
#define STS_TG      0x5     // Task Gate / Coum Transmitions
#define STS_IG16    0x6     // 16-bit Interrupt Gate
#define STS_TG16    0x7     // 16-bit Trap Gate
#define STS_T32A    0x9     // Available 32-bit TSS
#define STS_T32B    0xB     // Busy 32-bit TSS
#define STS_CG32    0xC     // 32-bit Call Gate
#define STS_IG32    0xE     // 32-bit Interrupt Gate
#define STS_TG32    0xF     // 32-bit Trap Gate

#define SEG_KCODE 1
#define SEG_KDATA 2
#define SEG_UCODE 3
#define SEG_UDATA 4

#define V2P(a) (((uint) (a)) - KERN_BASE)
#define P2V(a) (((void *) (a)) + KERN_BASE)

#define V2P_NC(x) ((x) - KERN_BASE)
#define P2V_NC(x) ((x) + KERN_BASE)

#define CR0_WP 0x00010000
#define CR0_PG 0X80000000

#define CR4_PSE 0x00000010

#define PG_SIZE 4096
#define IO_END 0x100000
#define KERN_END 0xE000000
#define KERN_BASE 0x80000000
#define KERN_LINK 0x80100000
#define DEV_SPACE 0xFE000000

#define KSTACKSIZE 4096

#define PG_ROUNDUP(sz)  (((sz)+PG_SIZE-1) & ~(PG_SIZE-1))
#define PG_ROUNDDOWN(a) ((char*)((((unsigned int)(a)) & ~(PG_SIZE-1))))

// Page table/directory entry flags.
#define PTE_P           0x001   // Present
#define PTE_W           0x002   // Writeable
#define PTE_U           0x004   // User
#define PTE_PWT         0x008   // Write-Through
#define PTE_PCD         0x010   // Cache-Disable
#define PTE_A           0x020   // Accessed
#define PTE_D           0x040   // Dirty
#define PTE_PS          0x080   // Page Size
#define PTE_MBZ         0x180   // Bits must be zero

#endif

