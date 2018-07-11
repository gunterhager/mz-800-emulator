         *= $0801         .BYTE $4C,$16,$08,$00,$97,$32         .BYTE $2C,$30,$3A,$9E,$32,$30         .BYTE $37,$30,$00,$00,$00,$A9         .BYTE $01,$85,$02         JSR PRINT         .BYTE 13         .TEXT "�ASLAX"         .BYTE 0         LDA #%00011011         STA DB         LDA #%11000110         STA AB         LDA #%10110001         STA XB         LDA #%01101100         STA YB         LDA #0         STA PB         TSX         STX SB         LDA #0         STA DB         STA XBNEXT     LDA DB         STA DA         STA DR         ASL DR         LDA AB         STA AR         LDA XB         STA XR         LDA YB         STA YR         LDA PB         ORA #%00110000         AND #%01111100         TAX         LDA DR         CMP #0         BNE NOZERO         TXA         ORA #%00000010         TAX         LDA DRNOZERO   ASL A         BCC NONEG         TXA         ORA #%10000000         TAXNONEG    LDA DB         BPL NOCARRY         TXA         ORA #%00000001         TAXNOCARRY  STX PR         LDA SB         STA SR         LDX SB         TXS         LDA PB         PHA         LDA AB         LDX XB         LDY YB         PLPCMD      ASL DA,X         PHP         CLD         STA AA         STX XA         STY YA         PLA         STA PA         TSX         STX SA         JSR CHECK         INC CMD+1         BNE NOINC         INC CMD+2NOINC    LDA XB         BNE NODEC         DEC CMD+2NODEC    DEC XB         INC DB         BNE JMPNEXT         INC PB         BEQ NONEXTJMPNEXT  JMP NEXTNONEXT         JSR PRINT         .TEXT " - OK"         .BYTE 13,0         LDA 2         BEQ LOADWAIT     JSR $FFE4         BEQ WAIT         JMP $8000LOAD     JSR PRINTNAME     .TEXT "LSRN"NAMELEN  = *-NAME         .BYTE 0         LDA #0         STA $0A         STA $B9         LDA #NAMELEN         STA $B7         LDA #<NAME         STA $BB         LDA #>NAME         STA $BC         PLA         PLA         JMP $E16FDB       .BYTE 0AB       .BYTE 0XB       .BYTE 0YB       .BYTE 0PB       .BYTE 0SB       .BYTE 0DA       .BYTE 0AA       .BYTE 0XA       .BYTE 0YA       .BYTE 0PA       .BYTE 0SA       .BYTE 0DR       .BYTE 0AR       .BYTE 0XR       .BYTE 0YR       .BYTE 0PR       .BYTE 0SR       .BYTE 0CHECK         .BLOCK         LDA DA         CMP DR         BNE ERROR         LDA AA         CMP AR         BNE ERROR         LDA XA         CMP XR         BNE ERROR         LDA YA         CMP YR         BNE ERROR         LDA PA         CMP PR         BNE ERROR         LDA SA         CMP SR         BNE ERROR         RTSERROR    JSR PRINT         .BYTE 13         .NULL "BEFORE  "         LDX #<DB         LDY #>DB         JSR SHOWREGS         JSR PRINT         .BYTE 13         .NULL "AFTER   "         LDX #<DA         LDY #>DA         JSR SHOWREGS         JSR PRINT         .BYTE 13         .NULL "RIGHT   "         LDX #<DR         LDY #>DR         JSR SHOWREGS         LDA #13         JSR $FFD2WAIT     JSR $FFE4         BEQ WAIT         CMP #3         BEQ STOP         RTSSTOP     LDA 2         BEQ BASIC         JMP $8000BASIC    JMP ($A002)SHOWREGS STX 172         STY 173         LDY #0         LDA (172),Y         JSR HEXB         LDA #32         JSR $FFD2         LDA #32         JSR $FFD2         INY         LDA (172),Y         JSR HEXB         LDA #32         JSR $FFD2         INY         LDA (172),Y         JSR HEXB         LDA #32         JSR $FFD2         INY         LDA (172),Y         JSR HEXB         LDA #32         JSR $FFD2         INY         LDA (172),Y         LDX #"N"         ASL A         BCC OK7         LDX #"�"OK7      PHA         TXA         JSR $FFD2         PLA         LDX #"V"         ASL A         BCC OK6         LDX #"�"OK6      PHA         TXA         JSR $FFD2         PLA         LDX #"0"         ASL A         BCC OK5         LDX #"1"OK5      PHA         TXA         JSR $FFD2         PLA         LDX #"B"         ASL A         BCC OK4         LDX #"�"OK4      PHA         TXA         JSR $FFD2         PLA         LDX #"D"         ASL A         BCC OK3         LDX #"�"OK3      PHA         TXA         JSR $FFD2         PLA         LDX #"I"         ASL A         BCC OK2         LDX #"�"OK2      PHA         TXA         JSR $FFD2         PLA         LDX #"Z"         ASL A         BCC OK1         LDX #"�"OK1      PHA         TXA         JSR $FFD2         PLA         LDX #"C"         ASL A         BCC OK0         LDX #"�"OK0      PHA         TXA         JSR $FFD2         PLA         LDA #32         JSR $FFD2         INY         LDA (172),Y         .BENDHEXB     PHA         LSR A         LSR A         LSR A         LSR A         JSR HEXN         PLA         AND #$0FHEXN     ORA #$30         CMP #$3A         BCC HEXN0         ADC #6HEXN0    JMP $FFD2PRINT    PLA         .BLOCK         STA PRINT0+1         PLA         STA PRINT0+2         LDX #1PRINT0   LDA !*,X         BEQ PRINT1         JSR $FFD2         INX         BNE PRINT0PRINT1   SEC         TXA         ADC PRINT0+1         STA PRINT2+1         LDA #0         ADC PRINT0+2         STA PRINT2+2PRINT2   JMP !*         .BEND