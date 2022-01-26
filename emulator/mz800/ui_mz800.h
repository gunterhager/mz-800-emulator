#pragma once
//
//  ui_mz800.h
//  MZ-800-Emulator
//
//  Created by Gunter Hager on 26.01.22.
//

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	mz800_t* mz800;
	ui_dbg_create_texture_t create_texture_cb;      /* texture creation callback for ui_dbg_t */
	ui_dbg_update_texture_t update_texture_cb;      /* texture update callback for ui_dbg_t */
	ui_dbg_destroy_texture_t destroy_texture_cb;    /* texture destruction callback for ui_dbg_t */
	ui_dbg_keys_desc_t dbg_keys;          /* user-defined hotkeys for ui_dbg_t */
} ui_mz800_desc_t;

typedef struct {
	mz800_t* mz800;
	int dbg_scanline;
	bool dbg_vsync;
	ui_z80_t cpu;
	ui_z80pio_t pio;
	ui_i8255_t ppi;
	ui_audio_t audio;
	ui_kbd_t kbd;
	ui_memmap_t memmap;
	ui_memedit_t memedit[4];
	ui_dasm_t dasm[4];
	ui_dbg_t dbg;
} ui_mz800_t;

void ui_mz800_init(ui_mz800_t* ui, const ui_mz800_desc_t* ui_desc);
void ui_mz800_discard(ui_mz800_t* ui);
void ui_mz800_draw(ui_mz800_t* ui);
mz800_debug_t ui_mz800_get_debug(ui_mz800_t* ui);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION (include in C++ source) ----------------------------------*/
#ifdef CHIPS_UI_IMPL
#ifndef __cplusplus
#error "implementation must be compiled as C++"
#endif
#include <string.h> /* memset */
#ifndef CHIPS_ASSERT
	#include <assert.h>
	#define CHIPS_ASSERT(c) assert(c)
#endif
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif

// MARK: - Pins

static const ui_chip_pin_t _ui_mz800_cpu_pins[] = {
	{ "D0",     0,      Z80_D0 },
	{ "D1",     1,      Z80_D1 },
	{ "D2",     2,      Z80_D2 },
	{ "D3",     3,      Z80_D3 },
	{ "D4",     4,      Z80_D4 },
	{ "D5",     5,      Z80_D5 },
	{ "D6",     6,      Z80_D6 },
	{ "D7",     7,      Z80_D7 },
	{ "M1",     8,      Z80_M1 },
	{ "MREQ",   9,      Z80_MREQ },
	{ "IORQ",   10,     Z80_IORQ },
	{ "RD",     11,     Z80_RD },
	{ "WR",     12,     Z80_WR },
	{ "RFSH",   13,     Z80_RFSH },
	{ "HALT",   14,     Z80_HALT },
	{ "INT",    15,     Z80_INT },
	{ "NMI",    16,     Z80_NMI },
	{ "WAIT",   17,     Z80_WAIT },
	{ "A0",     18,     Z80_A0 },
	{ "A1",     19,     Z80_A1 },
	{ "A2",     20,     Z80_A2 },
	{ "A3",     21,     Z80_A3 },
	{ "A4",     22,     Z80_A4 },
	{ "A5",     23,     Z80_A5 },
	{ "A6",     24,     Z80_A6 },
	{ "A7",     25,     Z80_A7 },
	{ "A8",     26,     Z80_A8 },
	{ "A9",     27,     Z80_A9 },
	{ "A10",    28,     Z80_A10 },
	{ "A11",    29,     Z80_A11 },
	{ "A12",    30,     Z80_A12 },
	{ "A13",    31,     Z80_A13 },
	{ "A14",    32,     Z80_A14 },
	{ "A15",    33,     Z80_A15 },
};

