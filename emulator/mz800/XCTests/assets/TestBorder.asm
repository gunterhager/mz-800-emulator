; Test setting a border color
;
; Use z80asm from the z88dk project to build binary file.
; Command line:
; z80asm -b TestBorder.asm

include "MZ800.inc"

org 02000h

  ; Set border
  ld b, PortBorderColorHigh ; b contains the upper part of the port address
  ld c, PortBorderColorLow ; c contains the lower part of the port address
  ld a, ColorLightBlue
  out (c), a
  halt
