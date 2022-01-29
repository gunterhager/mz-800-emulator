; Test writing too much into VRAM in MZ-700 mode
;
; Use z80asm from the z88dk project to build binary file.
; Command line:
; z80asm -b TestMZ700VRAMOverwrite.asm

include "MZ800.inc"

org 02000h

  ld a, 01
  ld hl, 01000h
  ld (hl), a
  halt

; Program features
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

ifdef ClearColor

  ; Setup memory addresses for color VRAM
  ld hl, MemoryMZ700VRAMColorStart
  ld de, MemoryMZ700VRAMColorStart + 1
  ld bc, 0ffffh ; load maximum number into byte counter

  ; Clear VRAM
  ld (hl), 071h ; clear first byte of VRAM with white foreground, blue background
  ldir ; clear the rest of the VRAM in one loop

endif

  ; Put character on screen
  ld hl, MemoryMZ700VRAMStart
  ld de, 40 ; characters per line
  ld a, 01h ; character code
  ld b, 14
characterLoop:
  ld (hl), a
  add a, 1
  add hl, de
  djnz characterLoop


  ; Put color on screen
  ld hl, MemoryMZ700VRAMColorStart
  ld de, 40 ; characters per line
  ld a, 71h ; color code
  ld b, 14
colorLoop:
  ld (hl), a
  add a, 1
  add hl, de
  djnz colorLoop

  halt