static const ui_chip_pin_t _ui_mz800_pio_pins[] = {
	{ "D0",     0,      Z80_D0 },
	{ "D1",     1,      Z80_D1 },
	{ "D2",     2,      Z80_D2 },
	{ "D3",     3,      Z80_D3 },
	{ "D4",     4,      Z80_D4 },
	{ "D5",     5,      Z80_D5 },
	{ "D6",     6,      Z80_D6 },
	{ "D7",     7,      Z80_D7 },
	{ "CE",     9,      Z80PIO_CE },
	{ "BASEL",  10,     Z80PIO_BASEL },
	{ "CDSEL",  11,     Z80PIO_CDSEL },
	{ "M1",     12,     Z80PIO_M1 },
	{ "IORQ",   13,     Z80PIO_IORQ },
	{ "RD",     14,     Z80PIO_RD },
	{ "INT",    15,     Z80PIO_INT },
	{ "ARDY",   20,     Z80PIO_ARDY },
	{ "ASTB",   21,     Z80PIO_ASTB },
	{ "PA0",    22,     Z80PIO_PA0 },
	{ "PA1",    23,     Z80PIO_PA1 },
	{ "PA2",    24,     Z80PIO_PA2 },
	{ "PA3",    25,     Z80PIO_PA3 },
	{ "PA4",    26,     Z80PIO_PA4 },
	{ "PA5",    27,     Z80PIO_PA5 },
	{ "PA6",    28,     Z80PIO_PA6 },
	{ "PA7",    29,     Z80PIO_PA7 },
	{ "BRDY",   30,     Z80PIO_ARDY },
	{ "BSTB",   31,     Z80PIO_ASTB },
	{ "PB0",    32,     Z80PIO_PB0 },
	{ "PB1",    33,     Z80PIO_PB1 },
	{ "PB2",    34,     Z80PIO_PB2 },
	{ "PB3",    35,     Z80PIO_PB3 },
	{ "PB4",    36,     Z80PIO_PB4 },
	{ "PB5",    37,     Z80PIO_PB5 },
	{ "PB6",    38,     Z80PIO_PB6 },
	{ "PB7",    39,     Z80PIO_PB7 },
};

static const ui_chip_pin_t _ui_mz800_ppi_pins[] = {
	{ "D0",     0,      I8255_D0 },
	{ "D1",     1,      I8255_D1 },
	{ "D2",     2,      I8255_D2 },
	{ "D3",     3,      I8255_D3 },
	{ "D4",     4,      I8255_D4 },
	{ "D5",     5,      I8255_D5 },
	{ "D6",     6,      I8255_D6 },
	{ "D7",     7,      I8255_D7 },

	{ "CS",     9,      I8255_CS },
	{ "RD",    10,      I8255_RD },
	{ "WR",    11,      I8255_WR },
	{ "A0",    12,      I8255_A0 },
	{ "A1",    13,      I8255_A1 },

	{ "PC0",   16,      I8255_PC0 },
	{ "PC1",   17,      I8255_PC1 },
	{ "PC2",   18,      I8255_PC2 },
	{ "PC3",   19,      I8255_PC3 },

	{ "PA0",   20,      I8255_PA0 },
	{ "PA1",   21,      I8255_PA1 },
	{ "PA2",   22,      I8255_PA2 },
	{ "PA3",   23,      I8255_PA3 },
	{ "PA4",   24,      I8255_PA4 },
	{ "PA5",   25,      I8255_PA5 },
	{ "PA6",   26,      I8255_PA6 },
	{ "PA7",   27,      I8255_PA7 },

	{ "PB0",   28,      I8255_PB0 },
	{ "PB1",   29,      I8255_PB1 },
	{ "PB2",   30,      I8255_PB2 },
	{ "PB3",   31,      I8255_PB3 },
	{ "PB4",   32,      I8255_PB4 },
	{ "PB5",   33,      I8255_PB5 },
	{ "PB6",   34,      I8255_PB6 },
	{ "PB7",   35,      I8255_PB7 },

	{ "PC4",   36,      I8255_PC4 },
	{ "PC5",   37,      I8255_PC5 },
	{ "PC6",   38,      I8255_PC6 },
	{ "PC7",   39,      I8255_PC7 },
};

// MARK: - Memory

#define _UI_MZ800_MEMLAYER_NUM (4)
static const char* _ui_mz800_memlayer_names[_UI_MZ800_MEMLAYER_NUM] = {
	"ROM 1", "CG ROM", "ROM 2", "RAM"
};

static uint8_t _ui_mz800_mem_read(int layer, uint16_t addr, void* user_data) {
	CHIPS_ASSERT(user_data);
	mz800_t* mz800 = (mz800_t*) user_data;
	if (layer == 0) {
		return mem_rd(&mz800->mem, addr);
	}
	else {
		return mem_layer_rd(&mz800->mem, layer-1, addr);
	}
}

static void _ui_mz800_mem_write(int layer, uint16_t addr, uint8_t data, void* user_data) {
	CHIPS_ASSERT(user_data);
	mz800_t* mz800 = (mz800_t*) user_data;
	if (layer == 0) {
		mem_wr(&mz800->mem, addr, data);
	}
	else {
		mem_layer_wr(&mz800->mem, layer-1, addr, data);
	}
}

