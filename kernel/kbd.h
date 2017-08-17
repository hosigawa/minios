#ifndef __KBD_H__
#define __KBD_H__

#define KEY_STA 0x64
#define KEY_CMD 0x64
#define KEY_DATA 0x60
#define KEYBOARD_READY 0x2

void init_kdb();
void wait_kbc_ready();
void kbd_proc();

#endif

