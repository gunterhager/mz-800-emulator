#pragma once
//
//  mzf.h
//  mz-800-emulator
//
// Support for files in MZF/MZT format.
//
//  Created by Gunter Hager on 21.07.18.
//

#include <stdint.h>
#include <stdbool.h>
#include "chips/z80.h"
#include "mz800.h"

// File attribute for CMT (cassette tapes)
enum mzf_attribute {
    mzf_obj = 0x01, // machine code program
    mzf_btx = 0x05, // BASIC program
    mzf_txt = 0x94  // text file
};

// MZF file header format
typedef struct {
    uint8_t attribute; // MZF attribute
    uint8_t name[17]; // name in MZ-ASCII, terminated with 0x0d ('\r')
    uint16_t file_length; // length of the binary data without header (little endian)
    uint16_t loading_address; // address where the file should be loaded to (little endian)
    uint16_t start_address; // address to jump at after loading (little endian)
    uint8_t comment[104]; // comment, mostly unused
} mzf_header;

bool mzf_load(const uint8_t *ptr, uint16_t num_bytes, mz800_t *sys, uint8_t *mem) {
    if (ptr == NULL) {
        return false;
    }
    const mzf_header *hdr = (const mzf_header *)ptr;

    if (num_bytes != sizeof(mzf_header) + hdr->file_length) {
        return false;
    }
    // File size can't exceed 64K
    if (hdr->file_length > 0x10000 - hdr->start_address) {
        return false;
    }
    if (hdr->attribute != mzf_obj) { // only obj loading supported currently
        return false;
    }
    
    ptr += sizeof(mzf_header);
    uint8_t *dst = mem + hdr->loading_address;
    
    memcpy(dst, ptr, hdr->file_length);
    z80_reset(&sys->cpu);
	sys->pins = z80_prefetch(&sys->cpu, hdr->start_address);
    return true;
}
