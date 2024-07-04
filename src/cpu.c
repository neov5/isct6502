#include "cpu.h"

#define hi(u) (((u16)(u))<<8)

u8 cpu_ucode_pc_r(cpu_state_t *st, u8 *mem) {
    u8 val = mem[st->PC++]; st->tick(); return val;
}

u8 cpu_ucode_addr_r(cpu_state_t *st, u8 *mem, u16 addr) {
    u8 val = mem[addr]; st->tick(); return val; 
}

u8 cpu_ucode_zpa_r(cpu_state_t *st, u8 *mem, u8 addr) {
    u8 val = mem[addr]; st->tick(); return val;
}

u8 cpu_instr_rol(cpu_state_t *st, u8 op) {

}

u16 cpu_ucode_pc_addr_r(cpu_state_t *st, u8 *mem) {
    u16 addr = mem[st->PC++];
    st->tick();
    addr |= ((u16)mem[st->PC++])<<8;
    st->tick();
    return addr;
}

void cpu_set_nz(cpu_state_t* st, u8 val) {
    st->P.N = (val >> 7);
    st->P.Z = (val == 0);
}

// read instructions
void cpu_instr_lda(cpu_state_t* st, u8 op) { st->A = op; cpu_set_nz(st, op); }
void cpu_instr_ldx(cpu_state_t* st, u8 op) { st->X = op; cpu_set_nz(st, op); }
void cpu_instr_ldy(cpu_state_t* st, u8 op) { st->Y = op; cpu_set_nz(st, op); }
void cpu_instr_ora(cpu_state_t* st, u8 op) { cpu_instr_lda(st, st->A | op); }
void cpu_instr_eor(cpu_state_t* st, u8 op) { cpu_instr_lda(st, st->A ^ op); }
void cpu_instr_and(cpu_state_t* st, u8 op) { cpu_instr_lda(st, st->A & op); }
// TODO adc, dbc, cmp, bit, lax

// rmw instructions
u8 cpu_instr_dec(cpu_state_t* st, u8 op) { return op-1; }
u8 cpu_instr_inc(cpu_state_t* st, u8 op) { return op+1; }

// write instructions
u8 cpu_instr_sta(cpu_state_t* st) { return st->A; }
u8 cpu_instr_stx(cpu_state_t* st) { return st->X; }
u8 cpu_instr_sty(cpu_state_t* st) { return st->Y; }

// implied instructions
void cpu_instr_clc(cpu_state_t* st) { st->P.C = 0; }
void cpu_instr_cld(cpu_state_t* st) { st->P.D = 0; }
void cpu_instr_cli(cpu_state_t* st) { st->P.I = 0; }
void cpu_instr_clv(cpu_state_t* st) { st->P.V = 0; }
void cpu_instr_sec(cpu_state_t* st) { st->P.C = 1; }
void cpu_instr_sed(cpu_state_t* st) { st->P.D = 1; }
void cpu_instr_sei(cpu_state_t* st) { st->P.I = 1; }
void cpu_instr_tax(cpu_state_t *st) { cpu_instr_ldx(st, st->A); }
void cpu_instr_tay(cpu_state_t *st) { cpu_instr_ldy(st, st->A); }
void cpu_instr_tsx(cpu_state_t *st) { cpu_instr_ldx(st, st->S); }
void cpu_instr_txa(cpu_state_t *st) { cpu_instr_lda(st, st->X); }
void cpu_instr_tya(cpu_state_t *st) { cpu_instr_lda(st, st->Y); }
void cpu_instr_txs(cpu_state_t *st) { st->S = st->X; }
void cpu_instr_dex(cpu_state_t *st) { cpu_instr_ldx(st, st->X-1); }
void cpu_instr_dey(cpu_state_t *st) { cpu_instr_ldy(st, st->Y-1); }
void cpu_instr_inx(cpu_state_t *st) { cpu_instr_ldx(st, st->X+1); }
void cpu_instr_iny(cpu_state_t *st) { cpu_instr_ldy(st, st->Y+1); }
void cpu_instr_nop(cpu_state_t *st) { /* do nothing */ }

// implied, accumulator instructions

void cpu_instrcl_all_imp(cpu_state_t *st, u8 *mem, void (*instr)(cpu_state_t*, u8*)) {
    instr(st, mem); // 2, .., n-1
    st->tick(); // n
}

void cpu_instrcl_all_acc(cpu_state_t *st, u8 *mem, void (*instr)(cpu_state_t*, u8*)) {
    instr(st, mem); // 2, .., n-1
    st->tick(); // n
}

