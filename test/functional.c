#include <stdio.h>
#include "cpu.h"

int tick_ctr = 0;
cpu_state_t cpu;

void tick_fn() {
    tick_ctr++;
    if (tick_ctr%100 == 0) printf(".");
    if (tick_ctr%1000 == 0) {
        printf("%x\n", (u16)cpu.PC);
    }
}

int main(int argc, char** argv) {

    cpu.tick = &tick_fn;
    u8 mem[0xFFFF];
    
    FILE *f = fopen(argv[1], "rb");
    printf("%s\n", argv[1]);
    fgets((char*)mem, 0xFFFF, f);
    fclose(f);

    printf("Read bytes from memory\n");

    // cpu_reset(&cpu, mem);
    cpu.PC = 0x400;
    cpu.S = 0xFF;
    int n_instrs = 5000;
    int i = 0;
    for (; i<n_instrs; i++) {
        u16 prev_pc = cpu.PC;
        int res = cpu_exec(&cpu, mem);
        if (cpu.PC == prev_pc) {
            printf("PC trapped at %x\n", prev_pc);
            break;
        }
        if (res != 0) {
            printf("Error at PC:%x, ret with code %d\n", cpu.PC, res);
            break;
        }
    }
    printf("DONE executed %d instrs\n", i);

    return 0;
}

