#pragma once
//
//  mz800.h
//  mz-800-emulator
//
//  Created by Gunter Hager on 21.07.18.
//

#include "chips/z80.h"
#include "chips/z80pio.h"
#include "chips/z80ctc.h"
#include "chips/crt.h"
#include "chips/kbd.h"
#include "chips/mem.h"
#include "gdg_whid65040_032.h"

typedef struct {
    
    // CPU Z80A
    z80_t cpu;
    
    // PPI i8255, keyboard and cassette driver
    // CTC i8253, programmable counter/timer
    // PIO Z80 PIO, parallel I/O unit
    // PSG SN 76489 AN, sound generator
    
    // GDG WHID 65040-032, CRT controller
    gdg_whid65040_032_t gdg;
    
    // CRT
    crt_t crt;
    
    // Keyboard
    kbd_t kbd;
    
    // Memory
    mem_t mem;
    
    // ROM
    uint8_t rom1[0x1000];  // 0x0000-0x0fff
    uint8_t cgrom[0x1000]; // 0x1000-0x1fff
    uint8_t rom2[0x2000];  // 0xe000-0xffff
    
    // VRAM
    // 0x8000-0xbfff VRAM not mapped here, emulated by the GDG
    bool vram_banked_in;
    
    // RAM
    uint8_t dram0[0x1000]; // 0x0000-0x0fff
    uint8_t dram1[0x1000]; // 0x1000-0x1fff
    uint8_t dram2[0x6000]; // 0x2000-0x7fff
    uint8_t dram3[0x4000]; // 0x8000-0xbfff
    uint8_t dram4[0x2000]; // 0xc000-0xdfff
    uint8_t dram5[0x2000]; // 0xe000-0xffff
    
    // HALT callback
    void (*halt_cb)(z80_t *cpu);
    
} mz800_t;
