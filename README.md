# isct6502

An [Instruction Stepped, Cycle Ticked][1] 6502 emulator in C. Meant for use in 
multi-chipset system emulation (eg NES, Apple II)

## Features

- [X] Support for **all** MOS6502 instructions and addressing modes
- [X] Cycle-accurate, based on cycle timings from [64doc][2]
- [X] Extensively tested with [Klaus Dormann's functional tests][3]
- [X] Simple functional interface with tick callback: no 'hacks' required to 
      tick systems at different clock speeds (eg NES PPU clk runs at 3x NES CPU clk)

## Usage

```
#include <stdio.h>
#include "cpu.h"

void tick_callback() {
    printf("Ticked");
    // do more stuff here if needed
}

int main() {
    u8 mem[0x10000]; // loaded from a file
    cpu_state_t st;
    
    // reads the reset address from 0xFFFE/0xFFFF into PC
    cpu_reset(st, mem);

    // execute instructions
    u16 pc;
    do {
        pc = st.PC;
        cpu_exec(st, mem);
    } while (st.PC != pc);

    return 0;
}
```

## TODO

- [ ] Hardware interrupt emulation
- [ ] Decimal Mode
- [ ] Undocumented Opcodes (addressing modes exist)

See `test/functional.c` for an example of loading memory from a 64k image

[1]: https://floooh.github.io/2019/12/13/cycle-stepped-6502.html#instruction-stepped-and-cycle-ticked
[2]: http://atarihq.com/danb/files/64doc.txt
[3]: https://github.com/Klaus2m5/6502_65C02_functional_tests
