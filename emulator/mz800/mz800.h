#pragma once
//
//  mz800.h
//  mz-800-emulator
//
//  Created by Gunter Hager on 21.07.18.
//

#include "chips/chips_common.h"
#include "chips/z80.h"
#include "chips/z80pio.h"
#include "chips/i8255.h"
#include "chips/clk.h"
#include "chips/kbd.h"
#include "chips/mem.h"
#include "gdg_whid65040_032.h"

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
	const void* ptr;
	size_t size;
} mz800_rom_image_t;

// configuration parameters for mz800_init()
typedef struct {
	mz800_debug_t debug;

	// video output config
	struct {
		void* ptr;              // pointer to a linear RGBA8 pixel buffer
		size_t size;            // size of the pixel buffer in bytes
	} pixel_buffer;

	// audio output config (if you don't want audio, set callback.func to zero)
	struct {
		mz800_audio_callback_t callback;  // called when audio_num_samples are ready */
		int num_samples;                  // default is MZ800_DEFAULT_AUDIO_SAMPLES
		int sample_rate;                  // playback sample rate, default is 44100
		float volume;                     // audio volume: 0.0..1.0, default is 0.25
	} audio;

	// ROM images
	struct {
		mz800_rom_image_t rom1;
		mz800_rom_image_t rom2;
		mz800_rom_image_t cgrom; // Character ROM
	} roms;
} mz800_desc_t;

// Memory sizes and locations
#define MZ800_ROM1_SIZE     0x1000
#define MZ800_CGROM_SIZE    0x1000
#define MZ800_ROM2_SIZE     0x2000

#define MZ800_ROM1_START    0x0000
#define MZ800_CGROM_START   0x1000
#define MZ800_ROM2_START    0xe000

// There are different VRAM addresses for MZ-700 and MZ-800 modes
#define MZ800_VRAM_START    0x8000
#define MZ800_VRAM_320_END  0xa000
#define MZ800_VRAM_640_END  0xc000
#define MZ800_VRAM_320_SIZE (MZ800_VRAM_320_END - MZ800_VRAM_START)
#define MZ800_VRAM_640_SIZE (MZ800_VRAM_640_END - MZ800_VRAM_START)

#define MZ700_VRAM_START    0xd000
#define MZ700_VRAM_END      0xe000
#define MZ700_VRAM_SIZE     (MZ700_VRAM_END - MZ700_VRAM_START)

// MZ-700 Memory mapped IO
#define MZ700_IO_START    0xe000
#define MZ700_IO_END      0xe009

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
	uint8_t rom1[MZ800_ROM1_SIZE];   // 0x0000-0x0fff
	uint8_t cgrom[MZ800_CGROM_SIZE]; // 0x1000-0x1fff
	uint8_t rom2[MZ800_ROM2_SIZE];   // 0xe000-0xffff

	// VRAM
	// VRAM not mapped here, emulated by the GDG
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
// Memory layout differs for MZ-700 mode.
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

void mz800_init(mz800_t* sys, mz800_desc_t* desc);
void mz800_discard(mz800_t* sys);
void mz800_reset(mz800_t* sys);
void mz800_init_roms(mz800_t* sys, mz800_desc_t* desc);
void mz800_init_memory_mapping(mz800_t* sys);
uint64_t mz800_update_memory_mapping(mz800_t* sys, uint64_t cpu_pins);
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

// Display size
#define MZ800_DISP_WIDTH (640)
#define MZ800_DISP_HEIGHT (200)

#define _MZ800_DEFAULT(val,def) (((val) != 0) ? (val) : (def))

// MARK: - MZ-800 specific functions

