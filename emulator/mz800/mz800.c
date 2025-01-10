//------------------------------------------------------------------------------
// mz800.c 
//
// Emulator for the SHARP MZ-800
//
//------------------------------------------------------------------------------

#define CHIPS_IMPL
#include "chips/chips_common.h"
#include "common.h"
#include "chips/z80.h"
#include "chips/z80pio.h"
#include "chips/i8255.h"
#include "chips/clk.h"
#include "chips/kbd.h"
#include "chips/mem.h"

#include "mzf.h"
#include "gdg_whid65040_032.h"
#include "mz800.h"
#include "../roms/mz800-roms.h"

#if defined(CHIPS_USE_UI)
	#define UI_DBG_USE_Z80
	#include "ui.h"
	#include "ui/ui_chip.h"
	#include "ui/ui_memedit.h"
	#include "ui/ui_memmap.h"
	#include "ui/ui_dasm.h"
	#include "ui/ui_dbg.h"
	#include "ui/ui_z80.h"
	#include "ui/ui_z80pio.h"
	#include "ui/ui_i8255.h"
	#include "ui_gdg_whid65040_032.h"
	#include "ui/ui_audio.h"
	#include "ui/ui_kbd.h"
	#include "ui_mz800.h"
#endif

static struct {
	mz800_t mz800;
	uint32_t frame_time_us;
	uint32_t ticks;
	double emu_time_ms;
#if defined(CHIPS_USE_UI)
	ui_mz800_t ui_mz800;
#endif
} state;

#ifdef CHIPS_USE_UI
#define BORDER_TOP (24)
#else
#define BORDER_TOP (8)
#endif
#define BORDER_LEFT (8)
#define BORDER_RIGHT (8)
#define BORDER_BOTTOM (32)
#define LOAD_DELAY_FRAMES (180)

// MARK: - Function declarations

/* sokol-app entry, configure application callbacks and window */
void app_init(void);
void app_frame(void);
void app_input(const sapp_event*);
void app_cleanup(void);


// MARK: - Audio


/* audio-streaming callback */
static void push_audio(const float* samples, int num_samples, void* user_data) {
	(void)user_data;
    saudio_push(samples, num_samples);
}

// MARK: - App

mz800_desc_t mz800_desc() {
	return (mz800_desc_t) {
        .ntpl = GDG_PAL,
		.audio = {
			.callback = { .func = push_audio },
			.sample_rate = saudio_sample_rate(),
		},
		.roms = {
			.rom1 = { .ptr = dump_mz800_rom1_bin, .size = sizeof(dump_mz800_rom1_bin) },
			.cgrom = { .ptr = dump_mz800_cgrom_bin, .size = sizeof(dump_mz800_cgrom_bin) },
			.rom2 = { .ptr = dump_mz800_rom2_bin, .size = sizeof(dump_mz800_rom2_bin) },
		},
		#if defined(CHIPS_USE_UI)
		.debug = ui_mz800_get_debug(&state.ui_mz800),
		#endif
	};
}

#if defined(CHIPS_USE_UI)
static void ui_draw_cb(const ui_draw_info_t* draw_info) {
    ui_mz800_draw(&state.ui_mz800, &(ui_mz800_frame_t){
        .display = draw_info->display
    });
}

static void ui_save_settings_cb(ui_settings_t* settings) {
    // TODO: Implement save settings
//    ui_mz800_save_settings(&state.ui_mz800, settings);
}
#endif

/* one-time application init */
void app_init() {
    saudio_setup(&(saudio_desc){0});
    mz800_desc_t desc = mz800_desc();
    mz800_init(&state.mz800, &desc);

    gfx_init(&(gfx_desc_t){
        .disable_speaker_icon = sargs_exists("disable-speaker-icon"),
        #ifdef CHIPS_USE_UI
        .draw_extra_cb = ui_draw,
        #endif
        .border = {
            .left = BORDER_LEFT,
            .right = BORDER_RIGHT,
            .top = BORDER_TOP,
            .bottom = BORDER_BOTTOM,
        },
        .pixel_aspect = {
            .width = 1,
            .height = 2,
        },
        .display_info = mz800_display_info(&state.mz800)
    });

	keybuf_init(&(keybuf_desc_t) { .key_delay_frames = 5 });
    clock_init();
	prof_init();
    fs_init();

#ifdef CHIPS_USE_UI
    ui_init(&(ui_desc_t){
        .draw_cb = ui_draw_cb,
        .save_settings_cb = ui_save_settings_cb,
        .imgui_ini_key = "gupi.mz800",
    });
    ui_mz800_init(&state.ui_mz800, &(ui_mz800_desc_t){
		.mz800 = &state.mz800,
        .dbg_texture = {
            .create_cb = ui_create_texture,
            .update_cb = ui_update_texture,
            .destroy_cb = ui_destroy_texture,
        },
		.dbg_keys = {
			.cont = { .keycode = SAPP_KEYCODE_F5, .name = "F5" },
			.stop = { .keycode = SAPP_KEYCODE_F5, .name = "F5" },
			.step_over = { .keycode = SAPP_KEYCODE_F6, .name = "F6" },
			.step_into = { .keycode = SAPP_KEYCODE_F7, .name = "F7" },
			.step_tick = { .keycode = SAPP_KEYCODE_F8, .name = "F8" },
			.toggle_breakpoint = { .keycode = SAPP_KEYCODE_F9, .name = "F9" }
		}
	});
#endif

	bool delay_input = false;
	if (sargs_exists("file")) {
		delay_input = true;
        fs_load_file_async(FS_CHANNEL_IMAGES, sargs_value("file"));
	}
	if (!delay_input) {
		if (sargs_exists("input")) {
			keybuf_put(sargs_value("input"));
		}
	}
 }

