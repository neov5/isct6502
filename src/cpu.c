#include "cpu.h"

#define hi(u) (((u16)(u))<<8)
#define lo(u) ((u)&0xFF)

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

void cpu_instrcl_all_imp(cpu_state_t *st, u8 *mem, void (*instr)(cpu_state_t*)) {
    instr(st); // 2, .., n-1
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
// void cpu_instrcl_branch(cpu_state_t *st, u8 *mem, u8 (*branch)(cpu_state_t*, u8)) {
//     u8 op = mem[st->PC++];   st->tick(); // 2
//     u8 bt = branch(st, op);  st->tick();
// }

// zero-page indirect preindexed [($nn, X)]
void cpu_instrcl_read_zpx(cpu_state_t *st, u8 *mem, void (*instr)(cpu_state_t*, u8)) {
    u8 ptraddr = mem[st->PC++];    st->tick(); // 2
    u8 ptr = mem[ptraddr] + st->X; st->tick(); // 3
    u16 addr = mem[ptr];           st->tick(); // 4
    addr |= hi(mem[lo(ptr+1)]);    st->tick(); // 5
    instr(st, mem[addr]);          st->tick(); // 6
}

void cpu_instrcl_rmw_zpx(cpu_state_t *st, u8 *mem, u8 (*instr)(cpu_state_t*, u8)) {
    u8 ptraddr = mem[st->PC++];    st->tick(); // 2
    u8 ptr = mem[ptraddr] + st->X; st->tick(); // 3
    u16 addr = mem[ptr];           st->tick(); // 4
    addr |= hi(mem[lo(ptr+1)]);    st->tick(); // 5
    u8 op = mem[addr];             st->tick(); // 6
    u8 result = instr(st, op);     st->tick(); // 7
    mem[addr] = result;            st->tick(); // 8
}

void cpu_instrcl_write_zpx(cpu_state_t *st, u8 *mem, u8 (*instr)(cpu_state_t*)) {
    u8 ptraddr = mem[st->PC++];    st->tick(); // 2
    u8 ptr = mem[ptraddr] + st->X; st->tick(); // 3
    u16 addr = mem[ptr];           st->tick(); // 4
    addr |= hi(mem[lo(ptr+1)]);    st->tick(); // 5
    mem[addr] = instr(st);         st->tick(); // 6
}

// zero-page preindexed indirect [($nn), Y]
void cpu_instrcl_read_zpy(cpu_state_t *st, u8 *mem, void (*instr)(cpu_state_t*, u8)) {
    u8 ptr = mem[st->PC++];           st->tick(); // 2
    u16 addr = mem[ptr];              st->tick(); // 3
    addr |= hi(mem[lo(ptr+1)]);
    u16 newaddr = ptr + st->Y;        st->tick(); // 4
    if ((addr & 0xFF) + st->Y > 0xFF) st->tick(); // fixup
    instr(st, mem[addr]);             st->tick(); // 5/6
}

void cpu_instrcl_rmw_zpy(cpu_state_t *st, u8 *mem, u8 (*instr)(cpu_state_t*, u8)) {
    u8 ptr = mem[st->PC++];       st->tick(); // 2
    u16 addr = mem[ptr] + st->Y;  st->tick(); // 3
    addr |= hi(mem[lo(ptr+1)]);   st->tick(); // 4
                                  st->tick(); // 5
    u8 op = mem[addr];            st->tick(); // 6
    u8 result = instr(st, op);    st->tick(); // 7
    mem[addr] = result;           st->tick(); // 8
}

void cpu_instrcl_write_zpy(cpu_state_t *st, u8 *mem, u8 (*instr)(cpu_state_t*)) {
    u8 ptr = mem[st->PC++];       st->tick(); // 2
    u16 addr = mem[ptr] + st->Y;  st->tick(); // 3
    addr |= hi(mem[lo(ptr+1)]);   st->tick(); // 4
                                  st->tick(); // 5
    mem[addr] = instr(st);        st->tick(); // 6
}

// absolute indirect addressing 
void cpu_instrcl_jmp_ind(cpu_state_t *st, u8 *mem) {
    u16 ptr = mem[st->PC++];        st->tick(); // 2
    ptr |= hi(mem[st->PC++]);       st->tick(); // 3
    u8 latch = mem[ptr];            st->tick(); // 4
    st->PC = hi(mem[(ptr & 0xFF00) | lo(ptr+1)]) | latch; st->tick(); // 5
}

void cpu_exec(cpu_state_t *st, u8 *mem) {

    u8 opc = mem[st->PC++]; st->tick();
    u8 res;
    switch (opc) {
        case 0xAA: cpu_instrcl_all_imp(st, mem, &cpu_instr_tax); break;
        case 0xA8: cpu_instrcl_all_imp(st, mem, &cpu_instr_tay); break;
        case 0xBA: cpu_instrcl_all_imp(st, mem, &cpu_instr_tsx); break;
        case 0x8A: cpu_instrcl_all_imp(st, mem, &cpu_instr_txa); break;
        case 0x9A: cpu_instrcl_all_imp(st, mem, &cpu_instr_txs); break;
        case 0x98: cpu_instrcl_all_imp(st, mem, &cpu_instr_tya); break;
        // case 0x48: cpu_instrcl_all_imp(st, mem, &cpu_instr_pha); break;
        // case 0x08: cpu_instrcl_all_imp(st, mem, &cpu_instr_php); break;
        // case 0x68: cpu_instrcl_all_imp(st, mem, &cpu_instr_pla); break;
        // case 0x28: cpu_instrcl_all_imp(st, mem, &cpu_instr_plp); break;
        case 0xCA: cpu_instrcl_all_imp(st, mem, &cpu_instr_dex); break;
        case 0x88: cpu_instrcl_all_imp(st, mem, &cpu_instr_dey); break;
        case 0xE8: cpu_instrcl_all_imp(st, mem, &cpu_instr_inx); break;
        case 0xC8: cpu_instrcl_all_imp(st, mem, &cpu_instr_iny); break;
        // case 0x00: cpu_instrcl_all_imp(st, mem, &cpu_instr_brk); break;
        // case 0x40: cpu_instrcl_all_imp(st, mem, &cpu_instr_rti); break;
        // case 0x60: cpu_instrcl_all_imp(st, mem, &cpu_instr_rts); break;
        case 0x18: cpu_instrcl_all_imp(st, mem, &cpu_instr_clc); break;
        case 0xD8: cpu_instrcl_all_imp(st, mem, &cpu_instr_cld); break;
        case 0x58: cpu_instrcl_all_imp(st, mem, &cpu_instr_cli); break;
        case 0xB8: cpu_instrcl_all_imp(st, mem, &cpu_instr_clv); break;
        case 0x38: cpu_instrcl_all_imp(st, mem, &cpu_instr_sec); break;
        case 0xF8: cpu_instrcl_all_imp(st, mem, &cpu_instr_sed); break;
        case 0x78: cpu_instrcl_all_imp(st, mem, &cpu_instr_sei); break;
        case 0xEA: cpu_instrcl_all_imp(st, mem, &cpu_instr_nop); break;

        // case 0x0A: cpu_instrcl_all_acc(st, mem, &cpu_instr_asl); break;
        // case 0x4A: cpu_instrcl_all_acc(st, mem, &cpu_instr_lsr); break;
        // case 0x2A: cpu_instrcl_all_acc(st, mem, &cpu_instr_rol); break;
        // case 0x6A: cpu_instrcl_all_acc(st, mem, &cpu_instr_ror); break;

        case 0xA9: cpu_instrcl_all_imm(st, mem, &cpu_instr_lda); break;
        case 0xA2: cpu_instrcl_all_imm(st, mem, &cpu_instr_ldx); break;
        case 0xA0: cpu_instrcl_all_imm(st, mem, &cpu_instr_ldy); break;
        case 0x29: cpu_instrcl_all_imm(st, mem, &cpu_instr_and); break;
        case 0x49: cpu_instrcl_all_imm(st, mem, &cpu_instr_eor); break;
        case 0x09: cpu_instrcl_all_imm(st, mem, &cpu_instr_ora); break;
        // case 0x69: cpu_instrcl_all_imm(st, mem, &cpu_instr_adc); break;
        // case 0xC9: cpu_instrcl_all_imm(st, mem, &cpu_instr_cmp); break;
        // case 0xE0: cpu_instrcl_all_imm(st, mem, &cpu_instr_cpx); break;
        // case 0xC0: cpu_instrcl_all_imm(st, mem, &cpu_instr_cpy); break;
        // case 0xE9: cpu_instrcl_all_imm(st, mem, &cpu_instr_sbc); break;

        case 0xAD: cpu_instrcl_read_abs(st, mem, &cpu_instr_lda); break;
        case 0xAE: cpu_instrcl_read_abs(st, mem, &cpu_instr_ldx); break;
        case 0xAC: cpu_instrcl_read_abs(st, mem, &cpu_instr_ldy); break;
        case 0x4D: cpu_instrcl_read_abs(st, mem, &cpu_instr_eor); break;
        case 0x2D: cpu_instrcl_read_abs(st, mem, &cpu_instr_and); break;
        case 0x0D: cpu_instrcl_read_abs(st, mem, &cpu_instr_ora); break;
        // case 0x6D: cpu_instrcl_read_abs(st, mem, &cpu_instr_adc); break;
        // case 0xED: cpu_instrcl_read_abs(st, mem, &cpu_instr_sbc); break;
        // case 0xCD: cpu_instrcl_read_abs(st, mem, &cpu_instr_cmp); break;
        // case 0xEC: cpu_instrcl_read_abs(st, mem, &cpu_instr_cpx); break;
        // case 0xCC: cpu_instrcl_read_abs(st, mem, &cpu_instr_cpy); break;
        // case 0x2C: cpu_instrcl_read_abs(st, mem, &cpu_instr_bit); break;

        // case 0x0E: cpu_instrcl_rmw_abs(st, mem, &cpu_instr_asl); break;
        // case 0x4E: cpu_instrcl_rmw_abs(st, mem, &cpu_instr_lsr); break;
        // case 0x2E: cpu_instrcl_rmw_abs(st, mem, &cpu_instr_rol); break;
        // case 0x6E: cpu_instrcl_rmw_abs(st, mem, &cpu_instr_ror); break;
        case 0xEE: cpu_instrcl_rmw_abs(st, mem, &cpu_instr_inc); break;
        case 0xCE: cpu_instrcl_rmw_abs(st, mem, &cpu_instr_dec); break;

        case 0x8D: cpu_instrcl_write_abs(st, mem, &cpu_instr_sta); break;
        case 0x8E: cpu_instrcl_write_abs(st, mem, &cpu_instr_stx); break;
        case 0x8C: cpu_instrcl_write_abs(st, mem, &cpu_instr_sty); break;

        case 0x4C: cpu_instrcl_jmp_abs(st, mem); break;

        case 0xBD: cpu_instrcl_read_abi(st, mem, st->X, &cpu_instr_lda); break;
        case 0xBC: cpu_instrcl_read_abi(st, mem, st->X, &cpu_instr_ldy); break;
        case 0x3D: cpu_instrcl_read_abi(st, mem, st->X, &cpu_instr_and); break;
        case 0x5D: cpu_instrcl_read_abi(st, mem, st->X, &cpu_instr_eor); break;
        case 0x1D: cpu_instrcl_read_abi(st, mem, st->X, &cpu_instr_ora); break;
        case 0x7D: cpu_instrcl_read_abi(st, mem, st->X, &cpu_instr_adc); break;
        case 0xDD: cpu_instrcl_read_abi(st, mem, st->X, &cpu_instr_cmp); break;
        case 0xFD: cpu_instrcl_read_abi(st, mem, st->X, &cpu_instr_sbc); break;
        case 0x1E: cpu_instrcl_rmw_abi(st, mem, st->X, &cpu_instr_asl); break;
        case 0x5E: cpu_instrcl_rmw_abi(st, mem, st->X, &cpu_instr_lsr); break;
        case 0x3E: cpu_instrcl_rmw_abi(st, mem, st->X, &cpu_instr_rol); break;
        case 0x7E: cpu_instrcl_rmw_abi(st, mem, st->X, &cpu_instr_ror); break;
        case 0xDE: cpu_instrcl_rmw_abi(st, mem, st->X, &cpu_instr_dec); break;
        case 0xFE: cpu_instrcl_rmw_abi(st, mem, st->X, &cpu_instr_inc); break;
        case 0x9D: cpu_instrcl_write_abi(st, mem, st->X, &cpu_instr_sta); break;

        case 0xB9: cpu_instrcl_read_abi(st, mem, st->Y, &cpu_instr_lda); break;
        case 0xBE: cpu_instrcl_read_abi(st, mem, st->Y, &cpu_instr_ldx); break;
        case 0x39: cpu_instrcl_read_abi(st, mem, st->Y, &cpu_instr_and); break;
        case 0x59: cpu_instrcl_read_abi(st, mem, st->Y, &cpu_instr_eor); break;
        case 0x19: cpu_instrcl_read_abi(st, mem, st->Y, &cpu_instr_ora); break;
        case 0x79: cpu_instrcl_read_abi(st, mem, st->Y, &cpu_instr_adc); break;
        case 0xD9: cpu_instrcl_read_abi(st, mem, st->Y, &cpu_instr_cmp); break;
        case 0xF9: cpu_instrcl_read_abi(st, mem, st->Y, &cpu_instr_sbc); break;
        case 0x99: cpu_instrcl_write_abi(st, mem, st->Y, &cpu_instr_sta); break;

        case 0x6C: cpu_instrcl_jmp_ind(st, mem); break;

        case 0xA5: cpu_instrcl_read_zpg(st, mem, &cpu_instr_lda); break;
        case 0xA6: cpu_instrcl_read_zpg(st, mem, &cpu_instr_ldx); break;
        case 0xA4: cpu_instrcl_read_zpg(st, mem, &cpu_instr_ldy); break;
        case 0x25: cpu_instrcl_read_zpg(st, mem, &cpu_instr_and); break;
        case 0x24: cpu_instrcl_read_zpg(st, mem, &cpu_instr_bit); break;
        case 0x45: cpu_instrcl_read_zpg(st, mem, &cpu_instr_eor); break;
        case 0x05: cpu_instrcl_read_zpg(st, mem, &cpu_instr_ora); break;
        case 0x65: cpu_instrcl_read_zpg(st, mem, &cpu_instr_adc); break;
        case 0xC5: cpu_instrcl_read_zpg(st, mem, &cpu_instr_cmp); break;
        case 0xE4: cpu_instrcl_read_zpg(st, mem, &cpu_instr_cpx); break;
        case 0xC4: cpu_instrcl_read_zpg(st, mem, &cpu_instr_cpy); break;
        case 0xE5: cpu_instrcl_read_zpg(st, mem, &cpu_instr_sbc); break;
        case 0xC6: cpu_instrcl_rmw_zpg(st, mem, &cpu_instr_dec); break;
        case 0xE6: cpu_instrcl_rmw_zpg(st, mem, &cpu_instr_inc); break;
        case 0x06: cpu_instrcl_rmw_zpg(st, mem, &cpu_instr_asl); break;
        case 0x46: cpu_instrcl_rmw_zpg(st, mem, &cpu_instr_lsr); break;
        case 0x26: cpu_instrcl_rmw_zpg(st, mem, &cpu_instr_rol); break;
        case 0x66: cpu_instrcl_rmw_zpg(st, mem, &cpu_instr_ror); break;
        case 0x85: cpu_instrcl_write_zpg(st, mem, &cpu_instr_sta); break;
        case 0x86: cpu_instrcl_write_zpg(st, mem, &cpu_instr_stx); break;
        case 0x84: cpu_instrcl_write_zpg(st, mem, &cpu_instr_sty); break;

        case 0xB5: cpu_instrcl_read_zpi(st, mem, st->X, &cpu_instr_lda); break;
        case 0xB4: cpu_instrcl_read_zpi(st, mem, st->X, &cpu_instr_ldy); break;
        case 0x35: cpu_instrcl_read_zpi(st, mem, st->X, &cpu_instr_and); break;
        case 0x55: cpu_instrcl_read_zpi(st, mem, st->X, &cpu_instr_eor); break;
        case 0x15: cpu_instrcl_read_zpi(st, mem, st->X, &cpu_instr_ora); break;
        case 0x75: cpu_instrcl_read_zpi(st, mem, st->X, &cpu_instr_adc); break;
        case 0xD5: cpu_instrcl_read_zpi(st, mem, st->X, &cpu_instr_cmp); break;
        case 0xF5: cpu_instrcl_read_zpi(st, mem, st->X, &cpu_instr_sbc); break;
        case 0x16: cpu_instrcl_rmw_zpi(st, mem, st->X, &cpu_instr_asl); break;
        case 0x56: cpu_instrcl_rmw_zpi(st, mem, st->X, &cpu_instr_lsr); break;
        case 0x36: cpu_instrcl_rmw_zpi(st, mem, st->X, &cpu_instr_rol); break;
        case 0x76: cpu_instrcl_rmw_zpi(st, mem, st->X, &cpu_instr_ror); break;
        case 0xD6: cpu_instrcl_rmw_zpi(st, mem, st->X, &cpu_instr_dec); break;
        case 0xF6: cpu_instrcl_rmw_zpi(st, mem, st->X, &cpu_instr_inc); break;
        case 0x95: cpu_instrcl_write_zpi(st, mem, st->X, &cpu_instr_sta); break;
        case 0x94: cpu_instrcl_write_zpi(st, mem, st->X, &cpu_instr_sty); break;

        case 0xB6: cpu_instrcl_read_zpi(st, mem, st->Y, &cpu_instr_ldx); break;
        case 0x96: cpu_instrcl_write_zpi(st, mem, st->Y, &cpu_instr_stx); break;
        
        case 0xA1: cpu_instrcl_read_zpx(st, mem, &cpu_instr_lda); break;
        case 0x21: cpu_instrcl_read_zpx(st, mem, &cpu_instr_and); break;
        case 0x41: cpu_instrcl_read_zpx(st, mem, &cpu_instr_eor); break;
        case 0x01: cpu_instrcl_read_zpx(st, mem, &cpu_instr_ora); break;
        case 0x61: cpu_instrcl_read_zpx(st, mem, &cpu_instr_adc); break;
        case 0xC1: cpu_instrcl_read_zpx(st, mem, &cpu_instr_cmp); break;
        case 0xE1: cpu_instrcl_read_zpx(st, mem, &cpu_instr_sbc); break;
        case 0x81: cpu_instrcl_write_zpx(st, mem, &cpu_instr_sta); break;

        case 0xB1: cpu_instrcl_read_zpy(st, mem, &cpu_instr_lda); break;
        case 0x31: cpu_instrcl_read_zpy(st, mem, &cpu_instr_and); break;
        case 0x51: cpu_instrcl_read_zpy(st, mem, &cpu_instr_eor); break;
        case 0x11: cpu_instrcl_read_zpy(st, mem, &cpu_instr_ora); break;
        case 0x71: cpu_instrcl_read_zpy(st, mem, &cpu_instr_adc); break;
        case 0xD1: cpu_instrcl_read_zpy(st, mem, &cpu_instr_cmp); break;
        case 0xF1: cpu_instrcl_read_zpy(st, mem, &cpu_instr_sbc); break;
        case 0x91: cpu_instrcl_write_zpy(st, mem, &cpu_instr_sta); break;

        case 0x90: cpu_instrcl_branch(st, mem, &cpu_instr_bcc); break;
        case 0xB0: cpu_instrcl_branch(st, mem, &cpu_instr_bcs); break;
        case 0xF0: cpu_instrcl_branch(st, mem, &cpu_instr_beq); break;
        case 0x30: cpu_instrcl_branch(st, mem, &cpu_instr_bmi); break;
        case 0xD0: cpu_instrcl_branch(st, mem, &cpu_instr_bne); break;
        case 0x10: cpu_instrcl_branch(st, mem, &cpu_instr_bpl); break;
        case 0x50: cpu_instrcl_branch(st, mem, &cpu_instr_bvc); break;
        case 0x70: cpu_instrcl_branch(st, mem, &cpu_instr_bvs); break;
    }
}
