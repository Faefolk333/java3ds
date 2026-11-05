.syntax unified
.arch armv6k
.fpu vfp
.text
.arm
.global probe_arm
probe_arm:
 mov r0, #10
 mov r1, #0
1: add r1, r1, r0
 subs r0, r0, #1
 bne 1b
 str r1, [r2]
 ldrb r3, [r2]
 svc #0x42
 b .
.org 0x100
.thumb
.global probe_thumb
.thumb_func
probe_thumb:
 movs r0, #5
 movs r1, #2
 muls r0, r1, r0
 str r0, [r2]
 svc #0x43
 b .
.org 0x200
.arm
.global probe_exclusive
probe_exclusive:
 ldrex r4, [r2]
 add r4, r4, #1
 strex r3, r4, [r2]
 svc #0x44
 b .
.org 0x300
.global probe_float
probe_float:
 mov r0, #0x40000000
 vmsr fpexc, r0
 vadd.f32 s2, s0, s1
 svc #0x45
 b .
.org 0x400
.global probe_spin
probe_spin:
 add r5, r5, #1
 b probe_spin
.org 0x500
.global probe_double
probe_double:
 mov r0, #0x40000000
 vmsr fpexc, r0
 vadd.f64 d2, d0, d1
 svc #0x46
 b .
