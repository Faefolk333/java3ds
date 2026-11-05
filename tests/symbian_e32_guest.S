.syntax unified
.arch armv4t
.text
.arm
.global e32_entry
e32_entry:
 push {r4,lr}
 ldr r4, data_pointer
 ldr r0, [r4]
 add r0, r0, #1
 str r0, [r4, #8]
 pop {r4,lr}
 bx lr
.org 0x40
 push {r4,lr}
 ldr ip, iat_pointer
 ldr ip, [ip]
 mov lr, pc
 bx ip
 pop {r4,lr}
 bx lr
.org 0x60
 svc #0xc00000
 bx lr
.org 0x68
 ldr r1, code_pointer
 str r0, [r1]
 bx lr
.org 0x78
1: add r0, r0, #1
 b 1b
.org 0x80
data_pointer: .word 0x00400000
iat_pointer: .word 0x100000c0
code_pointer: .word 0x10000000
 .word 0x00400008
.org 0xa0
 .word 0,0x40,0x60,0x68,0x78
.org 0xc0
 .word 7,0