void mz800_init(mz800_t* sys, mz800_desc_t* desc) {
	CHIPS_ASSERT(sys && desc);
	sys->valid = true;
	sys->debug = desc->debug;
	sys->audio.callback = desc->audio.callback;
	sys->audio.num_samples = _MZ800_DEFAULT(desc->audio.num_samples, MZ800_DEFAULT_AUDIO_SAMPLES);
	CHIPS_ASSERT(sys->audio.num_samples <= MZ800_MAX_AUDIO_SAMPLES);

	// Initialize hardware
	sys->pins = z80_init(&sys->cpu);
	z80pio_init(&sys->pio);
	i8255_init(&sys->ppi);
	gdg_whid65040_032_desc_t gdg_desc = (gdg_whid65040_032_desc_t) {
		.ntpl = false, // PAL
		.cgrom = sys->cgrom,
		.rgba8_buffer = desc->pixel_buffer.ptr,
		.rgba8_buffer_size = desc->pixel_buffer.size
	};
	gdg_whid65040_032_init(&sys->gdg, &gdg_desc);
	mem_init(&sys->mem);
	mz800_init_roms(sys, desc);
	mz800_init_memory_mapping(sys);
}

void mz800_discard(mz800_t* sys) {
	CHIPS_ASSERT(sys && sys->valid);
	sys->valid = false;
}

void mz800_reset(mz800_t* sys) {
	CHIPS_ASSERT(sys && sys->valid);
	// Reset memory
	mem_unmap_all(&sys->mem);
	memset(sys->dram, 0, sizeof(sys->dram));
	mz800_init_memory_mapping(sys);

	// Reset chips
	z80pio_reset(&sys->pio);
	i8255_reset(&sys->ppi);
	gdg_whid65040_032_reset(&sys->gdg);
	sys->pins = z80_reset(&sys->cpu);
}

void mz800_init_roms(mz800_t* sys, mz800_desc_t* desc) {
	CHIPS_ASSERT(sys && desc);
	CHIPS_ASSERT(desc->roms.rom1.ptr && (desc->roms.rom1.size == 0x1000));
	CHIPS_ASSERT(desc->roms.cgrom.ptr && (desc->roms.cgrom.size == 0x1000));
	CHIPS_ASSERT(desc->roms.rom2.ptr && (desc->roms.rom2.size == 0x2000));
	memcpy(sys->rom1, desc->roms.rom1.ptr, 0x1000);
	memcpy(sys->cgrom, desc->roms.cgrom.ptr, 0x1000);
	memcpy(sys->rom2, desc->roms.rom2.ptr, 0x2000);
}

/**
 Setup the initial memory mapping with ROM1, CGROM and ROM2, the rest is DRAM.
 */
void mz800_init_memory_mapping(mz800_t* sys) {
	CHIPS_ASSERT(sys);

	// According to SHARP Service Manual
	mem_map_rom(&sys->mem, 0, 0x0000, 0x1000, sys->rom1);
	mem_map_rom(&sys->mem, 0, 0x1000, 0x1000, sys->cgrom); // Character ROM
	mem_map_ram(&sys->mem, 0, 0x2000, 0x6000, sys->dram + 0x2000);
	sys->vram_banked_in = true;
	mem_map_ram(&sys->mem, 0, 0x8000, 0x4000, sys->dram + 0x8000); // VRAM isn't handled by regular memory mapping
	mem_map_ram(&sys->mem, 0, 0xc000, 0x2000, sys->dram + 0xc000);
	mem_map_rom(&sys->mem, 0, 0xe000, 0x2000, sys->rom2);
}

/**
 Updates the memory mapping for the bank switch IO requests.

 @param pins Z80 pins with IO request for bank switching.
 */
