//
//  ui_gdg_whid65040_032.h
//  MZ-800-Emulator
//
//  Created by Gunter Hager on 27.01.22.
//

/*#
	# ui_gdg_whid65040_032.h

	Debug visualization for gdg_whid65040_032.h

#*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* setup parameters for ui_gdg_whid65040_032_init()
	NOTE: all string data must remain alive until ui_gdg_whid65040_032_discard() is called!
*/
typedef struct {
	const char* title;          /* window title */
	gdg_whid65040_032_t* gdg;   /* pointer to gdg_whid65040_032_t instance to track */
	int x, y;                   /* initial window position */
	int w, h;                   /* initial window size or 0 for default size */
	bool open;                  /* initial open state */
	ui_chip_desc_t chip_desc;   /* chip visualization desc */
} ui_gdg_whid65040_032_desc_t;

typedef struct {
	const char* title;
	gdg_whid65040_032_t* gdg;
	float init_x, init_y;
	float init_w, init_h;
	bool open;
	bool valid;
	ui_chip_t chip;
} ui_gdg_whid65040_032_t;

void ui_gdg_whid65040_032_init(ui_gdg_whid65040_032_t* win, const ui_gdg_whid65040_032_desc_t* desc);
void ui_gdg_whid65040_032_discard(ui_gdg_whid65040_032_t* win);
void ui_gdg_whid65040_032_draw(ui_gdg_whid65040_032_t* win);

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

void ui_gdg_whid65040_032_init(ui_gdg_whid65040_032_t* win, const ui_gdg_whid65040_032_desc_t* desc) {
	CHIPS_ASSERT(win && desc);
	CHIPS_ASSERT(desc->title);
	CHIPS_ASSERT(desc->gdg);
	memset(win, 0, sizeof(ui_gdg_whid65040_032_t));
	win->title = desc->title;
	win->gdg = desc->gdg;
	win->init_x = (float) desc->x;
	win->init_y = (float) desc->y;
	win->init_w = (float) ((desc->w == 0) ? 440 : desc->w);
	win->init_h = (float) ((desc->h == 0) ? 370 : desc->h);
	win->open = desc->open;
	win->valid = true;
	ui_chip_init(&win->chip, &desc->chip_desc);
}

void ui_gdg_whid65040_032_discard(ui_gdg_whid65040_032_t* win) {
	CHIPS_ASSERT(win && win->valid);
	win->valid = false;
}

// MARK: - Draw

static const char* plane_number[4] = { "I", "II", "III", "IV" };

#define GDG_WriteFormatSingleWrite (0x00)
#define GDG_WriteFormatXOR         (0x20)
#define GDG_WriteFormatOR          (0x40)
#define GDG_WriteFormatReset       (0x60)
#define GDG_WriteFormatReplace     (0x80)
#define GDG_WriteFormatPSET        (0xc0)

static const char* _ui_gdg_whid65040_032_write_mode(uint8_t wmd) {
	switch (wmd & 0xe0) {
		case GDG_WriteFormatSingleWrite: return "SINGLE "; break;
		case GDG_WriteFormatXOR:         return "XOR    "; break;
		case GDG_WriteFormatOR:          return "OR     "; break;
		case GDG_WriteFormatReset:       return "RESET  "; break;
		case GDG_WriteFormatReplace:     return "REPLACE"; break;
		case GDG_WriteFormatPSET:        return "PSET   "; break;
		default:                         return "N/A    "; break;
	}
}

static const char* _ui_gdg_whid65040_032_plane_number(uint8_t plane) {
	switch (plane & 0x0f) {
		case 0x1:       return "I  "; break;
		case 0x1 << 1:  return "II "; break;
		case 0x1 << 2:  return "III"; break;
		case 0x1 << 3:  return "IV "; break;
		default:        return "-  "; break;
	}
}

static const char* _ui_gdg_whid65040_032_dmd(uint8_t dmd) {
	switch (dmd & 0x0f) {
		case 0x0: return "320x200 /  4 colors / A / I, II"; break;
		case 0x1: return "320x200 /  4 colors / B / III, IV"; break;
		case 0x2: return "320x200 / 16 colors / A / I, II, III, IV"; break;
		case 0x4: return "640x200 /  1 colors / A / I"; break;
		case 0x5: return "640x200 /  1 colors / B / III"; break;
		case 0x6: return "640x200 /  4 colors / A / I, III"; break;
		case 0x8: return "MZ-700"; break;
		default:  return "N/A"; break;
	}
}

static void _ui_gdg_whid65040_032_draw_registers(ui_gdg_whid65040_032_t* win) {
	gdg_whid65040_032_t* gdg = win->gdg;
	ImGui::Text("MZ Mode: %s", (gdg->is_mz700) ? "MZ-700":"MZ-800");

	ImGui::Text("WF     %02X", gdg->wf); ImGui::SameLine();
	ImGui::Text(" %s", _ui_gdg_whid65040_032_write_mode(gdg->wf)); ImGui::SameLine();
	ImGui::Text(" %s", (gdg->rf & (0x1 << 4)) ? "B":"A"); ImGui::SameLine();
	ImGui::Text(" %s", _ui_gdg_whid65040_032_plane_number(gdg->rf & (0x1 << 3)));  ImGui::SameLine();
	ImGui::Text(" %s", _ui_gdg_whid65040_032_plane_number(gdg->rf & (0x1 << 2))); ImGui::SameLine();
	ImGui::Text(" %s", _ui_gdg_whid65040_032_plane_number(gdg->rf & (0x1 << 1))); ImGui::SameLine();
	ImGui::Text(" %s", _ui_gdg_whid65040_032_plane_number(gdg->rf & 0x1));

	ImGui::Text("RF     %02X", gdg->rf); ImGui::SameLine();
	ImGui::Text(" %s", (gdg->rf & (0x1 << 7)) ? "SRCH":"SING"); ImGui::SameLine();
	ImGui::Text(" %s", (gdg->rf & (0x1 << 4)) ? "B":"A"); ImGui::SameLine();
	ImGui::Text(" %s", _ui_gdg_whid65040_032_plane_number(gdg->rf));

	ImGui::Text("DMD    %02X", gdg->dmd); ImGui::SameLine();
	ImGui::Text(" %s", _ui_gdg_whid65040_032_dmd(gdg->dmd));

	ImGui::Text("STATUS %02X", gdg->status);
}

static void _ui_gdg_whid65040_032_draw_scroll(ui_gdg_whid65040_032_t* win) {
	gdg_whid65040_032_t* gdg = win->gdg;
	ImGui::Text("SOF1   %02X", gdg->sof1);
	ImGui::Text("SOF2   %02X", gdg->sof2);
	ImGui::Text("SW     %02X", gdg->sw);
	ImGui::Text("SSA    %02X", gdg->ssa);
	ImGui::Text("SEA    %02X", gdg->sea);
}

static void _ui_gdg_whid65040_032_draw_border_color(ui_gdg_whid65040_032_t* win) {
	gdg_whid65040_032_t* gdg = win->gdg;
	ImGui::Text("Border Color:");
	const ImVec2 size(18,18);
	ImGui::ColorButton("##brd_color", ImColor(gdg->bcol_rgba8), ImGuiColorEditFlags_NoAlpha, size);
}

static void _ui_gdg_whid65040_032_draw_palette(ui_gdg_whid65040_032_t* win) {
	gdg_whid65040_032_t* gdg = win->gdg;
	ImGui::Text("Palette:");
	const ImVec2 size(18,18);
	for (int i = 0; i < 4; i++) {
		ImGui::PushID(i);
		ImGui::ColorButton("##plt_color", ImColor(gdg->plt_rgba8[i]), ImGuiColorEditFlags_NoAlpha, size);
		ImGui::PopID();
		if (((i+1) % 4) != 0) {
			ImGui::SameLine();
		}
	}
}

//static void _ui_am40010_draw_registers(ui_am40010_t* win) {
//	am40010_registers_t* r = &win->am40010->regs;
//	ImGui::Text("INKSEL %02X", r->inksel);
//	ImGui::Text("BORDER %02X", r->border);
//	ImGui::Text("INK   "); ImGui::SameLine();
//	for (int i = 0; i < 16; i++) {
//		ImGui::Text("%02X", r->ink[i]);
//		if (((i+1) % 8) != 0) {
//			ImGui::SameLine();
//		}
//		else if (i < 15) {
//			ImGui::Text("      "); ImGui::SameLine();
//		}
//	}
//	ImGui::Text("CONFIG %02X", r->config);
//	ImGui::Text("  Mode     %d", r->config & 3);
//	ImGui::Text("  LoROM    %s", (r->config & AM40010_CONFIG_LROMEN) ? "ON":"OFF");
//	ImGui::Text("  HiROM    %s", (r->config & AM40010_CONFIG_HROMEN) ? "ON":"OFF");
//	ImGui::Text("  IRQRes   %s", (r->config & AM40010_CONFIG_IRQRESET) ? "ON":"OFF");
//}


static void _ui_gdg_whid65040_032_draw_state(ui_gdg_whid65040_032_t* win) {
	gdg_whid65040_032_t* gdg = win->gdg;
	if (ImGui::CollapsingHeader("Registers", ImGuiTreeNodeFlags_DefaultOpen)) {
		_ui_gdg_whid65040_032_draw_registers(win);
	}
	if (ImGui::CollapsingHeader("Scroll", ImGuiTreeNodeFlags_DefaultOpen)) {
		_ui_gdg_whid65040_032_draw_scroll(win);
	}
	if (ImGui::CollapsingHeader("Palette & Border", ImGuiTreeNodeFlags_DefaultOpen)) {
		_ui_gdg_whid65040_032_draw_border_color(win);
		_ui_gdg_whid65040_032_draw_palette(win);
	}
	if (ImGui::CollapsingHeader("Planes", ImGuiTreeNodeFlags_DefaultOpen)) {
	}
//	ImGui::Columns(5, "##gdg_ports", false);
//	ImGui::SetColumnWidth(0, 64);
//	ImGui::SetColumnWidth(1, 32);
//	ImGui::SetColumnWidth(2, 32);
//	ImGui::SetColumnWidth(3, 32);
//	ImGui::SetColumnWidth(4, 32);
//	ImGui::NextColumn();
//	ImGui::Text("A"); ImGui::NextColumn();
//	ImGui::Text("B"); ImGui::NextColumn();
//	ImGui::Text("CHI"); ImGui::NextColumn();
//	ImGui::Text("CLO"); ImGui::NextColumn();
//	ImGui::Separator();
//	ImGui::Text("Mode"); ImGui::NextColumn();
//	ImGui::Text("%d", (ppi->control & I8255_CTRL_ACHI_MODE) >> 5); ImGui::NextColumn();
//	ImGui::Text("%d", (ppi->control & I8255_CTRL_BCLO_MODE) >> 2); ImGui::NextColumn();
//	ImGui::Text("%d", (ppi->control & I8255_CTRL_ACHI_MODE) >> 5); ImGui::NextColumn();
//	ImGui::Text("%d", (ppi->control & I8255_CTRL_BCLO_MODE) >> 2); ImGui::NextColumn();
//	ImGui::Text("In/Out"); ImGui::NextColumn();
//	ImGui::Text("%s", ((ppi->control & I8255_CTRL_A) == I8255_CTRL_A_INPUT) ? "IN":"OUT"); ImGui::NextColumn();
//	ImGui::Text("%s", ((ppi->control & I8255_CTRL_B) == I8255_CTRL_B_INPUT) ? "IN":"OUT"); ImGui::NextColumn();
//	ImGui::Text("%s", ((ppi->control & I8255_CTRL_CHI) == I8255_CTRL_CHI_INPUT) ? "IN":"OUT"); ImGui::NextColumn();
//	ImGui::Text("%s", ((ppi->control & I8255_CTRL_CLO) == I8255_CTRL_CLO_INPUT) ? "IN":"OUT"); ImGui::NextColumn();
//	ImGui::Text("Output"); ImGui::NextColumn();
//	ImGui::Text("%02X", ppi->pa.outp); ImGui::NextColumn();
//	ImGui::Text("%02X", ppi->pb.outp); ImGui::NextColumn();
//	ImGui::Text("%X", ppi->pc.outp >> 4); ImGui::NextColumn();
//	ImGui::Text("%X", ppi->pc.outp & 0xF); ImGui::NextColumn();
//	ImGui::Columns(); ImGui::Separator();
//	ImGui::Text("Control: %02X", ppi->control);
}

void ui_gdg_whid65040_032_draw(ui_gdg_whid65040_032_t* win) {
	CHIPS_ASSERT(win && win->valid && win->title && win->gdg);
	if (!win->open) {
		return;
	}
	ImGui::SetNextWindowPos(ImVec2(win->init_x, win->init_y), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(win->init_w, win->init_h), ImGuiCond_Once);
	if (ImGui::Begin(win->title, &win->open)) {
//		ImGui::BeginChild("##gdg_whid65040_032_chip", ImVec2(176, 0), true);
//		ui_chip_draw(&win->chip, win->gdg->pins);
//		ImGui::EndChild();
//		ImGui::SameLine();
		ImGui::BeginChild("##gdg_whid65040_032_state", ImVec2(0, 0), true);
		_ui_gdg_whid65040_032_draw_state(win);
		ImGui::EndChild();
	}
	ImGui::End();
}
#endif