void cpu_instrcl_all_imm(cpu_state_t *st, u8 *mem, void (*instr)(cpu_state_t*, u8)) {
    instr(st, mem[st->PC++]); st->tick(); // 2 .. n-1, n
}

// Absolute addressing 
void cpu_instrcl_read_abs(cpu_state_t *st, u8 *mem, void (*instr)(cpu_state_t*, u8)) {
    u16 addr = mem[st->PC++];  st->tick(); // 2
    addr |= hi(mem[st->PC++]); st->tick(); // 3
    instr(st, mem[addr]);      st->tick(); // 4
}

void cpu_instrcl_rmw_abs(cpu_state_t *st, u8 *mem, u8 (*instr)(cpu_state_t*, u8)) {
    u16 addr = mem[st->PC++];  st->tick(); // 2
    addr |= hi(mem[st->PC++]); st->tick(); // 3
    u8 op = mem[addr];         st->tick(); // 4
    u8 res = instr(st, op);    st->tick(); // 5
    mem[addr] = res;           st->tick(); // 6
}

void cpu_instrcl_write_abs(cpu_state_t *st, u8 *mem, u8 (*instr)(cpu_state_t*)) {
    u16 addr = mem[st->PC++];  st->tick(); // 2
    addr |= hi(mem[st->PC++]); st->tick(); // 3
    mem[addr] = instr(st);     st->tick(); // 4
}

void cpu_instrcl_jmp_abs(cpu_state_t *st, u8 *mem) {
    u16 addr = mem[st->PC++];                 st->tick(); // 2
    addr |= hi(mem[st->PC++]); st->PC = addr; st->tick(); // 3
}

// zero page addressing
void cpu_instrcl_read_zpa(cpu_state_t *st, u8 *mem, void (*instr)(cpu_state_t*, u8)) {
    u8 zpa = mem[st->PC++];   st->tick(); // 2
    instr(st, mem[zpa]);      st->tick(); // 3
}

void cpu_instrcl_rmw_zpa(cpu_state_t *st, u8 *mem, u8 (*instr)(cpu_state_t*, u8)) {
    u8 zpa = mem[st->PC++];   st->tick(); // 2
    u8 op = mem[zpa];         st->tick(); // 3
    u8 res = instr(st, op);   st->tick(); // 4
    mem[zpa] = res;           st->tick(); // 5
}

void cpu_instrcl_write_zpa(cpu_state_t *st, u8 *mem, u8 (*instr)(cpu_state_t*)) {
    u8 zpa = mem[st->PC++];   st->tick(); // 2
    mem[zpa] = instr(st);     st->tick(); // 3
}

// zero page indexed addressing
void cpu_instrcl_read_zpi(cpu_state_t *st, u8 *mem, u8 idx, void (*instr)(cpu_state_t*, u8)) {
    u8 zpa = mem[st->PC++];   st->tick(); // 2
    u8 addr = mem[zpa] + idx; st->tick(); // 3
    instr(st, mem[addr]);     st->tick(); // 4
}

void cpu_instrcl_rmw_zpi(cpu_state_t *st, u8 *mem, u8 idx, u8 (*instr)(cpu_state_t*, u8)) {
    u8 zpa = mem[st->PC++];   st->tick(); // 2
    u8 addr = mem[zpa] + idx; st->tick(); // 3
    u8 op = mem[addr];        st->tick(); // 4
    u8 res = instr(st, op);   st->tick(); // 5
    mem[zpa] = res;           st->tick(); // 6
}

void cpu_instrcl_write_zpi(cpu_state_t *st, u8 *mem, u8 idx, u8 (*instr)(cpu_state_t*)) {
    u8 zpa = mem[st->PC++];   st->tick(); // 2
    u8 addr = mem[zpa] + idx; st->tick(); // 3
    mem[addr] = instr(st);    st->tick(); // 4
}

// absolute indexed addressing
void cpu_instrcl_read_abi(cpu_state_t *st, u8 *mem, u8 idx, void (*instr)(cpu_state_t*, u8)) {
    u16 addr = mem[st->PC++];        st->tick(); // 2
    addr |= hi(mem[st->PC++]);       st->tick(); // 3
    u16 newaddr = addr + idx;
    if ((addr & 0xFF) + idx > 0xFF)  st->tick(); // fixup
    instr(st, mem[newaddr]);         st->tick(); // 4/5
}

void cpu_instrcl_rmw_abi(cpu_state_t *st, u8 *mem, u8 idx, u8 (*instr)(cpu_state_t*, u8)) {
    u16 addr = mem[st->PC++];        st->tick(); // 2
    addr |= hi(mem[st->PC++]);       st->tick(); // 3
                                     st->tick(); // 4
    u8 op = mem[addr];               st->tick(); // 5
    u8 res = instr(st, op);          st->tick(); // 6
    mem[addr] = res;                 st->tick(); // 7
}

