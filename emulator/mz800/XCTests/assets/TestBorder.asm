; Test setting a border color
;
; Use z80asm from the z88dk project to build binary file.
; Command line:
; z80asm -b TestBorder.asm

include "MZ800.inc"

org 02000h

; -----------------
; Set stack
  ld hl, StackStart ; stack goes down from stack start
  ld sp, hl

; -----------------
; Main loop
main:
  ld a, ColorLightBlue
  call set_border
  call wait

  ld a, ColorYellow
  call set_border
  call wait

  ld a, ColorPurple
  call set_border
  call wait
  jr main

; -----------------
; Set border
; a contains color
set_border:
  ld b, PortBorderColorHigh ; b contains the upper part of the port address
  ld c, PortBorderColorLow ; c contains the lower part of the port address
  out (c), a
  ret

; -----------------
; Wait loop
wait:
  ld bc, 1000h
outer:
  ld de, 10h
inner:
  dec de
  ld a, d
  or e
  jr nz, inner
  dec bc
  ld a, b
  or c
  jr nz, outer
  ret

  defs 256 ; room for stack
StackStart:
