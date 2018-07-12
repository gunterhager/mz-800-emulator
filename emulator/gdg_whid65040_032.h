#pragma once
//
//  gdg_whid65040_032.h
//
//  Header-only emulator of the GDG WHID 65040-032, a custom chip
//  found in the SHARP MZ-800 computer. It is used mainly as CRT controller.
//  The GDG acts as memory controller, too. We don't emulate that here.
//
//  Created by Gunter Hager on 03.07.18.
//

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
    
    /// GDG WHID 65040-032 state
    typedef struct {
        /// Write format register
        uint8_t wf;
        /// Read format register
        uint8_t rf;
        
        /// Display mode register
        uint8_t dmd;
        /// Display status register
        uint8_t status;
        
        /// Scroll offset register 1
        uint8_t sof1;
        /// Scroll offset register 2
        uint8_t sof2;
        /// Scroll width register
        uint8_t sw;
        /// Scroll start address register
        uint8_t ssa;
        /// Scroll end address register
        uint8_t sea;
        
        /// Border color register
        uint8_t bcol;
        
        /// Superimpose bit
        uint8_t cksw;
        
        /// Palette registers
        uint8_t plt[4];
        /// Palette switch register (for 16 color mode)
        uint8_t plt_sw;
        
        /// VRAM
        union {
            /// 4 planes for 320x200 (I, II, III, IV)
            uint8_t low_res[4][8000];
            /// 2 planes for 640x200 (I, III)
            uint8_t high_res[2][16000];
        } vram;
        
    } gdg_whid65040_032_t;
    
    /*
     GDG WHID 65040-032 pins
     
     AD ... CPU address bus (16 bit)
     DT ... CPU data bus (8 bit)
     MREQ
     RD
     WR
     IORQ
     M1
     NPL ... NTSC/PAL selection, low for PAL
     MOD7 ... MZ-700/800 mode selection, low for MZ-700 mode
     MNRT ... Manual reset
     WTGD ... Wait signal to CPU
     
     */
    
    /* control pins shared directly with Z80 CPU */
#define  GDG_M1    (1ULL<<24)       /* machine cycle 1 */
#define  GDG_IORQ  (1ULL<<26)       /* input/output request */
#define  GDG_RD    (1ULL<<27)       /* read */
#define  GDG_WR    (1ULL<<28)       /* write */
#define  GDG_INT   (1ULL<<30)       /* interrupt request */
#define  GDG_RESET (1ULL<<31)       /* put GDG into reset state (same as Z80 reset) */
    
    
    /* extract 8-bit data bus from 64-bit pins */
#define GDG_GET_DATA(p) ((uint8_t)(p>>16))
    /* merge 8-bit data bus value into 64-bit pins */
