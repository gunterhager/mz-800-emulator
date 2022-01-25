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
#include "../roms/mz800-roms.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MZ800_MAX_AUDIO_SAMPLES (1024)        // max number of audio samples in internal sample buffer
#define MZ800_DEFAULT_AUDIO_SAMPLES (128)     // default number of samples in internal sample buffer

// audio sample callback
typedef struct {
	void (*func)(const float* samples, int num_samples, void* user_data);
	void* user_data;
} mz800_audio_callback_t;

// debugging hook
typedef void (*mz800_debug_func_t)(void* user_data, uint64_t pins);
typedef struct {
	struct {
		mz800_debug_func_t func;
		void* user_data;
	} callback;
	bool* stopped;
} mz800_debug_t;

typedef struct {
    
    // CPU Z80A
    z80_t cpu;
    
    // PPI i8255, keyboard and cassette driver
	i8255_t ppi;

    // CTC i8253, programmable counter/timer

    // PIO Z80 PIO, parallel I/O unit
	z80pio_t pio;

	// PSG SN 76489 AN, sound generator
    
    // GDG WHID 65040-032, CRT controller
    gdg_whid65040_032_t gdg;
    
    // Keyboard
    kbd_t kbd;
    
    // Memory
    mem_t mem;
    
	uint64_t pins;
	bool valid;
	mz800_debug_t debug;

	struct {
		mz800_audio_callback_t callback;
		int num_samples;
		int sample_pos;
		float sample_buffer[MZ800_MAX_AUDIO_SAMPLES];
	} audio;

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

#define I(a) ((a) | Z80_RD | Z80_IORQ)
#define O(a) ((a) | Z80_WR | Z80_IORQ)

// MARK: - Memory layout
// Memory in the ranges 0x2000-0x7fff, 0xc000-0xdfff are always DRAM
// The memory bank values below can be used directly as bit mask for Z80 pins.
// | I/O        | 0x0000 | 0x1000 | 0x8000 | 0xe000 |
// | Address    | 0x0fff | 0x1fff | 0xbfff | 0xffff |
const uint32_t mz800_mem_banks[9] = {
	I(0xe0), // | x      | CGROM  | VRAM   | x      |
	I(0xe1), // | x      | DRAM   | DRAM   | x      |
	O(0xe0), // | DRAM   | DRAM   | x      | x      |
	O(0xe1), // | x      | x      | x      | DRAM   |
	O(0xe2), // | ROM    | x      | x      | x      |
	O(0xe3), // | x      | x      | x      | ROM    |
	O(0xe4), // | ROM    | CGROM  | VRAM   | ROM    |
	O(0xe5), // | x      | x      | x      | PROHIB | // Prohibited
	O(0xe6)  // | x      | x      | x      | RETURN | // Return to previous state
};

void mz800_init(mz800_t* sys);
void mz800_reset(mz800_t* sys);
void mz800_init_memory_mapping(mz800_t* sys);
void mz800_update_memory_mapping(mz800_t* sys, uint64_t pins);
uint32_t mz800_exec(mz800_t* sys, uint32_t micro_seconds);

static uint64_t mz800_cpu_tick(mz800_t* sys, uint64_t cpu_pins);
static uint64_t mz800_cpu_iorq(mz800_t* sys, uint64_t cpu_pins);

#ifdef __cplusplus
} // extern "C"
#endif

// MARK: - Implementation

#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_ASSERT
	#include <assert.h>
	#define CHIPS_ASSERT(c) assert(c)
#endif

#define NOT_IMPLEMENTED false

/* MZ-800 emulator state and callbacks */
#define MZ800_FREQ (3546895) // 3.546895 MHz
#define MZ800_DISP_WIDTH (640)
#define MZ800_DISP_HEIGHT (200)

// MARK: - MZ-800 specific functions

void mz800_init(mz800_t* sys) {
	CHIPS_ASSERT(sys);
	sys->valid = true;

	// Initialize hardware
	sys->pins = z80_init(&sys->cpu);
	z80pio_init(&sys->pio);
	i8255_init(&sys->ppi);
	gdg_whid65040_032_init(&sys->gdg, dump_mz800_cgrom_bin, gfx_framebuffer());
	mem_init(&sys->mem);
	mz800_init_memory_mapping(sys);
}

void mz800_reset(mz800_t* sys) {
	CHIPS_ASSERT(sys && sys->valid);
	mem_unmap_all(&sys->mem);
	sys->pins = z80_reset(&sys->cpu);
	z80pio_reset(&sys->pio);
	i8255_reset(&sys->ppi);
	gdg_whid65040_032_reset(&sys->gdg);
}

/**
 Setup the initial memory mapping with ROM1, CGROM and ROM2, the rest is DRAM.
 */
