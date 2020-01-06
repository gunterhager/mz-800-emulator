#pragma once
//
//  mz800.h
//  mz-800-emulator
//
//  Created by Gunter Hager on 21.07.18.
//

#include "chips/z80.h"
#include "chips/z80pio.h"
#include "chips/i8255.h"
#include "chips/clk.h"
#include "chips/kbd.h"
#include "chips/mem.h"
#include "gdg_whid65040_032.h"

typedef struct {
    
    // CPU Z80A
    z80_t cpu;
    
    // PPI i8255, keyboard and cassette driver
	i8255_t ppi;
	
    // CTC i8253, programmable counter/timer
	// TODO: not implemented

    // PIO Z80 PIO, parallel I/O unit
	z80pio_t pio;
    
	// PSG SN 76489 AN, sound generator
	// TODO: not implemented
    
    // GDG WHID 65040-032, CRT controller
    gdg_whid65040_032_t gdg;
    
    // Clock
    clk_t clk;

    // Keyboard
    kbd_t kbd;
    
    // Memory
    mem_t mem;
    
    // ROM
//    uint8_t rom1[0x1000];  // 0x0000-0x0fff
//    uint8_t cgrom[0x1000]; // 0x1000-0x1fff
//    uint8_t rom2[0x2000];  // 0xe000-0xffff
    
    // VRAM
    // 0x8000-0xbfff VRAM not mapped here, emulated by the GDG
    bool vram_banked_in;
    
    // RAM (64K)
    uint8_t dram[0x10000]; // 0x0000-0xffff
    
    // HALT callback
    void (*halt_cb)(z80_t *cpu);
    
} mz800_t;