static void _ui_mz800_update_memmap(ui_mz800_t* ui) {
	CHIPS_ASSERT(ui && ui->mz800);
	const mz800_t* mz800 = ui->mz800;
	ui_memmap_reset(&ui->memmap);
//	const uint8_t rom_enable = cpc->ga.regs.config;
//	if ((cpc->type == CPC_TYPE_464) || (cpc->type == CPC_TYPE_KCCOMPACT)) {
//		ui_memmap_layer(&ui->memmap, "ROM");
//			ui_memmap_region(&ui->memmap, "Lower ROM (OS)", 0x0000, 0x4000, !(rom_enable & AM40010_CONFIG_LROMEN));
//			ui_memmap_region(&ui->memmap, "Upper ROM (BASIC)", 0xC000, 0x4000, !(rom_enable & AM40010_CONFIG_HROMEN));
//		ui_memmap_layer(&ui->memmap, "RAM");
//			ui_memmap_region(&ui->memmap, "RAM 0", 0x0000, 0x4000, true);
//			ui_memmap_region(&ui->memmap, "RAM 1", 0x4000, 0x4000, true);
//			ui_memmap_region(&ui->memmap, "RAM 2", 0x8000, 0x4000, true);
//			ui_memmap_region(&ui->memmap, "RAM 3 (Screen)", 0xC000, 0x4000, true);
//	}
//	else {
//		const uint8_t ram_config_index = cpc->ga.ram_config & 7;
//		const uint8_t rom_select = cpc->ga.rom_select;
//		ui_memmap_layer(&ui->memmap, "ROM Layer 0");
//			ui_memmap_region(&ui->memmap, "OS ROM", 0x0000, 0x4000, !(rom_enable & AM40010_CONFIG_LROMEN));
//			ui_memmap_region(&ui->memmap, "BASIC ROM", 0xC000, 0x4000, !(rom_enable & AM40010_CONFIG_HROMEN) && (rom_select != 7));
//		ui_memmap_layer(&ui->memmap, "ROM Layer 1");
//			ui_memmap_region(&ui->memmap, "AMSDOS ROM", 0xC000, 0x4000, !(rom_enable & AM40010_CONFIG_HROMEN)  && (rom_select == 7));
//		for (int bank = 0; bank < 8; bank++) {
//			ui_memmap_layer(&ui->memmap, _ui_cpc_ram_banks[bank]);
//			bool bank_active = false;
//			for (int slot = 0; slot < 4; slot++) {
//				if (bank == _ui_cpc_ram_config[ram_config_index][slot]) {
//					ui_memmap_region(&ui->memmap, _ui_cpc_ram_name[bank], 0x4000*slot, 0x4000, true);
//					bank_active = true;
//				}
//			}
//			if (!bank_active) {
//				ui_memmap_region(&ui->memmap, _ui_cpc_ram_name[bank], 0x0000, 0x4000, false);
//			}
//		}
//	}
}

// MARK: - Menu

static void _ui_mz800_draw_menu(ui_mz800_t* ui) {
	CHIPS_ASSERT(ui && ui->mz800);
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("Hardware")) {
			ImGui::MenuItem("Memory Map", 0, &ui->memmap.open);
			ImGui::MenuItem("Keyboard Matrix", 0, &ui->kbd.open);
			ImGui::MenuItem("Audio Output", 0, &ui->audio.open);
			ImGui::MenuItem("Z80 (CPU)", 0, &ui->cpu.open);
			ImGui::MenuItem("Z80 (PIO)", 0, &ui->pio.open);
			ImGui::MenuItem("i8255 (PPI)", 0, &ui->ppi.open);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Debug")) {
			ImGui::MenuItem("CPU Debugger", 0, &ui->dbg.ui.open);
			ImGui::MenuItem("Breakpoints", 0, &ui->dbg.ui.show_breakpoints);
			ImGui::MenuItem("Execution History", 0, &ui->dbg.ui.show_history);
			ImGui::MenuItem("Memory Heatmap", 0, &ui->dbg.ui.show_heatmap);
			if (ImGui::BeginMenu("Memory Editor")) {
				ImGui::MenuItem("Window #1", 0, &ui->memedit[0].open);
				ImGui::MenuItem("Window #2", 0, &ui->memedit[1].open);
				ImGui::MenuItem("Window #3", 0, &ui->memedit[2].open);
				ImGui::MenuItem("Window #4", 0, &ui->memedit[3].open);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Disassembler")) {
				ImGui::MenuItem("Window #1", 0, &ui->dasm[0].open);
				ImGui::MenuItem("Window #2", 0, &ui->dasm[1].open);
				ImGui::MenuItem("Window #3", 0, &ui->dasm[2].open);
				ImGui::MenuItem("Window #4", 0, &ui->dasm[3].open);
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		ui_util_options_menu();
		ImGui::EndMainMenuBar();
	}
}

