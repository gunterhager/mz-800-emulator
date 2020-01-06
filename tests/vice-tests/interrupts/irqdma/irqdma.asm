; --- Defines

reference_data = $2000

zpdelaybmode   = $f7
zpd015         = $f8

zptimerstart   = $f5
zptimerstop    = $f6
zptimercurr    = $f9

zpdatalo       = $fa
zpdatahi       = $fb

zpfailslo      = $fc
zpfailshi      = $fd

; --- Code

*=$0801
basic:
; BASIC stub: "1 SYS 2061"
!by $0b,$08,$01,$00,$9e,$32,$30,$36,$31,$00,$00,$00

start:
    ldx #0
    stx zpfailslo
    stx zpfailshi
clp:
    lda #$20
    sta $0400,x
    sta $0500,x
    sta $0600,x
    sta $06e8,x
    lda #$01
    sta $d800+(24*40),x
    inx
    bne clp
    ldx #0
cl:
    lda testtxt,x
    beq sk
    sta $0400+(24*40),x
    inx
    bne cl
sk:
    jmp entrypoint

testtxt:
    !scr "irq"
    !byte $30+TESTNUM
    !if (B_MODE = 1) {
        !scr "b"
    }
    !byte 0

* = $0900
entrypoint:
    sei

    lda #4 ; start with 4 so we will see green (=5) on success
    sta $d020
    lda #$7f    ; disable timer irq
    sta $dc0d
    sta $dd0d
    lda #$00
    sta $dc0e
    sta $dc0f
    lda $dc0d
    lda $dd0d
    lda #$35
    sta $01
    lda #<irq_handler
    sta $fffe
    lda #>irq_handler
    sta $ffff
    lda #$1b
    sta $d011
    lda #$46
    sta $d012
    lda #$01    ; enable raster irq
    sta $d01a
    sta $d019
    lda #$64
    sta $d000
    sta $d002
    sta $d004
    sta $d006
    sta $d008
    sta $d00a
    sta $d00c
    sta $d00e
    lda #$4a
    sta $d001
    sta $d003
    sta $d005
    sta $d007
    sta $d009
    sta $d00b
    sta $d00d
    sta $d00f
    lda #$00
    sta $d010
    lda #$00
    sta zptimerstart     ; delay depending on CIA

!if B_MODE = 1 {
    lda #$40
    sta zpdelaybmode
    lda #$04
} else {
    lda #$00
}
    sta zpd015     ; d015 value

    lda #<reference_data
    sta zpdatalo
    ldy #$03
    jsr printhex
    lda #>reference_data
    sta zpdatahi
    ldy #$01
    jsr printhex
    cli
entry_loop:
    jmp entry_loop

;-------------------------------------------------------------

irq_handler:
    lda #<irq_handler_2
    sta $fffe
    lda #>irq_handler_2
    sta $ffff
    lda #$01
    sta $d019
    ldx $d012
    inx
    stx $d012
    cli
    ror $02
    ror $02
    ror $02
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    rti

irq_handler_2:
    lda #$01
    sta $d019
    ldx #$07
irq_handler_2_wait_1:
    dex
    bne irq_handler_2_wait_1
    nop
    nop
    lda $d012
    cmp $d012
    beq irq_handler_2_skip_1
irq_handler_2_skip_1:
    nop
    nop

    ; test the CIA type on first run
    lda zptimerstart     ; delay depending on CIA
    bne cia_ok
    jsr testcia
    jmp irq_handler_2_finish_test
cia_ok:

    ldx #$04
irq_handler_2_wait_2:
    dex
    bne irq_handler_2_wait_2

    inc $d021
    dec $d021
    lda #<irq_handler_3
    sta $fffe
    lda #>irq_handler_3
    sta $ffff
    lda #$46
    sta $d012
    cli

    lda zpd015     ; d015 value
    sta $d015
    lda #$ff
    sta $dd04
    lda #$00
    sta $dd05
    lda #$ff
    sta $dd06
    lda #$00
    sta $dd07
    lda zptimercurr     ; cia1 ta lo
    sta $dc04
    lda #$00
    sta $dc05
    lda #$81            ; enable timer irq
    sta $dc0d
    lda #$19
    sta $dd0e
    sta $dd0f
    sta $dc0e

!if B_MODE = 1 {
    lda zpdelaybmode
    jsr delay
}

!if TESTNUM = 1 {
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
}

!if TESTNUM = 2 {
    ldx #$00
    lda ($00,x)
    lda ($00,x)
    lda ($00,x)
    lda ($00,x)
    lda ($00,x)
    lda ($00,x)
    lda ($00,x)
    lda ($00,x)
    lda ($00,x)
    lda ($00,x)
    lda ($00,x)
    lda ($00,x)
    lda ($00,x)
    lda ($00,x)
    lda ($00,x)
    lda ($00,x)
    lda ($00,x)
    lda ($00,x)
    lda ($00,x)
}

!if TESTNUM = 3 {
    ldx #$00
    inc $ff00,x
    inc $ff00,x
    inc $ff00,x
    inc $ff00,x
    inc $ff00,x
    inc $ff00,x
    inc $ff00,x
    inc $ff00,x
    inc $ff00,x
    inc $ff00,x
    inc $ff00,x
    inc $ff00,x
    inc $ff00,x
    inc $ff00,x
    inc $ff00,x
    inc $ff00,x
    inc $ff00,x
    inc $ff00,x
    inc $ff00,x
}

!if TESTNUM = 4 {
    ldx #$00
    jsr dummy_rts
    jsr dummy_rts
    jsr dummy_rts
    jsr dummy_rts
    jsr dummy_rts
    jsr dummy_rts
    jsr dummy_rts
    jsr dummy_rts
    jsr dummy_rts
    jsr dummy_rts
    jsr dummy_rts
}

!if TESTNUM = 5 {
    ldx #$00
    beq lc113
lc113:
    beq lc115
lc115:
    beq lc117
lc117:
    beq lc119
lc119:
    beq lc11b
lc11b:
    beq lc11d
lc11d:
    beq lc11f
lc11f:
    beq lc121
lc121:
    beq lc123
lc123:
    beq lc125
lc125:
    beq lc127
lc127:
    beq lc129
lc129:
    beq lc12b
lc12b:
    beq lc12d
lc12d:
    beq lc12f
lc12f:
    beq lc131
lc131:
    beq lc133
lc133:
    beq lc135
lc135:
    beq lc137
lc137:
    beq lc139
lc139:
    beq lc13b
lc13b:
    beq lc13d
lc13d:
    beq lc13f
lc13f:
    beq lc141
lc141:
    beq lc143
lc143:
    beq lc145
lc145:
    beq lc147
lc147:
    beq lc149
lc149:
    beq lc14b
lc14b:
    beq lc14d
lc14d:
    beq lc14f
lc14f:
    beq lc151
lc151:
    beq lc153
lc153:
    beq lc155
lc155:
    beq lc157
lc157:
    beq lc159
lc159:
    beq lc15b
lc15b:
    beq lc15d
lc15d:
    beq lc15f
lc15f:
    beq lc161
lc161:
    beq lc163
lc163:
    beq lc165
lc165:
    beq lc167
lc167:
    beq lc169
lc169:
}

!if TESTNUM = 6 {
    ldx #$00
    sei ; i = 1

    php
    cli ; i = 0
    plp ; i = 1
    cli ; i = 0
    sei ; i = 1

    php
    cli ; i = 0
    plp ; i = 1
    cli ; i = 0
    sei ; i = 1

    php
    cli
    plp
    cli
    sei

    php
    cli
    plp
    cli
    sei

    php
    cli
    plp
    cli
    sei

    php
    cli
    plp
    cli
    sei

    php
    cli
    plp
    cli
    sei

    php
    cli ; i = 0
    plp ; i = 1
    cli ; i = 0
}

; if test 6 fails, but test 7 works, then PHP or PLP are broken

!if TESTNUM = 7 {
    ldx #$00
    sei ; i = 1
    cli ; i = 0
    sei ; i = 1
    cli ; i = 0
    sei ; i = 1
    cli ; i = 0
    sei ; i = 1
    cli ; i = 0
    sei ; i = 1
    cli
    sei
    cli
    sei
    cli
    sei
    cli
    sei
    cli
    sei
    cli
    sei
    cli
    sei
    cli
    sei
    cli
    sei
    cli
    sei
    cli ; i = 0
    sei ; i = 1
    cli ; i = 0
    sei ; i = 1
    cli ; i = 0
    sei ; i = 1
    cli ; i = 0
    sei ; i = 1
    cli ; i = 0
    sei ; i = 1
    cli ; i = 0
    sei ; i = 1
    cli ; i = 0
    sei ; i = 1
    cli ; i = 0
    sei ; i = 1
    cli ; i = 0
    sei ; i = 1
    cli ; i = 0
    sei ; i = 1
    cli ; i = 0
    sei ; i = 1
    cli ; i = 0
    sei ; i = 1
    cli ; i = 0
    sei ; i = 1
    cli ; i = 0
    nop
}

;------------------------------------------------------------------

    lda $dd06
    pha
    tya
    pha
    ldy #$00

    ; 1 = test, 0 = record
    lda #TEST_MODE
    cmp #$01
    beq irq_handler_2_test_mode

    ; store values
    pla
    sta (zpdatalo),y         ; $dd04 (timer a lo)
    iny
    pla
    sta (zpdatalo),y         ; $dd06 (timer b lo)
    jmp irq_handler_2_next

irq_handler_2_test_mode:

    ; compare values with reference data
    pla
    cmp (zpdatalo),y
    bne irq_handler_2_test_failed2
    iny
    pla
    cmp (zpdatalo),y
    bne irq_handler_2_test_failed
    jmp irq_handler_2_next

irq_handler_2_test_failed2:
    pla
irq_handler_2_test_failed:
    lda #2
    sta $d020
    sta irq_handler_failed_color+1
    jmp failed

irq_handler_2_next:
    ; advance to next
    lda zpdatalo
    clc
    adc #$02
    sta zpdatalo
    lda zpdatahi
    adc #$00
    sta zpdatahi

    ; show current addr
    ldy #$01
    jsr printhex
    lda zpdatalo
    ldy #$03
    jsr printhex

!if B_MODE = 1 {
    dec zpdelaybmode
    lda zpdelaybmode
    cmp #$38
    bne irq_handler_2_finish_test

    lda #$40
    sta zpdelaybmode
}

    inc zptimercurr     ; cia1 ta lo
    lda zptimercurr
    cmp zptimerstop     ; cia1 ta lo end value (+$80)
    bne irq_handler_2_finish_test

    lda zptimerstart     ; delay depending on CIA
    sta zptimercurr

    inc zpd015     ; d015 value
    lda zpd015
!if B_MODE = 1 {
    cmp #$14
} else {
    cmp #$80
}
    bne irq_handler_2_finish_test

    ; all tests done
irq_handler_failed_color:
    lda #5
    sta $d020
irq_handler_2_all_tests_successful:

    lda $d020
    and #$0f
    ldx #0 ; success
    cmp #5
    beq nofail
    ldx #$ff ; failure
nofail:
    stx $d7ff

    jmp irq_handler_2_all_tests_successful

irq_handler_2_finish_test:
    lda #$00
    sta $dc0e
    lda #$7f    ; disable timer irq
    sta $dc0d
    lda $dc0d
    lda #<irq_handler
    sta $fffe
    lda #>irq_handler
    sta $ffff
    rti

irq_handler_3:
    bit $dc0d
    ldy $dd04
    lda #$19
    sta $dd0f
    rti

!if TESTNUM = 4 {
dummy_rts:
    rts
}


testcia:

    lda #<irq_handler_testcia
    sta $fffe
    lda #>irq_handler_testcia
    sta $ffff
    lda #$46
    sta $d012
    cli
    lda #$2b
    sta $dd04
    lda #$00
    sta $dd05
    lda #$10
    sta $dc04
    lda #$00
    sta $dc05
    lda #$81    ; enable timer irq
    sta $dc0d
    lda #$19
    sta $dd0e
    sta $dd0f
    sta $dc0e

    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop

    rts

irq_handler_testcia:
    bit $dc0d
    ldy $dd04
    lda #$19
    sta $dd0f
    tya
    lsr
    clc