uint64_t mz800_update_memory_mapping(mz800_t* sys, uint64_t cpu_pins) {
	uint64_t pins_to_check = cpu_pins & (Z80_RD | Z80_WR | Z80_IORQ | 0xff);

	switch (pins_to_check) {

			// DRAM_DRAM_X_X
		case O(0xe0):
			mem_map_ram(&sys->mem, 0, 0x0000, 0x2000, sys->dram);
			break;

			// X_X_X_DRAM
		case O(0xe1):
			if (sys->gdg.is_mz700) {
				mem_map_ram(&sys->mem, 0, MZ700_VRAM_START, 0x3000, sys->dram + MZ700_VRAM_START);
			} else {
				mem_map_ram(&sys->mem, 0, 0xe000, 0x2000, sys->dram + 0xe000);
			}
			break;

			// ROM1_X_X_X
		case O(0xe2):
			mem_map_rom(&sys->mem, 0, 0x0000, 0x1000, sys->rom1);
			break;

			// X_X_X_ROM2
		case O(0xe3):
			// Special treatment in MZ-700: VRAM in 0xd000-0xdfff, Key, Timer in 0xe000-0xe070
			// This isn't handled by regular memory mapping
			if (sys->gdg.is_mz700) {
				sys->vram_banked_in = true;
			}
			mem_map_rom(&sys->mem, 0, 0xe000, 0x2000, sys->rom2);
			break;

			// ROM1_CGROM_VRAM_ROM2
		case O(0xe4):
			mem_map_rom(&sys->mem, 0, 0x0000, 0x1000, sys->rom1);
			if (!sys->gdg.is_mz700) {
				mem_map_rom(&sys->mem, 0, 0x1000, 0x1000, sys->cgrom);
			}
			sys->vram_banked_in = true;
			mem_map_rom(&sys->mem, 0, 0xe000, 0x2000, sys->rom2);
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
			mem_map_rom(&sys->mem, 0, 0x1000, 0x1000, sys->cgrom);
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
	return cpu_pins;
}

uint32_t mz800_exec(mz800_t* sys, uint32_t micro_seconds) {
	CHIPS_ASSERT(sys && sys->valid);

	const uint32_t num_ticks = clk_us_to_ticks(sys->gdg.cpu_clk0, micro_seconds);
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

static bool _mz800_is_mz700VRAM_addr(mz800_t* sys, uint16_t addr) {
	if (!sys->vram_banked_in) { return false; }
	if (!sys->gdg.is_mz700) { return false; }
	return (addr >= MZ700_VRAM_START) && (addr < MZ700_VRAM_END);
}

static bool _mz800_is_VRAM_addr(mz800_t* sys, uint16_t addr) {
	if (!sys->vram_banked_in) { return false; }
	if (sys->gdg.is_mz700) { return false; }
	if (addr < MZ800_VRAM_START) { return false; }
	bool is_640 = (sys->gdg.dmd & GDG_DMD_640);
	return addr < (is_640 ? MZ800_VRAM_640_END : MZ800_VRAM_320_END);
}

#define _Z80_ADDR_MASK (0xffffULL)

/// Translates memory mapped IO for MZ-700 mode to proper IO requests
static uint64_t _mz700_translate_iorq(mz800_t* sys, uint64_t cpu_pins) {
	uint16_t io_addr = 0;

	switch (cpu_pins & (_Z80_ADDR_MASK | Z80_RD | Z80_WR)) {
			// i8255
		case (MZ700_IO_START        | Z80_WR): io_addr = 0xd0; break; // W
		case (MZ700_IO_START + 0x01 | Z80_RD): io_addr = 0xd1; break; // R
		case (MZ700_IO_START + 0x02 | Z80_WR): io_addr = 0xd2; break; // R/W
		case (MZ700_IO_START + 0x02 | Z80_RD): io_addr = 0xd2; break;
		case (MZ700_IO_START + 0x03 | Z80_WR): io_addr = 0xd3; break; // W

			// i8253
		case (MZ700_IO_START + 0x04 | Z80_WR): io_addr = 0xd4; break; // R/W
		case (MZ700_IO_START + 0x04 | Z80_RD): io_addr = 0xd4; break;
		case (MZ700_IO_START + 0x05 | Z80_WR): io_addr = 0xd5; break; // R/W
		case (MZ700_IO_START + 0x05 | Z80_RD): io_addr = 0xd5; break;
		case (MZ700_IO_START + 0x06 | Z80_WR): io_addr = 0xd6; break; // R/W
		case (MZ700_IO_START + 0x06 | Z80_RD): io_addr = 0xd6; break;
		case (MZ700_IO_START + 0x07 | Z80_WR): io_addr = 0xd7; break; // W

			// Implementation of MZ-700 0xe008 is a bit unclear
//		case (MZ700_IO_START + 0x08 | Z80_WR): io_addr = 0xd7; break; // R/W
		case (MZ700_IO_START + 0x08 | Z80_RD): io_addr = 0xce; break;

		default:
			break;
	}

	if (io_addr != 0) {
		cpu_pins &= ~Z80_MREQ; cpu_pins |= Z80_IORQ;
		Z80_SET_ADDR(cpu_pins, io_addr);
	}
	return mz800_cpu_iorq(sys, cpu_pins);
}

#undef _Z80_ADDR_MASK

#define IN_RANGE(A,B,C) (((A)>=(B))&&((A)<=(C)))

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
		// MZ-700 VRAM range
		if (_mz800_is_mz700VRAM_addr(sys, addr)) {
			uint16_t vram_addr = addr - MZ700_VRAM_START;
			if (cpu_pins & Z80_RD) {
				Z80_SET_DATA(cpu_pins, gdg_whid65040_032_mem_rd(&sys->gdg, vram_addr));
			}
			else if (cpu_pins & Z80_WR) {
				gdg_whid65040_032_mem_wr(&sys->gdg, vram_addr, Z80_GET_DATA(cpu_pins));
			}
		}
		// MZ-800 VRAM range
		else if (_mz800_is_VRAM_addr(sys, addr)) {
			uint16_t vram_addr = addr - MZ800_VRAM_START;
			if (cpu_pins & Z80_RD) {
				Z80_SET_DATA(cpu_pins, gdg_whid65040_032_mem_rd(&sys->gdg, vram_addr));
			}
			else if (cpu_pins & Z80_WR) {
				gdg_whid65040_032_mem_wr(&sys->gdg, vram_addr, Z80_GET_DATA(cpu_pins));
			}
		}
		// MZ-700 memory mapped IO
		else if (sys->gdg.is_mz700
				 && IN_RANGE(addr, MZ700_IO_START, MZ700_IO_END)) {
			cpu_pins = _mz700_translate_iorq(sys, cpu_pins);
		}
		// Other memory
		else {
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

static uint64_t mz800_cpu_iorq(mz800_t* sys, uint64_t cpu_pins) {
	uint16_t address = Z80_GET_ADDR(cpu_pins) & 0xff; // check only the lower byte of the address

	// Serial I/O
	if (IN_RANGE(address, 0xb0, 0xb3)) {
		// TODO: not implemented
		CHIPS_ASSERT(NOT_IMPLEMENTED);
	}
	// GDG WHID 65040-032, CRT controller
	else if (IN_RANGE(address, 0xcc, 0xcf)) {
		cpu_pins = gdg_whid65040_032_tick(&sys->gdg, cpu_pins);
	}
	// PPI i8255, keyboard and cassette driver
	else if (IN_RANGE(address, 0xd0, 0xd3)) {
#warning "TODO: continue implementation of i8255 pins"
		uint64_t ppi_pins = (cpu_pins & Z80_PIN_MASK & ~I8255_PC_PINS) | I8255_CS;
		ppi_pins = i8255_tick(&sys->ppi, ppi_pins);
		// Copy data bus value to cpu pins
		if ((ppi_pins & (I8255_CS|I8255_RD)) == (I8255_CS|I8255_RD)) {
			Z80_SET_DATA(cpu_pins, I8255_GET_DATA(ppi_pins));
		}
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
		cpu_pins = mz800_update_memory_mapping(sys, cpu_pins);
	}
	// Joystick (read only)
	else if ((cpu_pins & Z80_RD) && (IN_RANGE(address, 0xf0, 0xf1))) {
		// TODO: not implemented
		CHIPS_ASSERT(NOT_IMPLEMENTED);
	}
	// GDG WHID 65040-032, Palette register (write only)
	else if ((cpu_pins & Z80_WR) && (address == 0xf0)) {
		cpu_pins = gdg_whid65040_032_tick(&sys->gdg, cpu_pins);
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
#warning "TODO: continue implementation of Z80 PIO pins"
		cpu_pins = z80pio_tick(&sys->pio, cpu_pins);
	}
	// DEBUG
	else {
		CHIPS_ASSERT(NOT_IMPLEMENTED);
	}

	return cpu_pins;
}

#undef IN_RANGE

#endif /* CHIPS_IMPL */
