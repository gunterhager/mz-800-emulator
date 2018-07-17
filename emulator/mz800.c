//------------------------------------------------------------------------------
// mz800.c 
//
// Emulator for the SHARP MZ-800
//
//------------------------------------------------------------------------------

#include "sokol_app.h"
#include "sokol_time.h"
#define CHIPS_IMPL
#include "chips/z80.h"
#include "chips/z80pio.h"
#include "chips/z80ctc.h"
#include "chips/crt.h"
#include "chips/kbd.h"
#include "chips/mem.h"
#include "gdg_whid65040_032.h"
#include "roms/mz800-roms.h"
#include "common/gfx.h"
#include <ctype.h> /* isupper, islower, toupper, tolower */

/* MZ-800 emulator state and callbacks */
#define MZ800_FREQ (3546895) // 3.546895 MHz
#define MZ800_DISP_WIDTH (640)
#define MZ800_DISP_HEIGHT (200)

typedef struct {
    
    // CPU Z80A
    z80_t cpu;
    uint32_t tick_count;
    
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



} mz800_t;
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
void mz800_decode_vram(void);

/* sokol-app entry, configure application callbacks and window */
void app_init(void);
void app_frame(void);
void app_input(const sapp_event*);
void app_cleanup(void);


// MARK: - Main

sapp_desc sokol_main(int argc, char* argv[]) {
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
    gfx_init(MZ800_DISP_WIDTH, MZ800_DISP_HEIGHT);
    
    // Clear screen
    for (uint32_t index = 0; index < MZ800_DISP_WIDTH * MZ800_DISP_HEIGHT; index++) {
        rgba8_buffer[index] = 0xffff0000;
    }

    mz800_init();
    last_time_stamp = stm_now();
}

/* per frame stuff, tick the emulator, handle input, decode and draw emulator display */
void app_frame() {
    double frame_time = stm_sec(stm_laptime(&last_time_stamp));
    /* skip long pauses when the app was suspended */
    if (frame_time > 0.1) {
        frame_time = 0.1;
    }
    uint32_t ticks_to_run = (uint32_t) ((MZ800_FREQ * frame_time) - overrun_ticks);
    uint32_t ticks_executed = z80_exec(&mz800.cpu, ticks_to_run);
    assert(ticks_executed >= ticks_to_run);
    overrun_ticks = ticks_executed - ticks_to_run;
    // TODO: Keyboard update
    // kbd_update(&mz800.kbd);
    gfx_draw();
}

/* keyboard input handling */
void app_input(const sapp_event* event) {
}

/* application cleanup callback */
void app_cleanup() {
    gfx_shutdown();
}

// MARK: - MZ-800 specific functions

void mz800_init(void) {
    mz800.tick_count = 0;
    
    mz800_init_memory_mapping();
    z80_init(&mz800.cpu, mz800_cpu_tick);
    
    // CPU start address
    mz800.cpu.state.PC = 0x2000;
    
    gdg_whid65040_032_init(&mz800.gdg);
}

/**
 Setup the initial memory mapping with ROM1 and ROM2, the rest is DRAM.
 */
void mz800_init_memory_mapping(void) {
    // According to SHARP Service Manual
    mem_map_rom(&mz800.mem, 0, 0x0000, 0x1000, mz800.rom1);
    mem_map_ram(&mz800.mem, 0, 0x1000, 0x1000, dump_mz800_cgrom); // Character ROM
    mem_map_ram(&mz800.mem, 0, 0x2000, 0x6000, dump_mz800_dram2); // 'load' custom program
    mz800.vram_banked_in = true;
    mem_map_ram(&mz800.mem, 0, 0x8000, 0x4000, mz800.dram3); // VRAM isn't handled by regular memory mapping
    mem_map_ram(&mz800.mem, 0, 0xc000, 0x2000, mz800.dram4);
    mem_map_rom(&mz800.mem, 0, 0xe000, 0x2000, mz800.rom2);
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
        mem_map_rom(&mz800.mem, 0, 0x0000, 0x1000, mz800.rom1);
    } else if (pins_to_check == mz800_mem_banks[5]) {
        mem_map_rom(&mz800.mem, 0, 0xe000, 0x2000, mz800.rom2);
    } else if (pins_to_check == mz800_mem_banks[6]) {
        mem_map_rom(&mz800.mem, 0, 0x0000, 0x1000, mz800.rom1);
        mem_map_rom(&mz800.mem, 0, 0x1000, 0x1000, dump_mz800_cgrom);
        mz800.vram_banked_in = true;
        mem_map_rom(&mz800.mem, 0, 0xe000, 0x2000, mz800.rom2);
    } else if (pins_to_check == mz800_mem_banks[7]) {
        // PROHIBIT not implemented
    } else if (pins_to_check == mz800_mem_banks[8]) {
        // RETURN not implemented
    }
}

uint64_t mz800_cpu_tick(int num_ticks, uint64_t pins) {
    uint64_t out_pins = pins;
    
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
    }
    // GDG WHID 65040-032, CRT controller
    else if (IN_RANGE(address, 0xcc, 0xcf)) {
        gdg_whid65040_032_iorq(&mz800.gdg, pins);
    }
    // PPI i8255, keyboard and cassette driver
    else if (IN_RANGE(address, 0xd0, 0xd3)) {
        // TODO: not implemented
    }
    // CTC i8253, programmable counter/timer
    else if (IN_RANGE(address, 0xd4, 0xd7)) {
        // TODO: not implemented
    }
    // FDC, floppy disc controller
    else if (IN_RANGE(address, 0xd8, 0xdf)) {
        // TODO: not implemented
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
    }
    // GDG WHID 65040-032, Palette register (write only)
    else if ((pins & Z80_WR) && (address == 0xf0)) {
        gdg_whid65040_032_iorq(&mz800.gdg, pins);
    }
    // PSG SN 76489 AN, sound generator
    else if (address == 0xf2) {
        // TODO: not implemented
    }
    // QDC, quick disk controller
    else if (IN_RANGE(address, 0xf4, 0xf7)) {
        // TODO: not implemented
    }
    // PIO Z80 PIO, parallel I/O unit
    else if (IN_RANGE(address, 0xfc, 0xff)) {
        // TODO: not implemented
    }
    // DEBUG
    else {
        CHIPS_ASSERT(1);
    }
    
    return pins;
}

/**
 Decode MZ-800 VRAM into RGBA8 buffer
 */
void mz800_decode_vram(void) {
    // Size of a scan line in bytes
    // Bit 2 of DMD: 1 ... 640 pixels, 0 ... 320 pixels
    // TODO: Check for MZ-700 mode
    int line_size = (mz800.gdg.dmd & 0x04) ? 80 : 40; // size on scan line in bytes
}
