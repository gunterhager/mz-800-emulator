//------------------------------------------------------------------------------
// mz800.c 
//
// Emulator for the SHARP MZ-800
//
//------------------------------------------------------------------------------

#include "sokol_app.h"
#define CHIPS_IMPL
#include "chips/z80.h"
#include "chips/z80pio.h"
#include "chips/z80ctc.h"
#include "chips/crt.h"
#include "chips/kbd.h"
#include "chips/mem.h"

#include "mzf.h"
#include "gdg_whid65040_032.h"
#include "roms/mz800-roms.h"
#include "mz800.h"

#include "common/gfx.h"
#include "common/fs.h"
#include "common/sound.h"
#include "common/clock.h"
#include "common/args.h"

#include <ctype.h> /* isupper, islower, toupper, tolower */

#define NOT_IMPLEMENTED false

/* MZ-800 emulator state and callbacks */
#define MZ800_FREQ (3546895) // 3.546895 MHz
#define MZ800_DISP_WIDTH (640)
#define MZ800_DISP_HEIGHT (200)

mz800_t mz800;

#define I(a) ((a) | Z80_RD | Z80_IORQ)
#define O(a) ((a) | Z80_WR | Z80_IORQ)

// Memory layout
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

uint32_t overrun_ticks;
uint64_t last_time_stamp;

// MARK: - Function declarations

void mz800_init(void);
void mz800_init_memory_mapping(void);
void mz800_update_memory_mapping(uint64_t pins);
uint64_t mz800_cpu_tick(int num_ticks, uint64_t pins);
uint64_t mz800_cpu_iorq(uint64_t pins);

/* sokol-app entry, configure application callbacks and window */
void app_init(void);
void app_frame(void);
void app_input(const sapp_event*);
void app_cleanup(void);


// MARK: - Main

sapp_desc sokol_main(int argc, char* argv[]) {
    args_init(argc, argv);
    fs_init();
    if (args_has("file")) {
        fs_load_file(args_string("file"));
    }
    return (sapp_desc) {
        .init_cb = app_init,
        .frame_cb = app_frame,
        .event_cb = app_input,
        .cleanup_cb = app_cleanup,
        .width = MZ800_DISP_WIDTH,
        .height = 2 * MZ800_DISP_HEIGHT,
        .window_title = "MZ-800"
    };
}


// MARK: - App

/* one-time application init */
void app_init() {
    gfx_init(MZ800_DISP_WIDTH, MZ800_DISP_HEIGHT, 1, 2);
    sound_init();
    clock_init(MZ800_FREQ);
    mz800_init();
 }

/* per frame stuff, tick the emulator, handle input, decode and draw emulator display */
void app_frame() {
    clock_ticks_executed(z80_exec(&mz800.cpu, clock_ticks_to_run()));
    // TODO: Keyboard update
    // kbd_update(&mz800.kbd);
    gfx_draw();
    
    /* load MZF file? */
    if (fs_ptr() && clock_frame_count() > 20) {
        mzf_load(fs_ptr(), fs_size(), &mz800.cpu, mz800.dram0);
        fs_free();
    }
}

/* keyboard input handling */
void app_input(const sapp_event* event) {
}

/* application cleanup callback */
void app_cleanup() {
    sound_shutdown();
    gfx_shutdown();
}

// MARK: - MZ-800 specific functions

void mz800_init(void) {
    mz800_init_memory_mapping();
    z80_init(&mz800.cpu, mz800_cpu_tick);
        
    gdg_whid65040_032_init(&mz800.gdg);

    // CPU start address
    mz800.cpu.state.PC = 0x0000;
}

/**
 Setup the initial memory mapping with ROM1, CGROM and ROM2, the rest is DRAM.
 */
void mz800_init_memory_mapping(void) {
    // According to SHARP Service Manual
    mem_map_rom(&mz800.mem, 0, 0x0000, 0x1000, dump_mz800_rom1);
    mem_map_ram(&mz800.mem, 0, 0x1000, 0x1000, dump_mz800_cgrom); // Character ROM
    mem_map_ram(&mz800.mem, 0, 0x2000, 0x6000, mz800.dram2);
    mz800.vram_banked_in = true;
    mem_map_ram(&mz800.mem, 0, 0x8000, 0x4000, mz800.dram3); // VRAM isn't handled by regular memory mapping
    mem_map_ram(&mz800.mem, 0, 0xc000, 0x2000, mz800.dram4);
    mem_map_rom(&mz800.mem, 0, 0xe000, 0x2000, dump_mz800_rom2);
}

/**
 Updates the memory mapping for the bank switch IO requests.

 @param pins Z80 pins with IO request for bank switching.
 */