#define GDG_SET_DATA(p,d) {p=((p&~0xFF0000)|((d&0xFF)<<16));}
    
    /* initialize a new GDG WHID 65040-032 instance */
    extern void gdg_whid65040_032_init(gdg_whid65040_032_t* gdg);
    /* reset an existing GDG WHID 65040-032 instance */
    extern void gdg_whid65040_032_reset(gdg_whid65040_032_t* gdg);
    /* perform an IORQ machine cycle */
    extern uint64_t gdg_whid65040_032_iorq(gdg_whid65040_032_t* gdg, uint64_t pins);
    
    /*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_DEBUG
#ifdef _DEBUG
#define CHIPS_DEBUG
#endif
#endif
#ifndef CHIPS_ASSERT
#include <assert.h>
#define CHIPS_ASSERT(c) assert(c)
#endif
    
    /**
     Call this once to initialize a new GDG WHID 65040-032 instance, this will
     clear the gdg_whid65040_032_t struct and go into a reset state.
     */
    void gdg_whid65040_032_init(gdg_whid65040_032_t* gdg) {
        CHIPS_ASSERT(gdg);
        gdg_whid65040_032_reset(gdg);
    }
    
    /**
     Puts the GDG WHID 65040-032 into the reset state.
     */
    void gdg_whid65040_032_reset(gdg_whid65040_032_t* gdg) {
        CHIPS_ASSERT(gdg);
        memset(gdg, 0, sizeof(*gdg));
    }
    
    /**
     Perform an IORQ machine cycle
     */
    uint64_t gdg_whid65040_032_iorq(gdg_whid65040_032_t* gdg, uint64_t pins) {
        uint64_t outpins = pins;
        if ((pins & (GDG_IORQ | GDG_M1)) != GDG_IORQ) {
            return outpins;
        }
        
        uint16_t address = Z80_GET_ADDR(pins);
        
        // Read
        if (pins & GDG_RD) {
            // Display status register
            if (address == 0x00ce) {
                Z80_SET_DATA(outpins, gdg->status);
            }
            // DEBUG
            else {
                CHIPS_ASSERT(1);
            }
        }
        
        // Write
        else if (pins & GDG_WR) {
            // Write format register
            if (address == 0x00cc) {
                gdg->wf = Z80_GET_DATA(pins);
            }
            // Read format register
            else if (address == 0x00cd) {
                gdg->rf = Z80_GET_DATA(pins) & 0x9f; // Bits 5, 6 can't be set
            }
            // Display mode register
            else if (address == 0xce) {
                gdg->dmd = Z80_GET_DATA(pins) & 0x0f; // Only the lower nibble can be set
            }
            // Scroll offset register 1
            else if (address == 0x01cf) {
                gdg->sof1 = Z80_GET_DATA(pins);
            }
            // Scroll offset register 2
            else if (address == 0x02cf) {
                gdg->sof2 = Z80_GET_DATA(pins) & 0x03; // Only bits 0, 1 can be set
            }
            // Scroll width register
            else if (address == 0x03cf) {
                gdg->sw = Z80_GET_DATA(pins) & 0x7f; // Bit 7 can't be set
            }
            // Scroll start address register
            else if (address == 0x04cf) {
                gdg->ssa = Z80_GET_DATA(pins) & 0x7f; // Bit 7 can't be set
            }
            // Scroll end address register
            else if (address == 0x05cf) {
                gdg->sea = Z80_GET_DATA(pins) & 0x7f; // Bit 7 can't be set
            }
            // Border color register
            else if (address == 0x06cf) {
                gdg->bcol = Z80_GET_DATA(pins) & 0x0f; // Only the lower nibble can be set
            }
            // Superimpose bit
            else if (address == 0x07cf) {
                gdg->cksw = Z80_GET_DATA(pins) & 0x80; // Only bit 7 can be set
            }
            // Palette register
            else if (address == 0x00f0) {
                uint8_t value = Z80_GET_DATA(pins) & 0x7f; // Bit 7 can't be set
                
                // Set plt_sw register
                if (value & 0x80) { // Bit 7 set, so we set plt_sw register
                    gdg->plt_sw = value & 0x03; // low 2 bits contain sw
                }
                // Set plt registers
                else {
                    uint8_t index = value >> 4; // high 3 bits contain palette register index
                    uint8_t color = value & 0x0f; // lower nibble contains color code in IGRB
                    gdg->plt[index] = color;
                }
            }
            // DEBUG
            else {
                CHIPS_ASSERT(1);
            }
        }
        
        return outpins;
    }
    
    /**
     Read a byte from VRAM. The meaning of the bits in the byte depend on
     the read format register of the GDG.

     @param gdg Pointer to GDG instance.
     @param addr Address in the VRAM to read from. VRAM addresses start from 0x0000 here.
     @return Returns a byte of information about the VRAM contents.
     */
    uint8_t gdg_whid65040_032_mem_rd(gdg_whid65040_032_t* gdg, uint16_t addr) {
        return 0;
    }
    
    /**
     Write a byte to VRAM. What gets actually written depends on the
     write format register of the GDG.

     @param gdg Pointer to GDG instance.
     @param addr Address in the VRAM to write to. VRAM addresses start from 0x0000 here.
     @param data Byte to write to VRAM.
     */
    void gdg_whid65040_032_mem_wr(gdg_whid65040_032_t* gdg, uint16_t addr, uint8_t data) {
        uint8_t write_mode = gdg->dmd >> 5;
        
        // MZ-700 mode
        if ((gdg->dmd & 0x0f) == 0x08) {
            // TODO: not implemented yet
        }
        // 640x200 mode
        else if (gdg->dmd & 0x04) {
            
        }
        // 320x200 mode
        else {
            
        }
    }
    
#endif /* CHIPS_IMPL */
    
#ifdef __cplusplus
} /* extern "C" */
#endif