!if B_MODE = 1 {
    adc #$30
} else {
    adc #$10
}
    sta zptimerstart     ; delay depending on CIA
    sta zptimercurr     ; cia1 ta lo
    clc
    adc #$80
    sta zptimerstop
    
    tya
    lsr
    asl
    asl
    asl
    tay
    ldx #$00

loopoutput:    
    lda ciatype,y
    sta $0420,x
    lda #$01
    sta $d820,x
    iny
    inx
    cpx #$08
    bne loopoutput
    rti
    
ciatype:
    !scr "old cia "
    !scr "new cia "

printhex:
    pha
    ; mask lower
    and #$0f
    ; lookup
    tax
    lda hex_lut,x
    ; print
    sta $0400,y
    ; lsr x4
    pla
    lsr
    lsr
    lsr
    lsr
    ; lookup
    tax
    lda hex_lut,x
    ; print
    dey
    sta $0400,y
    rts

; hex lookup table
hex_lut: 
!scr "0123456789abcdef"

failed:
  cpy #$01
  bne failvalue
  inc $0403
failvalue:
  pha
  ; reference value
  lda (zpdatalo),y
  ldy #$09
  jsr printhex
  pla
  ; actual value
  ldy #$06
  jsr printhex
;failloop:
;  inc $d020
;  jmp failloop

  ; show current addr
  lda zpdatahi
  ldy #$01+40
  jsr printhex
  lda zpdatalo
  ldy #$03+40
  jsr printhex

  ; number of fails
  inc zpfailslo
  bne skfail
  inc zpfailshi
skfail:

  ldy #$10
  lda zpfailslo
  jsr printhex
  ldy #$0e
  lda zpfailshi
  jsr printhex

  jmp irq_handler_2_next

!if B_MODE = 1 {
* = $0850
delay:              ;delay 80-accu cycles, 0<=accu<=64
    lsr             ;2 cycles akku=akku/2 carry=1 if accu was odd, 0 otherwise
    bcc waste1cycle ;2/3 cycles, depending on lowest bit, same operation for both
waste1cycle:
    sta smod+1      ;4 cycles selfmodifies the argument of branch
    clc             ;2 cycles
;now we have burned 10/11 cycles.. and jumping into a nopfield 
smod:
    bcc *+10
!by $EA,$EA,$EA,$EA,$EA,$EA,$EA,$EA
!by $EA,$EA,$EA,$EA,$EA,$EA,$EA,$EA
!by $EA,$EA,$EA,$EA,$EA,$EA,$EA,$EA
!by $EA,$EA,$EA,$EA,$EA,$EA,$EA,$EA
    rts             ;6 cycles
}

!if TEST_MODE = 1 {
    * = $2000
    !if B_MODE = 0 {
        !if TESTNUM = 1 {
            !bin "dumps/irq1.dump",,2
        }
        !if TESTNUM = 2 {
            !bin "dumps/irq2.dump",,2
        }
        !if TESTNUM = 3 {
            !bin "dumps/irq3.dump",,2
        }
        !if TESTNUM = 4 {
            !bin "dumps/irq4.dump",,2
        }
        !if TESTNUM = 5 {
            !bin "dumps/irq5.dump",,2
        }
        !if TESTNUM = 6 {
            !bin "dumps/irq6.dump",,2
        }
        !if TESTNUM = 7 {
            !bin "dumps/irq7.dump",,2
        }
    }
    !if B_MODE = 1 {
        !if TESTNUM = 1 {
            !bin "dumps/irq1b.dump",,2
        }
        !if TESTNUM = 2 {
            !bin "dumps/irq2b.dump",,2
        }
        !if TESTNUM = 3 {
            !bin "dumps/irq3b.dump",,2
        }
        !if TESTNUM = 4 {
            !bin "dumps/irq4b.dump",,2
        }
        !if TESTNUM = 5 {
            !bin "dumps/irq5b.dump",,2
        }
        !if TESTNUM = 6 {
            !bin "dumps/irq6b.dump",,2
        }
        !if TESTNUM = 7 {
            !bin "dumps/irq7b.dump",,2
        }
    }
}