// MARK: - Breakpoints

static int _ui_mz800_eval_bp(ui_dbg_t* dbg_win, int trap_id, uint64_t pins, void* user_data) {
	(void)pins;
	CHIPS_ASSERT(user_data);
//	ui_mz800_t* ui_mz800 = (ui_mz800_t*) user_data;
//	mz800_t* mz800 = ui_mz800->mz800;
//	int scanline = mz800->ga.crt.v_pos;
//	bool vsync = mz800->crtc.vs;
//	for (int i = 0; (i < dbg_win->dbg.num_breakpoints) && (trap_id == 0); i++) {
//		const ui_dbg_breakpoint_t* bp = &dbg_win->dbg.breakpoints[i];
//		if (bp->enabled) {
//			switch (bp->type) {
//				/* scanline number */
//				case UI_DBG_BREAKTYPE_USER+0:
//					if ((ui_cpc->dbg_scanline != scanline) && (scanline == bp->val)) {
//						trap_id = UI_DBG_BP_BASE_TRAPID + i;
//					}
//					break;
//				/* new scanline */
//				case UI_DBG_BREAKTYPE_USER+1:
//					if (ui_cpc->dbg_scanline != scanline) {
//						trap_id = UI_DBG_BP_BASE_TRAPID + i;
//					}
//					break;
//				/* new frame */
//				case UI_DBG_BREAKTYPE_USER+2:
//					if ((ui_cpc->dbg_scanline != scanline) && (scanline == 0)) {
//						trap_id = UI_DBG_BP_BASE_TRAPID + i;
//					}
//					break;
//				/* VSYNC */
//				case UI_DBG_BREAKTYPE_USER+3:
//					if (vsync && !ui_cpc->dbg_vsync) {
//						trap_id = UI_DBG_BP_BASE_TRAPID + i;
//					}
//					break;
//			}
//		}
//	}
//	ui_mz800->dbg_scanline = scanline;
//	ui_mz800->dbg_vsync = vsync;
	return trap_id;
}

// MARK: - Life cycle

