//------------------------------------------------------------------------------
//  gdg_whid65040_032-test.c
//------------------------------------------------------------------------------
#define CHIPS_IMPL
#include "../emulator/mz800/gdg_whid65040_032.h"
#include "../emulator/roms/mz800-roms.h"
#include "utest.h"

#define T(b) ASSERT_TRUE(b)

// Display size
#define MZ800_DISPLAY_WIDTH (640)
#define MZ800_DISPLAY_HEIGHT (200)
#define MZ800_FRAMEBUFFER_SIZE_PIXEL (MZ800_DISPLAY_WIDTH * MZ800_DISPLAY_HEIGHT)
#define MZ800_FRAMEBUFFER_BYTES_PER_PIXEL (4)
#define MZ800_FRAMEBUFFER_SIZE_BYTES (MZ800_FRAMEBUFFER_SIZE_PIXEL * MZ800_FRAMEBUFFER_BYTES_PER_PIXEL)

// Frame buffer
static uint32_t framebuffer[MZ800_FRAMEBUFFER_SIZE_PIXEL];

gdg_whid65040_032_desc_t gdg_whid65040_032_desc() {
    return (gdg_whid65040_032_desc_t) {
        .ntpl = 0,
        .cgrom = dump_mz800_cgrom_bin,
        .rgba8_buffer = framebuffer,
        .rgba8_buffer_size = sizeof(framebuffer),
    };
}

UTEST(gdg_whid65040_032, init) {
    gdg_whid65040_032_t sys;
    gdg_whid65040_032_desc_t desc = gdg_whid65040_032_desc();
    gdg_whid65040_032_init(&sys, &desc);
    T(sys.ntpl == desc.ntpl);
    T(sys.cgrom == desc.cgrom);
    T(sys.rgba8_buffer == desc.rgba8_buffer);
    T(sys.rgba8_buffer_size == desc.rgba8_buffer_size);
}
