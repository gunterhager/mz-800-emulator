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
        .ntpl = false,
        .is_VRAM_extension_installed = false,
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
    T(!sys.is_mz700);
    T(sys.status & 0x02);
}

UTEST(gdg_whid65040_032, set_MZ800_mode) {
    gdg_whid65040_032_t sys;
    gdg_whid65040_032_desc_t desc = gdg_whid65040_032_desc();
    gdg_whid65040_032_init(&sys, &desc);
    
    // Set MZ-800 mode
    gdg_whid65040_032_set_dmd(&sys, 0x00);
    T(sys.status & 0x02);
}

UTEST(gdg_whid65040_032, set_MZ700_mode) {
    gdg_whid65040_032_t sys;
    gdg_whid65040_032_desc_t desc = gdg_whid65040_032_desc();
    gdg_whid65040_032_init(&sys, &desc);
    
    // Set MZ-800 mode
    gdg_whid65040_032_set_dmd(&sys, 0x00);
    T(sys.status & 0x02);
    // Set MZ-700 mode
    gdg_whid65040_032_set_dmd(&sys, GDG_DMD_MZ700);
    T(!(sys.status & 0x02));
}

UTEST(gdg_whid65040_032, MEM_WR_SINGLE_WRITE) {
    gdg_whid65040_032_t sys;
    gdg_whid65040_032_desc_t desc = gdg_whid65040_032_desc();
    gdg_whid65040_032_init(&sys, &desc);
    
    // Set MZ-800 mode, 320x200, 16 colors
    gdg_whid65040_032_set_dmd(&sys, GDG_DMD_HICOLOR);
    
    // Pixels: black / black / yellow / yellow / magenta / magenta / cyan /cyan
    uint8_t plane_data[] = {0x0f, 0x33, 0x3C, 0x00};
    for (uint8_t bit = 0; bit < 4; bit++) {
        gdg_whid65040_032_set_wf(&sys, (1 << bit));
        gdg_whid65040_032_mem_wr(&sys, 0x0000, plane_data[bit]);
    }
    T(sys.vram1[0] == plane_data[0]);                       // plane I
    T(sys.vram1[GDG_VRAM_PLANE_OFFSET] == plane_data[1]);   // plane II
    T(sys.vram2[0] == plane_data[2]);                       // plane III
    T(sys.vram2[GDG_VRAM_PLANE_OFFSET] == plane_data[3]);   // plane IV
}

UTEST(gdg_whid65040_032, MEM_WR_REPLACE) {
    gdg_whid65040_032_t sys;
    gdg_whid65040_032_desc_t desc = gdg_whid65040_032_desc();
    gdg_whid65040_032_init(&sys, &desc);
    
    // Set MZ-800 mode, 320x200, 16 colors
    gdg_whid65040_032_set_dmd(&sys, GDG_DMD_HICOLOR);
    
    // Pixels: black / black / yellow / yellow / magenta / magenta / cyan /cyan
    uint8_t plane_data[] = {0x0f, 0x33, 0x3C, 0x00};
    sys.vram1[0] = plane_data[0];                       // plane I
    sys.vram1[GDG_VRAM_PLANE_OFFSET] = plane_data[1];   // plane II
    sys.vram2[0] = plane_data[2];                       // plane III
    sys.vram2[GDG_VRAM_PLANE_OFFSET] = plane_data[3];   // plane IV
    
    // Replace with: black / light yellow / black / light yellow / black / light yellow / black / light yellow
    gdg_whid65040_032_set_wf(&sys, (1 << 7) | (1 << 3) | (1 << 2) | (1 << 1));
    gdg_whid65040_032_mem_wr(&sys, 0x0000, 0x55);
    
    T(sys.vram1[0] == 0x00);                        // plane I
    T(sys.vram1[GDG_VRAM_PLANE_OFFSET] == 0x55);    // plane II
    T(sys.vram2[0] == 0x55);                        // plane III
    T(sys.vram2[GDG_VRAM_PLANE_OFFSET] == 0x55);    // plane IV

}