void ui_mz800_init(ui_mz800_t* ui, const ui_mz800_desc_t* ui_desc) {
	CHIPS_ASSERT(ui && ui_desc);
	CHIPS_ASSERT(ui_desc->mz800);
	ui->mz800 = ui_desc->mz800;

	int x = 20, y = 20, dx = 10, dy = 10;
	{
		ui_dbg_desc_t desc = {0};
		desc.title = "CPU Debugger";
		desc.x = x;
		desc.y = y;
		desc.z80 = &ui->mz800->cpu;
		desc.read_cb = _ui_mz800_mem_read;
		desc.break_cb = _ui_mz800_eval_bp;
		desc.create_texture_cb = ui_desc->create_texture_cb;
		desc.update_texture_cb = ui_desc->update_texture_cb;
		desc.destroy_texture_cb = ui_desc->destroy_texture_cb;
		desc.keys = ui_desc->dbg_keys;
		desc.user_data = ui;
		/* custom breakpoint types */
		desc.user_breaktypes[0].label = "Scanline at";
		desc.user_breaktypes[0].show_val16 = true;
		desc.user_breaktypes[1].label = "New Scanline";
		desc.user_breaktypes[2].label = "New Frame";
		desc.user_breaktypes[3].label = "VSYNC";
		ui_dbg_init(&ui->dbg, &desc);
	}
	x += dx; y += dy;
	{
		ui_z80_desc_t desc = {0};
		desc.title = "Z80 CPU";
		desc.cpu = &ui->mz800->cpu;
		desc.x = x;
		desc.y = y;
		UI_CHIP_INIT_DESC(&desc.chip_desc, "Z80\nCPU", 36, _ui_mz800_cpu_pins);
		ui_z80_init(&ui->cpu, &desc);
	}
	x += dx; y += dy;
	{
		ui_z80pio_desc_t desc = {0};
		desc.title = "Z80 PIO";
		desc.pio = &ui->mz800->pio;
		desc.x = x;
		desc.y = y;
		UI_CHIP_INIT_DESC(&desc.chip_desc, "Z80\nPIO", 40, _ui_mz800_pio_pins);
		ui_z80pio_init(&ui->pio, &desc);
	}
	x += dx; y += dy;
	{
		ui_i8255_desc_t desc = {0};
		desc.title = "i8255";
		desc.i8255 = &ui->mz800->ppi;
		desc.x = x;
		desc.y = y;
		UI_CHIP_INIT_DESC(&desc.chip_desc, "i8255", 40, _ui_mz800_ppi_pins);
		ui_i8255_init(&ui->ppi, &desc);
	}
	x += dx; y += dy;
	{
		ui_audio_desc_t desc = {0};
		desc.title = "Audio Output";
		desc.sample_buffer = ui->mz800->audio.sample_buffer;
		desc.num_samples = ui->mz800->audio.num_samples;
		desc.x = x;
		desc.y = y;
		ui_audio_init(&ui->audio, &desc);
	}
	x += dx; y += dy;
	{
		ui_kbd_desc_t desc = {0};
		desc.title = "Keyboard Matrix";
		desc.kbd = &ui->mz800->kbd;
		desc.layers[0] = "None";
		desc.layers[1] = "Shift";
		desc.layers[2] = "Ctrl";
		desc.x = x;
		desc.y = y;
		ui_kbd_init(&ui->kbd, &desc);
	}
	x += dx; y += dy;
	{
		ui_memedit_desc_t desc = {0};
		for (int i = 0; i < _UI_MZ800_MEMLAYER_NUM; i++) {
			desc.layers[i] = _ui_mz800_memlayer_names[i];
		}
		desc.read_cb = _ui_mz800_mem_read;
		desc.write_cb = _ui_mz800_mem_write;
		desc.user_data = ui;
		static const char* titles[] = { "Memory Editor #1", "Memory Editor #2", "Memory Editor #3", "Memory Editor #4" };
		for (int i = 0; i < 4; i++) {
			desc.title = titles[i]; desc.x = x; desc.y = y;
			ui_memedit_init(&ui->memedit[i], &desc);
			x += dx; y += dy;
		}
	}
	x += dx; y += dy;
	{
		ui_memmap_desc_t desc = {0};
		desc.title = "Memory Map";
		desc.x = x;
		desc.y = y;
		ui_memmap_init(&ui->memmap, &desc);
	}
	x += dx; y += dy;
	{
		ui_dasm_desc_t desc = {0};
		for (int i = 0; i < _UI_MZ800_MEMLAYER_NUM; i++) {
			desc.layers[i] = _ui_mz800_memlayer_names[i];
		}
		desc.cpu_type = UI_DASM_CPUTYPE_Z80;
		desc.start_addr = 0x0000;
		desc.read_cb = _ui_mz800_mem_read;
		desc.user_data = ui;
		static const char* titles[4] = { "Disassembler #1", "Disassembler #2", "Disassembler #2", "Dissassembler #3" };
		for (int i = 0; i < 4; i++) {
			desc.title = titles[i]; desc.x = x; desc.y = y;
			ui_dasm_init(&ui->dasm[i], &desc);
			x += dx; y += dy;
		}
	}}

void ui_mz800_discard(ui_mz800_t* ui) {
	CHIPS_ASSERT(ui && ui->mz800);
	ui->mz800 = 0;
	ui_z80_discard(&ui->cpu);
	ui_z80pio_discard(&ui->pio);
	ui_i8255_discard(&ui->ppi);
	ui_kbd_discard(&ui->kbd);
	ui_audio_discard(&ui->audio);
	ui_memmap_discard(&ui->memmap);
	for (int i = 0; i < 4; i++) {
		ui_memedit_discard(&ui->memedit[i]);
		ui_dasm_discard(&ui->dasm[i]);
	}
	ui_dbg_discard(&ui->dbg);
}

void ui_mz800_draw(ui_mz800_t* ui) {
	CHIPS_ASSERT(ui && ui->mz800);
	_ui_mz800_draw_menu(ui);
	if (ui->memmap.open) {
		_ui_mz800_update_memmap(ui);
	}
	ui_audio_draw(&ui->audio, ui->mz800->audio.sample_pos);
	ui_kbd_draw(&ui->kbd);
	ui_z80_draw(&ui->cpu);
	ui_z80pio_draw(&ui->pio);
	ui_i8255_draw(&ui->ppi);
	ui_memmap_draw(&ui->memmap);
	for (int i = 0; i < 4; i++) {
		ui_memedit_draw(&ui->memedit[i]);
		ui_dasm_draw(&ui->dasm[i]);
	}
	ui_dbg_draw(&ui->dbg);
}

mz800_debug_t ui_mz800_get_debug(ui_mz800_t* ui) {
	CHIPS_ASSERT(ui);
	mz800_debug_t res = {};
	res.callback.func = (mz800_debug_func_t)ui_dbg_tick;
	res.callback.user_data = &ui->dbg;
	res.stopped = &ui->dbg.dbg.stopped;
	return res;
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif /* CHIPS_UI_IMPL */
