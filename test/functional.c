#include <stdio.h>
#include "cpu.h"

int tick_ctr = 0;
int inst_ctr = 0;
cpu_state_t cpu;
u8 mem[0x10000];

void tick_fn() {
    tick_ctr++;
}

u8 bus_read_fn(u16 addr) { return mem[addr]; }
void bus_write_fn(u8 val, u16 addr) { mem[addr] = val; }

int main(int argc, char** argv) {

    cpu.tick = &tick_fn;
    cpu.bus_read = &bus_read_fn;
    cpu.bus_write = &bus_write_fn;
    
    FILE *f = fopen(argv[1], "rb");
    printf("%s\n", argv[1]);
    fread(mem, 0x10000, 1, f);
    fclose(f);

    printf("Read bytes from memory\n");

    // cpu_reset(&cpu, mem);
    cpu.PC = 0x400;
    cpu.S = 0xFF;
    cpu.P.B = 1;
    cpu.P.u = 1;
    printf("i\tPC\tinst\tX\tY\tA\tS\tP\n");
    for (; ; inst_ctr++) {
        u16 prev_pc = cpu.PC;
        int res = cpu_exec(&cpu);
        if (inst_ctr > 26764000) {
            printf("Success\n");
            break;
            // use to debug:
            // if (inst_ctr == 158258) printf("%x\n", mem[0x11]);
            // printf("%x\t%x\t%x\t%d\t%d\t%d\t%u\t%x\n", inst_ctr, prev_pc, mem[prev_pc], (int8_t)(cpu.X), (int8_t)(cpu.Y), (int8_t)(cpu.A), cpu.S, *(u8*)(&cpu.P));
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
    printf("DONE executed %d instrs taking %d cycles\n", inst_ctr, tick_ctr);

    return 0;
}

