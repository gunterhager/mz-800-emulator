;
; SHARP MZ-800 Memory
;

;------------------------------------------
; Memory map

defc MemoryMonitorROM1Start  = 00000h
defc MemoryMonitorROM1End    = 00fffh

defc MemoryMonitorCGROMStart = 01000h ; Character ROM
defc MemoryMonitorCGROMEnd   = 01fffh

defc MemoryVRAMStart         = 08000h
defc MemoryVRAMShortEnd      = 09fffh ; 320x200
defc MemoryVRAMLongEnd       = 0bfffh ; 640x200

defc MemoryMonitorROM2Start  = 0e000h
defc MemoryMonitorROM2End    = 0ffffh

;------------------------------------------
; Memory bank ports

; Ports that need to be read to switch bank

defc PortInBank0 = 0e0h ; -    | CGROM | VRAM | -
defc PortInBank1 = 0e1h ; -    | DRAM  | DRAM | -

; Ports that need to be written to switch bank

defc PortBank0 = 0e0h ; DRAM | DRAM  | -    | -
defc PortBank1 = 0e1h ; -    | -     | -    | DRAM
defc PortBank2 = 0e2h ; ROM1 | -     | -    | -
defc PortBank3 = 0e3h ; -    | -     | -    | ROM2
defc PortBank4 = 0e4h ; ROM1 | CGROM | VRAM | ROM2

defc PortBank_DRAM_DRAM_X_X        = 0e0h
defc PortBank_X_X_X_DRAM           = 0e1h
defc PortBank_ROM1_X_X_X           = 0e2h
defc PortBank_X_X_X_ROM2           = 0e3h
defc PortBank_ROM1_CGROM_VRAM_ROM2 = 0e4h

defc PortBankProhibitOn = 0e5h
defc PortBankProhibitOff = 0e6h
