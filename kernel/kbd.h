#ifndef __KBD_H__
#define __KBD_H__

#define KEY_STA 0x64
#define KEY_CMD 0x64
#define KEY_DATA 0x60
#define KEYBOARD_READY 0x2

#define NO 0
// Special keycodes
#define KEY_HOME        0xE0
#define KEY_END         0xE1
#define KEY_UP          0xE2
#define KEY_DN          0xE3
#define KEY_LF          0xE4
#define KEY_RT          0xE5
#define KEY_PGUP        0xE6
#define KEY_PGDN        0xE7
#define KEY_INS         0xE8
#define KEY_DEL         0xE9

#define C(x) ((x) - '@')

void init_kdb();
void wait_kbc_ready();
int kbd_getc();
void kbd_proc();

#endif

