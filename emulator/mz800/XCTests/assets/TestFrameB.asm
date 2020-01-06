; Writing to Frame B in 640x200, 1 color and then switch to 640x200, 4 color
;
; Use z80asm from the z88dk project to build binary file.
; Command line:
; z80asm -b TestFrameB.asm

include "MZ800.inc"

org 02000h

; Program features
define UseHiRes
define ClearScreen

ifdef UseHiRes
  defc ScreenLines = 200
  defc BytesPerLine = 80
  defc DisplayMode = DisplayMode640x200_1ColorFrameB
  defc Planes = FormatPlaneIII
else
  defc ScreenLines = 200
  defc BytesPerLine = 40
  defc DisplayMode = DisplayMode320x200_4ColorFrameB
  defc Planes = FormatPlaneI | FormatPlaneII
endif

  ; Set display mode
  ld	a, DisplayMode
  out	(PortDisplayModeRegister), a

  ; Bank in VRAM
  out (PortBank_ROM1_CGROM_VRAM_ROM2), a ; contents of a don't matter

ifdef ClearScreen

  ; Set VRAM write format
  ld a, WriteFormatSingleWrite | Planes
  out (PortWriteFormatRegister), a

  ; Setup memory addresses
  ld hl, MemoryVRAMStart
  ld de, MemoryVRAMStart + 1
  ld bc, BytesPerLine * ScreenLines - 1 ; length of VRAM - 1

  ; Clear VRAM
  ld (hl), 0 ; clear first byte of VRAM
  ldir ; clear the rest of the VRAM in one loop

endif

  ; Set VRAM write format
  ld a, WriteFormatPSET | FormatPlaneIII
  out (PortWriteFormatRegister), a

  ; Put pixel on screen
  ld hl, MemoryVRAMStart ;+ BytesPerLine - 1
  ld de, BytesPerLine
  ld a, 000110011b
  ld b, ScreenLines - 1
pixelLoop:
  ld (hl), a
  add hl, de
  djnz pixelLoop

  halt