void mz800_init_memory_mapping(mz800_t* sys) {
	// According to SHARP Service Manual
	mem_map_rom(&sys->mem, 0, 0x0000, 0x1000, dump_mz800_rom1_bin);
	mem_map_ram(&sys->mem, 0, 0x1000, 0x1000, dump_mz800_cgrom_bin); // Character ROM
	mem_map_ram(&sys->mem, 0, 0x2000, 0x6000, sys->dram + 0x2000);
	sys->vram_banked_in = true;
	mem_map_ram(&sys->mem, 0, 0x8000, 0x4000, sys->dram + 0x8000); // VRAM isn't handled by regular memory mapping
	mem_map_ram(&sys->mem, 0, 0xc000, 0x2000, sys->dram + 0xc000);
	mem_map_rom(&sys->mem, 0, 0xe000, 0x2000, dump_mz800_rom2_bin);
}

/**
 Updates the memory mapping for the bank switch IO requests.

 @param pins Z80 pins with IO request for bank switching.
 */
void mz800_update_memory_mapping(mz800_t* sys, uint64_t pins) {
	uint64_t pins_to_check = pins & (Z80_RD | Z80_WR | Z80_IORQ | 0xff);

	switch (pins_to_check) {

		case O(0xe0):
		mem_map_ram(&sys->mem, 0, 0x0000, 0x2000, sys->dram);
		break;

		case O(0xe1):
		if (sys->gdg.is_mz700) {
			mem_map_ram(&sys->mem, 0, 0xd000, 0x3000, sys->dram + 0xd000);
		} else {
			mem_map_ram(&sys->mem, 0, 0xe000, 0x2000, sys->dram + 0xe000);
		}
		break;

		case O(0xe2):
			mem_map_rom(&sys->mem, 0, 0x0000, 0x1000, dump_mz800_rom1_bin);
		break;

		case O(0xe3):
		// Special treatment in MZ-700: VRAM in 0xd000-0xdfff, Key, Timer in 0xe000-0xe070
		// This isn't handled by regular memory mapping
		if (sys->gdg.is_mz700) {
			sys->vram_banked_in = true;
		}
			mem_map_rom(&sys->mem, 0, 0xe000, 0x2000, dump_mz800_rom2_bin);
		break;

		case O(0xe4):
			mem_map_rom(&sys->mem, 0, 0x0000, 0x1000, dump_mz800_rom1_bin);
		if (!sys->gdg.is_mz700) {
			mem_map_rom(&sys->mem, 0, 0x1000, 0x1000, dump_mz800_cgrom_bin);
		}
			sys->vram_banked_in = true;
			mem_map_rom(&sys->mem, 0, 0xe000, 0x2000, dump_mz800_rom2_bin);
		break;

		case O(0xe5):
		// ROM prohibited mode; not sure if i need to support this.
		// PROHIBIT not implemented
		CHIPS_ASSERT(NOT_IMPLEMENTED);
		break;

		case O(0xe6):
		// ROM return to previous state mode; not sure if i need to support this.
		// RETURN not implemented
		CHIPS_ASSERT(NOT_IMPLEMENTED);
		break;

		case I(0xe0):
		mem_map_rom(&sys->mem, 0, 0x1000, 0x1000, dump_mz800_cgrom_bin);
			sys->vram_banked_in = true;
		break;

		case I(0xe1):
		mem_map_ram(&sys->mem, 0, 0x1000, 0x1000, sys->dram + 0x1000);
			sys->vram_banked_in = false;
		break;

		default:
		// Should never happen.
		CHIPS_ASSERT(NOT_IMPLEMENTED);
		break;
	}
}

uint32_t mz800_exec(mz800_t* sys, uint32_t micro_seconds) {
	CHIPS_ASSERT(sys && sys->valid);
	const uint32_t num_ticks = clk_us_to_ticks(MZ800_FREQ, micro_seconds);
	uint64_t pins = sys->pins;
	if (0 == sys->debug.callback.func) {
		// run without debug hook
		for (uint32_t tick = 0; tick < num_ticks; tick++) {
			pins = mz800_cpu_tick(sys, pins);
		}
	}
	else {
		// run with debug hook
		for (uint32_t tick = 0; (tick < num_ticks) && !(*sys->debug.stopped); tick++) {
			pins = mz800_cpu_tick(sys, pins);
			sys->debug.callback.func(sys->debug.callback.user_data, pins);
		}
	}
	sys->pins = pins;
	kbd_update(&sys->kbd, micro_seconds);
	return num_ticks;
}

