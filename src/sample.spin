CON
    _CLKMODE = XTAL1 | PLL16X
    _XINFREQ = 5_000_000
#define ABC
OBJ
    serial : "FullDuplexSerial"
VAR
    long x

PUB m1
    serial.start(31, 30, 0, 115200)
    x := 123

PUB m1
    if x => 0
        serial.dec(x)
    else
        serial.hex(-x, 8)
    serial.tx($0A)

PRI m3
    x := 11

PRI m4
    x := 12

DAT
entry
                mov x, #12      wz
    if_z        jmp #$

DAT
x               long 0

