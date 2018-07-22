; Test setting a border color

include "MZ800.inc"

org 02000h

  ; Set border
  ld b, PortBorderColorHigh ; b contains the upper part of the port address
  ld c, PortBorderColorLow ; c contains the lower part of the port address
  ld a, ColorLightBlue
  out (c), a
  halt
