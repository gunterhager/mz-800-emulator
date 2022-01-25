//------------------------------------------------------------------------------
// mz800.c 
//
// Emulator for the SHARP MZ-800
//
//------------------------------------------------------------------------------

#include "common.h"
#define CHIPS_IMPL
#include "chips/z80.h"
#include "chips/z80pio.h"
#include "chips/clk.h"
#include "chips/kbd.h"
#include "chips/mem.h"

#include "mzf.h"
#include "gdg_whid65040_032.h"
#include "mz800.h"

static struct {
	mz800_t mz800;
	uint32_t frame_time_us;
	uint32_t ticks;
	double emu_time_ms;
} state;

#define BORDER_TOP (8)
#define BORDER_LEFT (8)
#define BORDER_RIGHT (8)
#define BORDER_BOTTOM (32)


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

/* one-time application init */
void app_init() {
	gfx_init(&(gfx_desc_t){
		.border_left = BORDER_LEFT,
		.border_right = BORDER_RIGHT,
		.border_top = BORDER_TOP,
		.border_bottom = BORDER_BOTTOM,
		.emu_aspect_y = 2
	});

	keybuf_init(&(keybuf_desc_t) { .key_delay_frames=7 });
    clock_init();
	prof_init();
    saudio_setup(&(saudio_desc){0});
    fs_init();
    
    mz800_init(&state.mz800);
    
	bool delay_input = false;
	if (sargs_exists("file")) {
		delay_input = true;
		fs_start_load_file(sargs_value("file"));
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
	gfx_draw(MZ800_DISP_WIDTH, MZ800_DISP_HEIGHT);

    /* load MZF file? */
	handle_file_loading();

#warning "TODO: implement keyboard input"
//	send_keybuf_input();
}

/* keyboard input handling */
void app_input(const sapp_event* event) {
	// accept dropped files also when ImGui grabs input
	if (event->type == SAPP_EVENTTYPE_FILES_DROPPED) {
		fs_start_load_dropped_file();
	}

    // TODO: Keyboard input handling
}

/* application cleanup callback */
void app_cleanup() {
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
		.width = MZ800_DISP_WIDTH + BORDER_LEFT + BORDER_RIGHT,
		.height = 2 * MZ800_DISP_HEIGHT + BORDER_TOP + BORDER_BOTTOM,
		.window_title = "MZ-800",
		.icon.sokol_default = true,
		.enable_dragndrop = true
	};
}

// MARK: - File loading
static void handle_file_loading(void) {
	fs_dowork();
	const uint32_t load_delay_frames = 120;
	if (fs_ptr() && (clock_frame_count_60hz() > load_delay_frames)) {
		bool load_success = mzf_load(fs_ptr(), fs_size(), &state.mz800, state.mz800.dram);
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
		fs_reset();
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

	sdtx_puts("mz800");

	sdtx_font(0);
	sdtx_color1i(text_color);
	sdtx_pos(0.0f, 1.5f);
	sdtx_printf("frame:%.2fms emu:%.2fms (min:%.2fms max:%.2fms) ticks:%d", (float)state.frame_time_us * 0.001f, emu_stats.avg_val, emu_stats.min_val, emu_stats.max_val, state.ticks);
}

