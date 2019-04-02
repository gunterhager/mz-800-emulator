; Dump all characters in CGROM to the screen without using monitor routines
;
; Use z80asm from the z88dk project to build binary file.
; Command line:
; z80asm -b TestCharacters.asm

include "MZ800.inc"

defc LoadingAddress = 02000h
org LoadingAddress

; Program features
define UseHiRes
;define DrawLine
define ClearScreen
define Border

ifdef UseHiRes
  defc ScreenLines = 200
  defc BytesPerLine = 80
  defc DisplayMode = DisplayMode640x200_4Color
  defc Planes = FormatPlaneI | FormatPlaneIII
else
  defc ScreenLines = 200
  defc BytesPerLine = 40
  defc DisplayMode = DisplayMode320x200_4ColorFrameA
  defc Planes = FormatPlaneI | FormatPlaneII
endif

  ; Set stack
  ld hl, StackStart ; stack goes down from stack start
  ld sp, hl

  ; Set display mode
  ld	a, DisplayMode
  out	(PortDisplayModeRegister), a

ifdef Border

  ; Set border
  ld b, PortBorderColorHigh ; b contains the upper part of the port address
  ld c, PortBorderColorLow ; c contains the lower part of the port address
  ld a, ColorLightBlue
  out (c), a

endif

  ; Setup palette
  ld a, Palette0 | ColorBlue
  out (PortPaletteRegister), a
  ld a, Palette1 | ColorLightGreen
  out (PortPaletteRegister), a
  ld a, Palette2 | ColorLightPurple
  out (PortPaletteRegister), a
  ld a, Palette3 | ColorLightWhite
  out (PortPaletteRegister), a

  ; Bank in VRAM
  out (PortBank_ROM1_CGROM_VRAM_ROM2), a ; contents of a don't matter
  ;in a, (PortInBank0)

  ; Set VRAM read format
  ld a, Planes
  out (PortReadFormatRegister), a

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


ifdef DrawLine

  ; Set VRAM write format
  ld a, WriteFormatPSET | FormatPlaneI
  out (PortWriteFormatRegister), a

  ; Put pixel on screen
  ld hl, MemoryVRAMStart ;+ BytesPerLine - 1
  ld de, BytesPerLine
  ld a, 000000001b
  ld b, ScreenLines - 1
pixelLoop:
  ld (hl), a
  add hl, de
  djnz pixelLoop

else

ifdef UseHiRes
  ld a, WriteFormatPSET | FormatPlaneI | FormatPlaneIII
else
  ld a, WriteFormatPSET | FormatPlaneI | FormatPlaneII
endif
  out (PortWriteFormatRegister), a

  
  ld hl, MemoryVRAMStart
  ld ix, CGROM_A

  ld b, 25 ; lines on screen

  exx ; alternate registers
  ld hl, 511 ; number of characters, starting with A in CGROM
  ld de, 1 ; to decrement with setting z flag
  exx ; normal registers

drawLines:
  push hl ; save registers
  push ix
  push bc
  call drawCharacterLine
  pop bc
  pop ix
  pop hl

  ld de, BytesPerLine * 8
  add hl, de
  add ix, de

  djnz drawLines


endif

halt:
  halt

;--------------------
; Draw a line of characters from CGROM
; Uses bc, bc', hl, ix
; hl: start of line in CGROM
; ix: start of line in VRAM
drawCharacterLine:
  ld bc, 0 ; character offset
  push ix ; put registers on stack for loop
  push hl

  exx ; alternate registers
  ld b, BytesPerLine ; we want to draw a full line of characters

characterLoop:
  exx ; normal registers
  pop hl ; get of start of line in VRAM
  pop ix ; start of line in CGROM
  push ix ; save registers for next loop
  push hl

  add hl, bc ; start of next character in VRAM line
  
  add ix, bc ; start of next character in CGROM (ix + bc * 8)
  add ix, bc
  add ix, bc
  add ix, bc
  add ix, bc
  add ix, bc
  add ix, bc
  add ix, bc
  call drawCharacter
  inc bc ; increment character offset
  exx ; alternate registers

  sbc hl, de ; check if we have shown all characters
  jr z, halt

  djnz characterLoop
  exx ; normal registers

  pop hl ; clean stack
  pop hl

  ret

;--------------------
; drawCharacter
; Uses an unrolled loop.
; Uses: a, de, hl
; hl: address of character in VRAM
; ix: address of character map (remains unchanged)
drawCharacter:
  ld de, BytesPerLine ; 80 for 640x200, 40 for 320x200
  ld a, (ix + 0) ; line 0 of character data
  ld (hl), a ; copy to VRAM
  add hl, de ; goto next scanline in VRAM
  ld a, (ix + 1)
  ld (hl), a
  add hl, de
  ld a, (ix + 2)
  ld (hl), a
  add hl, de
  ld a, (ix + 3)
  ld (hl), a
  add hl, de
  ld a, (ix + 4)
  ld (hl), a
  add hl, de
  ld a, (ix + 5)
  ld (hl), a
  add hl, de
  ld a, (ix + 6)
  ld (hl), a
  add hl, de
  ld a, (ix + 7)
  ld (hl), a
  add hl, de
  ret


.mapLine
  defb 000000001b
  defb 000000010b
  defb 000000100b
  defb 000001000b
  defb 000010000b
  defb 000100000b
  defb 001000000b
  defb 010000000b

  defs 256 ; room for stack
StackStart: