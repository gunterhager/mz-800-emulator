; Writing to MZ-700 character and color VRAM
;
; Use z80asm from the z88dk project to build binary file.
; Command line:
; z80asm -b TestMZ700VRAM.asm

include "MZ800.inc"

org 02000h

; Program features
;define ClearScreen
define ClearColor

defc DisplayMode = DisplayMode40x25_8ColorMZ700

  ; Set display mode
  ld	a, DisplayMode
  out	(PortDisplayModeRegister), a

  ; Bank in VRAM
  out (PortBank_ROM1_CGROM_VRAM_ROM2), a ; contents of a don't matter

  ; Set VRAM write format
  ld a, WriteFormatMZ700
  out (PortWriteFormatRegister), a

  ; Set VRAM read format
  ld a, ReadFormatMZ700
  out (PortReadFormatRegister), a

ifdef ClearScreen

  ; Setup memory addresses for character VRAM
  ld hl, MemoryMZ700VRAMStart
  ld de, MemoryMZ700VRAMStart + 1
  ld bc, MemoryMZ700VRAMColorEnd - MemoryMZ700VRAMColorStart

  ; Clear VRAM
  ld (hl), 01h ; clear first byte of VRAM
  ldir ; clear the rest of the VRAM in one loop

endif

ifdef ClearColor

  ; Setup memory addresses for color VRAM
  ld hl, MemoryMZ700VRAMColorStart
  ld de, MemoryMZ700VRAMColorStart + 1
  ld bc, MemoryMZ700VRAMColorEnd - MemoryMZ700VRAMColorStart

  ; Clear VRAM
  ld (hl), 71h ; clear first byte of VRAM with white foreground, blue background
  ldir ; clear the rest of the VRAM in one loop

  ;halt

endif

  ; Put character on screen
  ld hl, MemoryMZ700VRAMStart
  ld de, 40 ; characters per line
  ld a, 01h ; character code
  ld b, 4
characterLoop:
  ld (hl), a
  add hl, de
  djnz characterLoop

  halt