void mz800_update_memory_mapping(uint64_t pins) {
    uint64_t pins_to_check = pins & (Z80_RD | Z80_WR | Z80_IORQ | 0xff);
    if (pins_to_check == mz800_mem_banks[0]) {
        mem_map_rom(&mz800.mem, 0, 0x1000, 0x1000, mz800.cgrom);
        mz800.vram_banked_in = true;
    } else if (pins_to_check == mz800_mem_banks[1]) {
        mem_map_ram(&mz800.mem, 0, 0x1000, 0x1000, mz800.dram1);
        mz800.vram_banked_in = false;
    } else if (pins_to_check == mz800_mem_banks[2]) {
        mem_map_ram(&mz800.mem, 0, 0x0000, 0x1000, mz800.dram0);
        mem_map_ram(&mz800.mem, 0, 0x1000, 0x1000, mz800.dram1);
    } else if (pins_to_check == mz800_mem_banks[3]) {
        mem_map_ram(&mz800.mem, 0, 0xe000, 0x2000, mz800.dram5);
    } else if (pins_to_check == mz800_mem_banks[4]) {
        mem_map_rom(&mz800.mem, 0, 0x0000, 0x1000, dump_mz800_rom1);
    } else if (pins_to_check == mz800_mem_banks[5]) {
        mem_map_rom(&mz800.mem, 0, 0xe000, 0x2000, dump_mz800_rom2);
    } else if (pins_to_check == mz800_mem_banks[6]) {
        mem_map_rom(&mz800.mem, 0, 0x0000, 0x1000, dump_mz800_rom1);
        mem_map_rom(&mz800.mem, 0, 0x1000, 0x1000, dump_mz800_cgrom);
        mz800.vram_banked_in = true;
        mem_map_rom(&mz800.mem, 0, 0xe000, 0x2000, dump_mz800_rom2);
    } else if (pins_to_check == mz800_mem_banks[7]) {
        // PROHIBIT not implemented
        CHIPS_ASSERT(NOT_IMPLEMENTED);
    } else if (pins_to_check == mz800_mem_banks[8]) {
        // RETURN not implemented
        CHIPS_ASSERT(NOT_IMPLEMENTED);
    }
}

uint64_t mz800_cpu_tick(int num_ticks, uint64_t pins) {
    uint64_t out_pins = pins;
    
    // HALT callback, used for unit tests
    if(pins & Z80_HALT) {
        mz800.halt_cb(&mz800.cpu);
    }
    
    // TODO: interrupt acknowledge
    
    // Memory request
    if (pins & Z80_MREQ) {
        const uint16_t addr = Z80_GET_ADDR(pins);
        if (mz800.vram_banked_in && ((addr & 0xc000) == 0x8000)) { // Address range of VRAM
            uint16_t vram_addr = addr - 0x8000;
            if (pins & Z80_RD) {
                Z80_SET_DATA(out_pins, gdg_whid65040_032_mem_rd(&mz800.gdg, vram_addr));
            }
            else if (pins & Z80_WR) {
                gdg_whid65040_032_mem_wr(&mz800.gdg, vram_addr, Z80_GET_DATA(pins), rgba8_buffer);
            }
        } else {
            if (pins & Z80_RD) {
                Z80_SET_DATA(out_pins, mem_rd(&mz800.mem, addr));
            }
            else if (pins & Z80_WR) {
                mem_wr(&mz800.mem, addr, Z80_GET_DATA(pins));
            }
        }
    }
    
    // IO request
    if ((pins & Z80_IORQ) && (pins & (Z80_RD|Z80_WR))) {
        out_pins = mz800_cpu_iorq(pins);
    }

    return out_pins;
}

#define IN_RANGE(A,B,C) (((A)>=(B))&&((A)<=(C)))

uint64_t mz800_cpu_iorq(uint64_t pins) {
    uint16_t address = Z80_GET_ADDR(pins) & 0xff; // check only the lower byte of the address
    
    // Serial I/O
    if (IN_RANGE(address, 0xb0, 0xb3)) {
        // TODO: not implemented
        CHIPS_ASSERT(NOT_IMPLEMENTED);
    }
    // GDG WHID 65040-032, CRT controller
    else if (IN_RANGE(address, 0xcc, 0xcf)) {
        gdg_whid65040_032_iorq(&mz800.gdg, pins);
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
        mz800_update_memory_mapping(pins);
    }
    // Joystick (read only)
    else if ((pins & Z80_RD) && (IN_RANGE(address, 0xf0, 0xf1))) {
        // TODO: not implemented
        CHIPS_ASSERT(NOT_IMPLEMENTED);
    }
    // GDG WHID 65040-032, Palette register (write only)
    else if ((pins & Z80_WR) && (address == 0xf0)) {
        gdg_whid65040_032_iorq(&mz800.gdg, pins);
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
    
    return pins;
}
