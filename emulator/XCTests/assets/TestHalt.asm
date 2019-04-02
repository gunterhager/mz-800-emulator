; Test: HALT
;
; Use z80asm from the z88dk project to build binary file.
; Command line:
; z80asm -b TestHalt.asm

org 02000h
	halt
	