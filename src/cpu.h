#ifndef __CPU_H__
#define __CPU_H__

#include <inttypes.h>

typedef uint16_t u16;
typedef uint8_t u8;
typedef uint32_t u32;

typedef struct {
    u8 N: 1;
    u8 V: 1;
    u8 u: 1;
    u8 B: 1;
    u8 D: 1;
    u8 I: 1;
    u8 Z: 1;
    u8 C: 1;
} cpu_sr_t;

typedef struct {
    u8 A;
    u8 Y;
    u8 X;
    u16 PC;
    u8 S;
    cpu_sr_t P;

    void (*tick)(void); 

} cpu_state_t;

void cpu_exec(cpu_state_t *st, u8* mem);

#endif