static void handle_file_loading(void);
//static void send_keybuf_input(void);
static void draw_status_bar(void);

/* per frame stuff, tick the emulator, handle input, decode and draw emulator display */
void app_frame() {
	state.frame_time_us = clock_frame_time();
	const uint64_t emu_start_time = stm_now();
	state.ticks = mz800_exec(&state.mz800, state.frame_time_us);
	state.emu_time_ms = stm_ms(stm_since(emu_start_time));
	draw_status_bar();
    gfx_draw(mz800_display_info(&state.mz800));

    /* load MZF file? */
	handle_file_loading();

#warning TODO: implement keyboard input
//	send_keybuf_input();
}

/* keyboard input handling */
void app_input(const sapp_event* event) {
	// accept dropped files also when ImGui grabs input
	if (event->type == SAPP_EVENTTYPE_FILES_DROPPED) {
        fs_load_dropped_file_async(FS_CHANNEL_IMAGES);
	}
#ifdef CHIPS_USE_UI
	if (ui_input(event)) {
		// input was handled by UI
		return;
	}
#endif
	// TODO: Keyboard input handling
}

/* application cleanup callback */
void app_cleanup() {
	mz800_discard(&state.mz800);
#ifdef CHIPS_USE_UI
	ui_mz800_discard(&state.ui_mz800);
	ui_discard();
#endif
    saudio_shutdown();
    gfx_shutdown();
    sargs_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
	sargs_setup(&(sargs_desc){ .argc = argc, .argv = argv });
	return (sapp_desc) {
		.init_cb = app_init,
		.frame_cb = app_frame,
		.event_cb = app_input,
		.cleanup_cb = app_cleanup,
		.width = MZ800_DISPLAY_WIDTH + BORDER_LEFT + BORDER_RIGHT,
		.height = 2 * MZ800_DISPLAY_HEIGHT + BORDER_TOP + BORDER_BOTTOM,
		.window_title = "MZ-800",
		.icon.sokol_default = true,
		.enable_dragndrop = true
	};
}

// MARK: - Display

chips_display_info_t mz800_display_info(mz800_t* sys) {
    const chips_display_info_t res = {
        .frame = {
            .dim = {
                .width = MZ800_DISPLAY_WIDTH,
                .height = MZ800_DISPLAY_HEIGHT,
            },
            .buffer = {
                .ptr = sys ? sys->fb : 0,
                .size = MZ800_FRAMEBUFFER_SIZE_BYTES,
            },
            .bytes_per_pixel = 4,
        },
        .screen = {
            .x = 0,
            .y = 0,
            .width = MZ800_DISPLAY_WIDTH,
            .height = MZ800_DISPLAY_HEIGHT,
        }
    };
    CHIPS_ASSERT(((sys == 0) && (res.frame.buffer.ptr == 0)) || ((sys != 0) && (res.frame.buffer.ptr != 0)));
    return res;
}


// MARK: - File loading
static void handle_file_loading(void) {
	fs_dowork();
    const uint32_t load_delay_frames = LOAD_DELAY_FRAMES;
    if (fs_success(FS_CHANNEL_IMAGES) && (clock_frame_count_60hz() > load_delay_frames)) {
        chips_range_t data = fs_data(FS_CHANNEL_IMAGES);
		bool load_success = mzf_load(data.ptr, data.size, &state.mz800, state.mz800.dram);
		if (load_success) {
			if (clock_frame_count_60hz() > (load_delay_frames + 10)) {
				gfx_flash_success();
			}
			if (sargs_exists("input")) {
				keybuf_put(sargs_value("input"));
			}
		}
		else {
			gfx_flash_error();
		}
        fs_reset(FS_CHANNEL_IMAGES);
	}
}

// MARK: - Status bar

static void draw_status_bar(void) {
	prof_push(PROF_EMU, (float)state.emu_time_ms);
	prof_stats_t emu_stats = prof_stats(PROF_EMU);

	const uint32_t text_color = 0xFFFFFFFF;

	const float w = sapp_widthf();
	const float h = sapp_heightf();
	sdtx_canvas(w, h);
	sdtx_origin(1.0f, (h / 8.0f) - 3.5f);
	sdtx_font(0);

	gdg_whid65040_032_t* gdg = &state.mz800.gdg;
	sdtx_printf("MZ Mode: %s", (gdg->is_mz700) ? "MZ-700":"MZ-800");

	sdtx_font(0);
	sdtx_color1i(text_color);
	sdtx_pos(0.0f, 1.5f);
	sdtx_printf("frame:%.2fms emu:%.2fms (min:%.2fms max:%.2fms) ticks:%d", (float)state.frame_time_us * 0.001f, emu_stats.avg_val, emu_stats.min_val, emu_stats.max_val, state.ticks);
}

