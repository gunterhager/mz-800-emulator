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

#define NOT_IMPLEMENTED false

#define VRAMSIZE (640 * 200)
/// GDG WHID 65040-032 state
typedef struct {
	/// Write format register
	/// Determines how pixel data is written to VRAM.
	uint8_t wf;

	/// Read format register
	/// Determines how pixel data is read from VRAM:
	uint8_t rf;

	/// Display mode register
	/// Determines how the data in the VRAM is interpreted when decoding to RGB8 buffer.
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
	/// In MZ-700 mode: Starting at 0x0000 each byte corresponds to a character (40x25).
	/// Starting at 0x0800 each byte corresponds to a color code that controls the color
	/// for each character and its background. Bit 7 controls if the alternative character set
	/// should be used for that character.
	/// In MZ-800 Mode: one byte for each pixel, we need only 4 bit per pixel.
	/// Each bit corresponds to a pixel on planes I, II, III, IV.
	uint8_t vram[VRAMSIZE];

	/// CGROM contains bitmapped character shapes.
	uint8_t *cgrom;

	/// RGBA8 buffer for displaying color graphics on screen.
	uint32_t *rgba8_buffer;

	// Private status properties

	/// Mask that indicates which planes to write to.
	uint8_t write_planes;
	/// Mask that indicates which planes to reset.
	uint8_t reset_planes;

	/// Mask that indicates which planes to read from.
	uint8_t read_planes;

	/// Indicates if machine is in MZ-700 mode. This is actually toggled by setting the DMD register.
	bool is_mz700;

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

void gdg_whid65040_032_init(gdg_whid65040_032_t* gdg, uint8_t *cgrom, uint32_t *rgba8_buffer);
void gdg_whid65040_032_reset(gdg_whid65040_032_t* gdg);
uint64_t gdg_whid65040_032_iorq(gdg_whid65040_032_t* gdg, uint64_t pins);
uint64_t gdg_whid65040_032_tick(gdg_whid65040_032_t *gdg, uint64_t pins);

uint8_t gdg_whid65040_032_mem_rd(gdg_whid65040_032_t* gdg, uint16_t addr);
void gdg_whid65040_032_mem_wr(gdg_whid65040_032_t* gdg, uint16_t addr, uint8_t data);
void gdg_whid65040_032_set_rf(gdg_whid65040_032_t* gdg, uint8_t value);
void gdg_whid65040_032_set_wf(gdg_whid65040_032_t* gdg, uint8_t value);

void gdg_whid65040_032_set_dmd(gdg_whid65040_032_t* gdg, uint8_t value);

void gdg_whid65040_032_decode_vram(gdg_whid65040_032_t* gdg, uint16_t addr);
void gdg_whid65040_032_decode_vram_mz800(gdg_whid65040_032_t* gdg, uint16_t addr);
void gdg_whid65040_032_decode_vram_mz700(gdg_whid65040_032_t* gdg, uint16_t addr);

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

// Color definition helpers
#define CI0 (0x78)
#define CI1 (0xdf)
#define CI(i) ((i) ? CI1 : CI0)
#define COLOR_IGRB_TO_ABGR(i, g, r, b) (0xff000000 | (((b) * CI(i)) << 16) | (((g) * CI(i)) << 8) | ((r) * CI(i)))

/// Colors - the MZ-800 has 16 fixed colors.
/// Color codes on the MZ-800 are IGRB (Intensity, Green, Red, Blue).
/// NOTE: the colors are encoded in ABGR.
const uint32_t mz800_colors[16] = {
	// Intensity low
	COLOR_IGRB_TO_ABGR(0, 0, 0, 0), // 0000 black
	COLOR_IGRB_TO_ABGR(0, 0, 0, 1), // 0001 blue
	COLOR_IGRB_TO_ABGR(0, 0, 1, 0), // 0010 red
	COLOR_IGRB_TO_ABGR(0, 0, 1, 1), // 0011 purple
	COLOR_IGRB_TO_ABGR(0, 1, 0, 0), // 0100 green
	COLOR_IGRB_TO_ABGR(0, 1, 0, 1), // 0101 cyan
	COLOR_IGRB_TO_ABGR(0, 1, 1, 0), // 0110 yellow
	COLOR_IGRB_TO_ABGR(0, 1, 1, 1), // 0111 white
	// Intensity high
	COLOR_IGRB_TO_ABGR(1, 0, 0, 0), // 1000 gray
	COLOR_IGRB_TO_ABGR(1, 0, 0, 1), // 1001 light blue
	COLOR_IGRB_TO_ABGR(1, 0, 1, 0), // 1010 light red
	COLOR_IGRB_TO_ABGR(1, 0, 1, 1), // 1011 light purple
	COLOR_IGRB_TO_ABGR(1, 1, 0, 0), // 1100 light green
	COLOR_IGRB_TO_ABGR(1, 1, 0, 1), // 1101 light cyan
	COLOR_IGRB_TO_ABGR(1, 1, 1, 0), // 1110 light yellow
	COLOR_IGRB_TO_ABGR(1, 1, 1, 1)  // 1111 light white
};

#pragma mark - Life cycle

/**
 Call this once to initialize a new GDG WHID 65040-032 instance, this will
 clear the gdg_whid65040_032_t struct and go into a reset state.

 @param gdg Pointer to GDG instance.
 @param cgrom Pointer to character ROM.
 @param rgba8_buffer RBGA8 buffer to display color graphics.
 */
void gdg_whid65040_032_init(gdg_whid65040_032_t* gdg, uint8_t *cgrom, uint32_t *rgba8_buffer) {
	CHIPS_ASSERT(gdg);
	gdg_whid65040_032_reset(gdg);
	gdg->cgrom = cgrom;
	gdg->rgba8_buffer = rgba8_buffer;
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
	uint16_t low_address = address & 0x00ff;

	// Read
	if (pins & GDG_RD) {
		// Display status register
		if (low_address == 0x00ce) {
			Z80_SET_DATA(outpins, gdg->status);
		}
		// DEBUG
		else {
			CHIPS_ASSERT(NOT_IMPLEMENTED);
		}
	}

	// Write
	else if (pins & GDG_WR) {
		// Write format register
		if (low_address == 0x00cc) {
			uint8_t value = Z80_GET_DATA(pins);
			gdg_whid65040_032_set_wf(gdg, value);
		}
		// Read format register
		else if (low_address == 0x00cd) {
			gdg->rf = Z80_GET_DATA(pins) & 0x9f; // Bits 5, 6 can't be set
		}
		// Display mode register
		else if (low_address == 0x00ce) {
			uint8_t value = Z80_GET_DATA(pins) & 0x0f; // Only the lower nibble can be set
			gdg_whid65040_032_set_dmd(gdg, value);
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
		else if (low_address == 0x00f0) {
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
			CHIPS_ASSERT(NOT_IMPLEMENTED);
		}
	}

	return outpins;
}

uint64_t gdg_whid65040_032_tick(gdg_whid65040_032_t *gdg, uint64_t pins) {
#warning "TODO: Implement tick function"
	return pins;
}

#pragma mark - VRAM

/**
 Read a byte from VRAM. The meaning of the bits in the byte depend on
 the read format register of the GDG.

 @param gdg Pointer to GDG instance.
 @param addr Address in the VRAM to read from. VRAM addresses start from 0x0000 here.
 @return Returns a byte of information about the VRAM contents.
 */
uint8_t gdg_whid65040_032_mem_rd(gdg_whid65040_032_t* gdg, uint16_t addr) {

	if (gdg->is_mz700 && (gdg->rf == 0x01)) {
		return gdg->vram[addr];
	} else {
		return 0;
#warning "TODO: mem_rd: Implement"

		uint8_t plane_select = gdg->rf & 0x0f;
		bool is_searching = gdg->rf & (1 << 7);
		if (is_searching) {

		} else {

		}
	}
}

/**
 Write a byte to VRAM. What gets actually written depends on the
 write format register of the GDG.
 Pixel data will also be written to the RGBA8 buffer.

 @param gdg Pointer to GDG instance.
 @param addr Address in the VRAM to write to. VRAM addresses start from 0x0000 here.
 @param data Byte to write to VRAM. Each bit corresponds to a single pixel.
 */
void gdg_whid65040_032_mem_wr(gdg_whid65040_032_t* gdg, uint16_t addr, uint8_t data) {

	if (gdg->is_mz700 && (gdg->wf == 0x01)) {
		gdg->vram[addr] = data;
	} else {
		uint8_t write_mode = gdg->wf >> 5;
		uint8_t *plane_ptr = gdg->vram + addr * 8;

		// Write into VRAM
		for (uint8_t bit = 0; bit < 8; bit++, plane_ptr++, data >>= 1) {
			switch (write_mode) {
				case 0: // 000 Single write
					if (data & 0x01) {
						*plane_ptr |= gdg->write_planes;
					} else {
						*plane_ptr &= ~gdg->reset_planes;
					}
					break;

				case 1: // 001 XOR
					if (data & 0x01) {
						*plane_ptr ^= gdg->write_planes;
					}
					break;

				case 2: // 010 OR
					if (data & 0x01) {
						*plane_ptr |= gdg->write_planes;
					}
					break;

				case 3: // 011 Reset
					if (data & 0x01) {
						*plane_ptr &= ~gdg->reset_planes;
					}
					break;

				case 4: // 10x Replace
				case 5:
					if (data & 0x01) {
						*plane_ptr = gdg->write_planes;
					} else {
						*plane_ptr = 0;
					}
					break;

				case 6: // 11x PSET
				case 7:
					if (data & 0x01) {
						*plane_ptr = gdg->write_planes;
					}
					break;
			}
		}
	}

	// Decode VRAM into RGB8 buffer
	gdg_whid65040_032_decode_vram(gdg, addr);
}

/**
 Sets the write format register.

 @param gdg Pointer to GDG instance.
 @param value Value to write into the register.
 */
void gdg_whid65040_032_set_wf(gdg_whid65040_032_t* gdg, uint8_t value) {
	gdg->wf = value;

	// MZ-700 mode
	if (value == 0x01) {
		return;
	}

	if (gdg->dmd & 0x02) { // 640x200 4 colors, 320x200 16 colors
		// Frame B is irrelevant here, no frame switching allowed
		if (gdg->dmd & 0x04) { // 640x200 mode
			// Planes I, III can be selected
			gdg->write_planes = value & 0x05;
			gdg->reset_planes = ~0x05; // Planes II, IV will be cleared
		} else { // 320x200 mode
			// Planes I, II, III, IV can be selected
			gdg->write_planes = value & 0x0f;
			gdg->reset_planes = ~0x0f; // I don't think that's really necessary since these bits will always be 0
		}
	} else { // 640x200 1 color, 320x200 4 colors
		bool use_frame_b = value & 0x10;
		if (value & 0x80) { // Replace or PSET
			if (gdg->dmd & 0x04) { // 640x200 mode
				if (use_frame_b) {
					gdg->write_planes &= 0x04; // Plane III
					gdg->reset_planes = ~0x04;
				} else {
					gdg->write_planes &= 0x01; // Plane I
					gdg->reset_planes = ~0x01;                    }
			} else { // 320x200 mode
				if (use_frame_b) {
					gdg->write_planes &= 0x0c; // Planes III, IV
					gdg->reset_planes = ~0x0c;
				} else {
					gdg->write_planes &= 0x03; // Planes I, II
					gdg->reset_planes = ~0x03;
				}
			}
		} else { // Single Write, XOR, OR, Reset
			gdg->write_planes &= 0x03;
			gdg->reset_planes = ~0x03;
		}
	}

}

/**
 Sets the read format register.

 @param gdg Pointer to GDG instance.
 @param value Value to write into the register.
 */
void gdg_whid65040_032_set_rf(gdg_whid65040_032_t* gdg, uint8_t value) {
	gdg->rf = value;
	bool use_frame_b = value & 0x10;

	// Set read planes mask here
#warning "TODO: set_rf: Implement setting read planes mask"
}


#pragma mark - Display mode

/**
 Sets the display mode register.

 @param gdg Pointer to GDG instance.
 @param value Value to write into the register.
 */
void gdg_whid65040_032_set_dmd(gdg_whid65040_032_t* gdg, uint8_t value) {
	uint8_t old_dmd = gdg->dmd;
	gdg->dmd = value;

	// MZ-700 mode
	gdg->is_mz700 = (value & 0x0f) == 0x08;

	// Trigger decoding of VRAM if mode changed
	if (old_dmd != value) {
		//            for (uint16_t addr = 0x0000; addr < VRAMSIZE; addr++) {
		//                gdg_whid65040_032_decode_vram(gdg, addr);
		//            }
	}
}

#pragma mark - Decoding VRAM

/**
 Decode one byte of VRAM into the RGBA8 buffer.

 @param gdg Pointer to GDG instance.
 @param addr Address in the VRAM to decode from. VRAM addresses start from 0x0000 here.
 */
void gdg_whid65040_032_decode_vram(gdg_whid65040_032_t* gdg, uint16_t addr) {
	if (gdg->is_mz700) {
		gdg_whid65040_032_decode_vram_mz700(gdg, addr);
	} else {
		gdg_whid65040_032_decode_vram_mz800(gdg, addr);
	}
}

/**
 Decode one byte of VRAM into the RGBA8 buffer in MZ-700 mode.

 @param gdg Pointer to GDG instance.
 @param addr Address in the VRAM to decode from. VRAM addresses start from 0x0000 here.
 */
void gdg_whid65040_032_decode_vram_mz700(gdg_whid65040_032_t* gdg, uint16_t addr) {
	// Convert addr to address offsets in character VRAM and color VRAM
	// Character range: 0x0000 - 0x03f7
	uint16_t character_code_addr = (addr >= 0x0800) ? addr - 0x0800 : addr;
	// Color range: 0x0800 - 0x0bf7
	uint16_t color_addr = (addr >= 0x0800) ? addr : addr + 0x0800;

	// Convert color code to foreground and background colors
	uint8_t color_code = gdg->vram[color_addr];
	uint8_t fg_color_code = (color_code & 0x70) >> 4;
	fg_color_code = (fg_color_code == 0) ? 0 : fg_color_code | 0x8; // All colors except black should be high intensity
	uint32_t fg_color = mz800_colors[fg_color_code];
	uint8_t bg_color_code = color_code & 0x07;
	bg_color_code = (bg_color_code == 0) ? 0 : bg_color_code | 0x8;  // All colors except black should be high intensity
	uint32_t bg_color = mz800_colors[bg_color_code];

	// Use bit 7 of color code to select start address in character rom
	bool use_alternate_characters = color_code & 0x80;

	// Convert character code to address offset in character rom
	uint8_t character_code = gdg->vram[character_code_addr];
	uint16_t character_addr = character_code * 8; // Each character consists of 8 byte
	if (use_alternate_characters) {
		character_addr += 256 * 8; // A full character set contains 256 characters
	}

	// Calculate character coordinates
	uint32_t column = character_code_addr % 40; // 40 characters on a line
	uint32_t row = character_code_addr / 40; // 25 lines
	uint32_t character_width = 8 * 2; // Width of character in hires pixel
	uint32_t line_width = 40 * character_width; // Width of line in hires pixel
	uint32_t character_height = 8; // Height of character in pixel
	uint32_t character_pixel_addr = column * character_width + row * line_width * character_height;

	// Character data lookup
	for (uint8_t char_byte_index = 0; char_byte_index < 8; char_byte_index++) { // Iterate over character bytes
		uint8_t char_byte = gdg->cgrom[character_addr + char_byte_index];
		uint32_t index = character_pixel_addr; // Pixel index in rgba8_buffer
		uint32_t offset = char_byte_index * line_width; // offset for index to get correct raster line
		for (uint8_t bit = 0; bit < 8; bit++, index++) { // Iterate over bits in character byte
			// Get color for pixel
			uint8_t value = (char_byte >> bit) & 0x01;
			uint32_t color = value ? fg_color : bg_color;

			// Write character data to RGBA8 buffer (in 320x200 resolution, 2 bytes per pixel)
			gdg->rgba8_buffer[index + offset] = color;
			index++;
			gdg->rgba8_buffer[index + offset] = color;
		}
	}


}

/**
 Decode one byte of VRAM into the RGBA8 buffer in MZ-800 mode.

 @param gdg Pointer to GDG instance.
 @param addr Address in the VRAM to decode from. VRAM addresses start from 0x0000 here.
 */
void gdg_whid65040_032_decode_vram_mz800(gdg_whid65040_032_t* gdg, uint16_t addr) {
	// Setup
	uint8_t *plane_ptr = gdg->vram + addr * 8;
	bool hires = gdg->dmd & 0x04; // 640x200 if set
	uint32_t index = addr * 8 * (hires ? 1 : 2); // Pixel index in rgba8_buffer, in lores we write 2 pixels for each lores pixel

	// Iterate over 8 bits of a single VRAM byte
	// We need to set one byte of RGBA8 buffer for each VRAM bit (2 bytes in 320x200 mode)
	for (uint8_t bit = 0; bit < 8; bit++, plane_ptr++, index++) {
		// Get value from VRAM
		uint8_t value = *plane_ptr;

		// Look up in palette
		uint8_t mz_color;
		if (gdg->dmd == 0x02) { // Special lookup for 320x200, 16 colors
			if (((value >> 2) & 0x03) == gdg->plt_sw) { // If plane III and IV match palette switch
				mz_color = gdg->plt[value]; // Take color from palette
			} else {
				mz_color = value; // Take color directly from plane data
			}
		} else { // All other modes take color from palette
			bool use_frame_b = gdg->dmd & 0x01;
			if (hires) {
				if (gdg->dmd & 0x02) { // Special lookup for 640x200, 4 colors
					int8_t palette = (value | (value >> 1)) & 0x03; // Combine planes I, III
					mz_color = gdg->plt[palette];
				} else {
					int8_t palette = (use_frame_b ? (value >> 2) : value) & 0x01;
					mz_color = gdg->plt[palette];
				}
			} else {
				int8_t palette = (use_frame_b ? (value >> 2) : value) & 0x03;
				mz_color = gdg->plt[palette];
			}
		}

		// Look up final color
		uint32_t color = mz800_colors[mz_color];

		// Write to RGBA8 buffer
		if (!hires) {
			// We need to write 2 pixels for each lores pixel
			gdg->rgba8_buffer[index++] = color;
		}
		gdg->rgba8_buffer[index] = color;
	}

}

#endif /* CHIPS_IMPL */

#ifdef __cplusplus
} /* extern "C" */
#endif