void cpu_instrcl_write_abi(cpu_state_t *st, u8 *mem, u8 idx, u8 (*instr)(cpu_state_t*)) {
    u16 addr = mem[st->PC++];        st->tick(); // 2
    addr |= hi(mem[st->PC++]);       st->tick(); // 3
                                     st->tick(); // 4
    mem[addr] = instr(st);           st->tick(); // 5
}

// TODO branches - relative addressing


// zero-page indirect preindexed [($nn, X)]
void cpu_instrcl_read_zpx(cpu_state_t *st, u8 *mem, void (*instr)(cpu_state_t*, u8)) {
    u8 ptraddr = mem[st->PC++];    st->tick(); // 2
    u8 ptr = mem[ptraddr] + st->X; st->tick(); // 3
    u16 addr = mem[ptr];           st->tick(); // 4
    addr |= hi(mem[ptr+1]);        st->tick(); // 5
    instr(st, mem[addr]);          st->tick(); // 6
}

void cpu_instrcl_rmw_zpx(cpu_state_t *st, u8 *mem, u8 (*instr)(cpu_state_t*, u8)) {
    u8 ptraddr = mem[st->PC++];    st->tick(); // 2
    u8 ptr = mem[ptraddr] + st->X; st->tick(); // 3
    u16 addr = mem[ptr];           st->tick(); // 4
    addr |= hi(mem[ptr+1]);        st->tick(); // 5
    u8 op = mem[addr];             st->tick(); // 6
    u8 result = instr(st, op);     st->tick(); // 7
    mem[addr] = result;            st->tick(); // 8
}

void cpu_instrcl_write_zpx(cpu_state_t *st, u8 *mem, u8 (*instr)(cpu_state_t*)) {
    u8 ptraddr = mem[st->PC++];    st->tick(); // 2
    u8 ptr = mem[ptraddr] + st->X; st->tick(); // 3
    u16 addr = mem[ptr];           st->tick(); // 4
    addr |= hi(mem[ptr+1]);        st->tick(); // 5
    mem[addr] = instr(st);         st->tick(); // 6
}

// zero-page preindexed indirect [($nn), Y]
void cpu_instrcl_read_zpy(cpu_state_t *st, u8 *mem, void (*instr)(cpu_state_t*, u8)) {
    u8 ptr = mem[st->PC++];           st->tick(); // 2
    u16 addr = mem[ptr];              st->tick(); // 3
    addr |= hi(mem[ptr+1]);
    u16 newaddr = ptr + st->Y;        st->tick(); // 4
    if ((addr & 0xFF) + st->Y > 0xFF) st->tick(); // fixup
    instr(st, mem[addr]);             st->tick(); // 5/6
}

void cpu_instrcl_rmw_zpy(cpu_state_t *st, u8 *mem, u8 (*instr)(cpu_state_t*, u8)) {
    u8 ptr = mem[st->PC++];       st->tick(); // 2
    u16 addr = mem[ptr] + st->Y;  st->tick(); // 3
    addr |= hi(mem[ptr+1]);       st->tick(); // 4
                                  st->tick(); // 5
    u8 op = mem[addr];            st->tick(); // 6
    u8 result = instr(st, op);    st->tick(); // 7
    mem[addr] = result;           st->tick(); // 8
}

void cpu_instrcl_write_zpy(cpu_state_t *st, u8 *mem, u8 (*instr)(cpu_state_t*)) {
    u8 ptr = mem[st->PC++];       st->tick(); // 2
    u16 addr = mem[ptr] + st->Y;  st->tick(); // 3
    addr |= hi(mem[ptr+1]);       st->tick(); // 4
                                  st->tick(); // 5
    mem[addr] = instr(st);        st->tick(); // 6
}

// absolute indirect addressing 
void cpu_instrcl_jmp_ind(cpu_state_t *st, u8 *mem) {

}

void cpu_exec(cpu_state_t *st, u8 *mem) {

    u8 opc = mem[st->PC++]; st->tick();
    u8 res;
    switch (opc) {
        case 0x00: cpu_instrcl_all_imp(st, mem, cpu_instr_brk);
        case 0x01: cpu_instrcl_read_ind(st, mem, st->X, cpu_instr_ora);
        case 0x05: cpu_instrcl_read_zpa(st, mem, cpu_instr_ora);
        case 0x05: cpu_instrcl_rmw_zpa(st, mem, cpu_instr_asl);
        case 0x07: cpu_instrcl_all_imp(st, mem, cpu_instr_asl);

    }
}