static uint64_t mz800_cpu_tick(mz800_t* sys, uint64_t cpu_pins) {
	cpu_pins = z80_tick(&sys->cpu, cpu_pins);

	// HALT callback, used for unit tests
	if (cpu_pins & Z80_HALT) {
		if (sys->halt_cb) {
			sys->halt_cb(&sys->cpu);
		}
	}

	// TODO: interrupt acknowledge

	// Memory request
	if (cpu_pins & Z80_MREQ) {
		const uint16_t addr = Z80_GET_ADDR(cpu_pins);
		if (sys->vram_banked_in // MZ-700 VRAM range
			&& sys->gdg.is_mz700
			&& (addr >= 0xd000) && (addr < 0xe000)) {
			uint16_t vram_addr = addr - 0xd000;
			if (cpu_pins & Z80_RD) {
				Z80_SET_DATA(cpu_pins, gdg_whid65040_032_mem_rd(&sys->gdg, vram_addr));
			}
			else if (cpu_pins & Z80_WR) {
				gdg_whid65040_032_mem_wr(&sys->gdg, vram_addr, Z80_GET_DATA(cpu_pins));
			}
		} else if (sys->vram_banked_in // MZ-800 VRAM range
				   && !sys->gdg.is_mz700
				   && (addr >= 0x8000) && (addr < 0xc000)) {
			uint16_t vram_addr = addr - 0x8000;
			if (cpu_pins & Z80_RD) {
				Z80_SET_DATA(cpu_pins, gdg_whid65040_032_mem_rd(&sys->gdg, vram_addr));
			}
			else if (cpu_pins & Z80_WR) {
				gdg_whid65040_032_mem_wr(&sys->gdg, vram_addr, Z80_GET_DATA(cpu_pins));
			}
		} else { // other memory
			if (cpu_pins & Z80_RD) {
				Z80_SET_DATA(cpu_pins, mem_rd(&sys->mem, addr));
			}
			else if (cpu_pins & Z80_WR) {
				mem_wr(&sys->mem, addr, Z80_GET_DATA(cpu_pins));
			}
		}
	}

	// IO request
	else if ((cpu_pins & Z80_IORQ) && (cpu_pins & (Z80_RD|Z80_WR))) {
		cpu_pins = mz800_cpu_iorq(sys, cpu_pins);
	}

	return cpu_pins;
}

#define IN_RANGE(A,B,C) (((A)>=(B))&&((A)<=(C)))

static uint64_t mz800_cpu_iorq(mz800_t* sys, uint64_t cpu_pins) {
	uint16_t address = Z80_GET_ADDR(cpu_pins) & 0xff; // check only the lower byte of the address

	// Serial I/O
	if (IN_RANGE(address, 0xb0, 0xb3)) {
		// TODO: not implemented
		CHIPS_ASSERT(NOT_IMPLEMENTED);
	}
	// GDG WHID 65040-032, CRT controller
	else if (IN_RANGE(address, 0xcc, 0xcf)) {
		gdg_whid65040_032_tick(&sys->gdg, cpu_pins);
	}
	// PPI i8255, keyboard and cassette driver
	else if (IN_RANGE(address, 0xd0, 0xd3)) {
		// TODO: not implemented
		CHIPS_ASSERT(NOT_IMPLEMENTED);
	}
	// CTC i8253, programmable counter/timer
	else if (IN_RANGE(address, 0xd4, 0xd7)) {
		// TODO: not implemented
		CHIPS_ASSERT(NOT_IMPLEMENTED);
	}
	// FDC, floppy disc controller
	else if (IN_RANGE(address, 0xd8, 0xdf)) {
		// TODO: not implemented
		CHIPS_ASSERT(NOT_IMPLEMENTED);
	}
	// GDG WHID 65040-032, Memory bank switch
	else if (IN_RANGE(address, 0xe0, 0xe6)) {
		// Currently this isn't supported by the GDG emulation,
		// so we do the bank switch directly here.
		mz800_update_memory_mapping(sys, cpu_pins);
	}
	// Joystick (read only)
	else if ((cpu_pins & Z80_RD) && (IN_RANGE(address, 0xf0, 0xf1))) {
		// TODO: not implemented
		CHIPS_ASSERT(NOT_IMPLEMENTED);
	}
	// GDG WHID 65040-032, Palette register (write only)
	else if ((cpu_pins & Z80_WR) && (address == 0xf0)) {
		gdg_whid65040_032_tick(&sys->gdg, cpu_pins);
	}
	// PSG SN 76489 AN, sound generator
	else if (address == 0xf2) {
		// TODO: not implemented
		CHIPS_ASSERT(NOT_IMPLEMENTED);
	}
	// QDC, quick disk controller
	else if (IN_RANGE(address, 0xf4, 0xf7)) {
		// TODO: not implemented
		CHIPS_ASSERT(NOT_IMPLEMENTED);
	}
	// PIO Z80 PIO, parallel I/O unit
	else if (IN_RANGE(address, 0xfc, 0xff)) {
		// TODO: not implemented
		CHIPS_ASSERT(NOT_IMPLEMENTED);
	}
	// DEBUG
	else {
		CHIPS_ASSERT(NOT_IMPLEMENTED);
	}

	return cpu_pins;
}

#endif /* CHIPS_IMPL */
