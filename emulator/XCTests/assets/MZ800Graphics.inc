;
; SHARP MZ-800 CRT Controller
;

;------------------------------------------
; Write format register for accessing VRAM

defc PortWriteFormatRegister = 0cch

; The following write formats need to be OR'ed with
; bits that select frame A/B and planes I/II/III/IV.
defc WriteFormatSingleWrite = 000000000b
defc WriteFormatXOR         = 000100000b
defc WriteFormatOR          = 001000000b
defc WriteFormatReset       = 001100000b
defc WriteFormatReplace     = 010000000b
defc WriteFormatPSET        = 011000000b

; Frame selection bit
defc FormatFrameSelectB = 000010000b

; Plane selection bits
defc FormatPlaneI   = 000000001b
defc FormatPlaneII  = 000000010b
defc FormatPlaneIII = 000000100b
defc FormatPlaneIV  = 000001000b

; The MZ-700 write format is to be used as is.
defc WriteFormatMZ700       = 000000001b

;------------------------------------------
; Read format register for accessing VRAM

; Use frame and plane selection bits from write format register
; to select frames and planes here, too.

defc PortReadFormatRegister = 0cdh

; Read/Search selection - set this bit to enable Search mode

defc ReadFormatSearch = 010000000b

;------------------------------------------
; Display mode

defc PortDisplayModeRegister = 0ceh

defc DisplayMode320x200_4ColorFrameA = 00000b
defc DisplayMode320x200_4ColorFrameB = 00001b
defc DisplayMode320x200_16Color      = 00010b

defc DisplayMode640x200_1ColorFrameA = 00100b
defc DisplayMode640x200_1ColorFrameB = 00101b
defc DisplayMode640x200_4Color       = 00110b

defc DisplayMode40x25_8ColorMZ700    = 01000b

;------------------------------------------
; Colors

defgroup {
  ColorBlack
  ColorBlue
  ColorRed
  ColorPurple
  ColorGreen
  ColorCyan
  ColorYellow
  ColorWhite

  ColorGray
  ColorLightBlue
  ColorLightRed
  ColorLightPurple
  ColorLightGreen
  ColorLightCyan
  ColorLightYellow
  ColorLightWhite
}

;------------------------------------------
; Border color

defc PortBorderColorHigh = 006h
defc PortBorderColorLow  = 0cfh

;------------------------------------------
; Palette register

defc PortPaletteRegister = 0f0h

; Palette codes
; For normal color modes
; Need to be OR'ed with color codes to set color
defc Palette0 = 00000000b
defc Palette1 = 00010000b
defc Palette2 = 00100000b
defc Palette3 = 00110000b

; Palette blocks
; For 16 color mode
defc PaletteBlock0 = 01000000b
defc PaletteBlock1 = 01000001b
defc PaletteBlock2 = 01000010b
defc PaletteBlock3 = 01000011b
