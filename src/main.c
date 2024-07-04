#include <stdio.h>
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
} sr_t;

typedef struct {
    u8 A;
    u8 Y;
    u8 X;
    u16 PC;
    u8 S;
    sr_t P;
} cpu_state_t;

u16 mem_get_dw(u8* mem, u16 idx) { 
    return *((u16*)&mem[idx]); // assumes little endian
}

void cpu_set_nz(cpu_state_t* state, u8 val) {
    state->P.N = (val >> 7);
    state->P.Z = (val == 0);
}

void cpu_set_a(cpu_state_t* state, u8 a_val) { state->A = a_val; cpu_set_nz(state, a_val); }
void cpu_set_x(cpu_state_t* state, u8 x_val) { state->X = x_val; cpu_set_nz(state, x_val); }
void cpu_set_y(cpu_state_t* state, u8 y_val) { state->Y = y_val; cpu_set_nz(state, y_val); }

u16 cpu_addr_acc(cpu_state_t* state, u8* mem) { return state->A; }
u16 cpu_addr_abs(cpu_state_t* state, u8* mem) { return mem_get_dw(mem, state->PC); }
u16 cpu_addr_abs_x(cpu_state_t* state, u8* mem) { return cpu_addr_abs(state, mem) + state->X; }
u16 cpu_addr_abs_y(cpu_state_t* state, u8* mem) { return cpu_addr_abs(state, mem) + state->Y; }
u16 cpu_addr_imm(cpu_state_t* state, u8* mem) { return mem[state->PC]; }
u16 cpu_addr_ind(cpu_state_t* state, u8* mem) { return mem_get_dw(mem, mem_get_dw(mem, state->PC)); }
u16 cpu_addr_ind_x(cpu_state_t* state, u8* mem) { return mem_get_dw(mem, mem[state->PC] + state->X); }
u16 cpu_addr_ind_y(cpu_state_t* state, u8* mem) { return mem_get_dw(mem, mem[state->PC]) + state->Y; }
u16 cpu_addr_rel(cpu_state_t* state, u8* mem) { return (state->PC-1) + mem[state->PC]; }
u16 cpu_addr_zpg(cpu_state_t* state, u8* mem) { return mem[state->PC]; }
u16 cpu_addr_zpg_x(cpu_state_t* state, u8* mem) { return mem[state->PC] + state->X; }
u16 cpu_addr_zpg_y(cpu_state_t* state, u8* mem) { return mem[state->PC] + state->Y; }

void cpu_instr_adc(cpu_state_t* st, u8* mem) { 
    u8 new_st = st->A+(*mem)+st->P.C;
    cpu_set_a(st, new_st);
    /* TODO */
}

void cpu_instr_bvc(cpu_state_t* st, u8* mem) { /* TODO */ }
void cpu_instr_bvs(cpu_state_t* st, u8* mem) { /* TODO */ }

void cpu_instr_bvc(cpu_state_t* st, u8* mem) { /* TODO */ }
void cpu_instr_bvs(cpu_state_t* st, u8* mem) { /* TODO */ }

void cpu_instr_cmp(cpu_state_t* st, u8* mem) { cpu_set_nz(st, st->A - *mem); }
void cpu_instr_cpx(cpu_state_t* st, u8* mem) { cpu_set_nz(st, st->X - *mem); }
void cpu_instr_cpy(cpu_state_t* st, u8* mem) { cpu_set_nz(st, st->Y - *mem); }

void cpu_instr_eor(cpu_state_t* st, u8* mem) { cpu_set_a(st, st->A ^ *mem); }

void cpu_instr_dec(cpu_state_t* st, u8* mem) { *mem = *mem - 1; }
void cpu_instr_dex(cpu_state_t* st) { cpu_set_x(st, st->X+1); }
void cpu_instr_dey(cpu_state_t* st) { cpu_set_y(st, st->Y+1); }
void cpu_instr_inc(cpu_state_t* st, u8* mem) { *mem = *mem + 1; }
void cpu_instr_inx(cpu_state_t* st) { cpu_set_x(st, st->X+1); }
void cpu_instr_iny(cpu_state_t* st) { cpu_set_y(st, st->Y+1); }

void cpu_instr_jmp(cpu_state_t* st, u8* op) {

}

void cpu_instr_jsr(cpu_state_t* st, u8* addr) {

}

void cpu_instr_lda(cpu_state_t* st, u8* operand) { cpu_set_a(st, *operand); }
void cpu_instr_ldx(cpu_state_t* st, u8* operand) { cpu_set_x(st, *operand); }
void cpu_instr_ldy(cpu_state_t* st, u8* operand) { cpu_set_y(st, *operand); }

void cpu_instr_lsr(cpu_state_t* st, u8* operand) {
    st->P.C = *operand & 0x1;
    *operand = (*operand >> 1);
    st->P.N = 0;
    st->P.Z = (*operand == 0);
}

void cpu_instr_ora(cpu_state_t* st, u8* operand) { cpu_set_a(st, st->A | *operand); }
void cpu_instr_pha(cpu_state_t* st, u8* mem) { mem[st->S--] = st->A; }
void cpu_instr_php(cpu_state_t* st, u8* mem) { mem[st->S--] = *((u8*)&st->P); }
void cpu_instr_pla(cpu_state_t* st, u8* mem) { cpu_set_a(st, mem[++st->S]); }
void cpu_instr_plp(cpu_state_t* st, u8* mem) { st->P = *(sr_t*)&mem[++st->S]; }

void cpu_instr_rol(cpu_state_t* st, u8* operand) {
    u8 c_prev = st->P.C;
    st->P.C = (*operand & 0x7f)>>7;
    *operand = (*operand << 1) | c_prev;
    st->P.N = 0;
    st->P.Z = (*operand == 0);
}

void cpu_instr_ror(cpu_state_t* st, u8* operand) {
    u8 c_prev = st->P.C;
    st->P.C = (*operand & 0x1);
    *operand = (*operand >> 1) | (c_prev << 7);
    st->P.N = 0;
    st->P.Z = (*operand == 0);
}

void cpu_instr_rti(cpu_state_t* st, u8* loc) {
    // TODO 
}

void cpu_instr_rts(cpu_state_t* st, u8* loc) {
    // TODO
}

void cpu_instr_clv(cpu_state_t* st) { st->P.V = 0; }
void cpu_instr_clc(cpu_state_t* st) { st->P.C = 0; }
void cpu_instr_cld(cpu_state_t* st) { st->P.D = 0; }
void cpu_instr_cli(cpu_state_t* st) { st->P.I = 0; }
void cpu_instr_sec(cpu_state_t* st) { st->P.C = 1; }
void cpu_instr_sed(cpu_state_t* st) { st->P.D = 1; }
void cpu_instr_sei(cpu_state_t* st) { st->P.I = 1; }

void cpu_instr_sta(cpu_state_t* st, u8* loc) { *loc = st->A; }
void cpu_instr_stx(cpu_state_t* st, u8* loc) { *loc = st->X; }
void cpu_instr_sty(cpu_state_t* st, u8* loc) { *loc = st->Y; }

void cpu_instr_tax(cpu_state_t* st) { cpu_set_x(st, st->A); }
void cpu_instr_tay(cpu_state_t* st) { cpu_set_y(st, st->A); }
void cpu_instr_tsx(cpu_state_t* st) { cpu_set_x(st, st->S); }
void cpu_instr_txa(cpu_state_t* st) { cpu_set_a(st, st->X); }
void cpu_instr_txs(cpu_state_t* st) { st->S = st->X; }
void cpu_instr_tya(cpu_state_t* st) { cpu_set_a(st, st->Y); }

int cpu_istep(cpu_state_t* state, u8* mem) {

    u8 instr = mem[state->PC];
    state->PC++;
    switch(instr) {
        case 0x69: cpu_instr_adc(state, cpu_addr_imm(state, mem));

}

int main() {

    return 0;
}
