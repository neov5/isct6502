#include <stdio.h>
#include "cpu.h"

int tick_ctr = 0;
int inst_ctr = 0;
cpu_state_t cpu;

void tick_fn() {
    // if (inst_ctr == 41798 || inst_ctr == 41799) printf("%d\n", cpu.P.C);
    // if (tick_ctr%100 == 0) printf(".");
    // if (tick_ctr%1000 == 0) {
    //     printf("%x\n", (u16)cpu.PC);
    // }
}

int main(int argc, char** argv) {

    cpu.tick = &tick_fn;
    u8 mem[0x10000];
    
    FILE *f = fopen(argv[1], "rb");
    printf("%s\n", argv[1]);
    // fgets((char*)mem, 0xFFFF, f);
    fread(mem, 0x10000, 1, f);
    fclose(f);

    printf("Read bytes from memory\n");

    // cpu_reset(&cpu, mem);
    cpu.PC = 0x400;
    cpu.S = 0xFF;
    cpu.P.B = 1;
    cpu.P.u = 1;
    int n_instrs = 80000;
    printf("%x\n", mem[0x0206]);
    // printf("%x\n", mem[0x16]);
    printf("i\tPC\tinst\tX\tY\tA\tS\tP\n");
    for (; inst_ctr<n_instrs; inst_ctr++) {
        u16 prev_pc = cpu.PC;
        // printf("Executing opc 0x%x\n", (uint8_t)(mem[cpu.PC]));
        int res = cpu_exec(&cpu, mem);
        // if (inst_ctr == 41798) {
        //     printf("%d\n", cpu.P.Z);
        // }
        if (inst_ctr > 54500) {
            printf("%d\t%x\t%x\t%d\t%d\t%d\t%u\t%x\n", inst_ctr, prev_pc, mem[prev_pc], (int8_t)(cpu.X), (int8_t)(cpu.Y), (int8_t)(cpu.A), cpu.S, *(u8*)(&cpu.P));
        }
        if (cpu.PC == prev_pc) {
            printf("PC trapped at %x\n", prev_pc);
            break;
        }
        if (res != 0) {
            printf("Error at PC:%x, ret with code %d\n", cpu.PC, res);
            break;
        }
    }
    printf("DONE executed %d instrs\n", inst_ctr);

    return 0;
}

