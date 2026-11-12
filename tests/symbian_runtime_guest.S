.syntax unified
.arch armv4t
.text
.arm
// Original diagnostic only. Function addresses are supplied at run time from
// UID-checked EUSER exports, never copied from a phone ROM.
.macro INVOKE offset
 ldr ip, [r4, #\offset]
 mov lr, pc
 bx ip
.endm
.global native_trap_check
native_trap_check:
 push {r4-r11,ip,lr}
 sub sp, sp, #96
 mov r4, r0
 ldr r5, [r4, #36]
 ldr r6, [r4, #32]
 mov r7, #123
 mov r0, sp
 add r1, sp, #76
 INVOKE 0
 cmp r0, #0
 bne trap_caught
 ldr r0, [r5, #8]
 add r0, r0, #1
 str r0, [r5, #8]
 mov r0, #256
 INVOKE 20
 str r0, [r5, #20]
 INVOKE 28
 ldr r0, =native_cleanup_callback
 add r1, r5, #24
 INVOKE 12
 cmp r6, #0
 beq normal_path
 mov r7, #99
 mvn r0, #41
 INVOKE 8
 mov r0, #1
 str r0, [r5, #12]
 b trap_finish
normal_path:
 INVOKE 16
 INVOKE 16
 mov r0, sp
 INVOKE 4
 b trap_finish
trap_caught:
 str r7, [r5, #16]
trap_finish:
 ldr r0, [sp, #76]
 str r0, [r5]
 add sp, sp, #96
 pop {r4-r11,ip,lr}
 bx lr
 .ltorg
.org 0x180
native_cleanup_callback:
 ldr r1, [r0]
 ldr r2, [r0, #4]
 ldr r3, [r1, #4]
 add r3, r3, #1
 str r3, [r1, #4]
 ldr r3, [r1, #40]
 add r3, r3, r3, lsl #2
 add r3, r2, r3, lsl #1
 str r3, [r1, #40]
 bx lr
.org 0x200
.global native_nested_check
native_nested_check:
 push {r4-r11,ip,lr}
 sub sp, sp, #160
 mov r4, r0
 ldr r5, [r4, #36]
 mov r0, sp
 add r1, sp, #76
 INVOKE 0
 cmp r0, #0
 bne outer_caught
 ldr r0, =native_cleanup_callback
 add r1, r5, #24
 INVOKE 12
 add r0, sp, #80
 add r1, sp, #156
 INVOKE 0
 cmp r0, #0
 bne inner_caught
 ldr r0, =native_cleanup_callback
 add r1, r5, #32
 INVOKE 12
 mvn r0, #42
 INVOKE 8
 mov r0, #1
 str r0, [r5, #12]
 b nested_finish
inner_caught:
 ldr r0, [sp, #156]
 str r0, [r5, #44]
 mvn r0, #41
 INVOKE 8
 mov r0, #1
 str r0, [r5, #12]
 b nested_finish
outer_caught:
 ldr r0, [sp, #76]
 str r0, [r5]
nested_finish:
 ldr r0, [r5]
 add sp, sp, #160
 pop {r4-r11,ip,lr}
 bx lr
 .ltorg
.org 0x400
.global native_active_run
native_active_run:
 ldr r1, [r0, #24]
 ldr r2, [r0, #28]
 ldr r3, [r1]
 add r3, r3, #1
 str r3, [r1]
 ldr r3, [r1, #4]
 add r3, r3, r3, lsl #2
 add r3, r2, r3, lsl #1
 str r3, [r1, #4]
 ldr r2, [r0, #36]
 cmp r2, #0
 bxeq lr
 ldr ip, [r0, #32]
 mov r0, r2
 bx ip
.org 0x480
native_active_error:
 ldr r2, [r0, #24]
 str r1, [r2, #8]
 ldr r3, [r2, #12]
 add r3, r3, #1
 str r3, [r2, #12]
 mov r0, #0
 bx lr
.org 0x4c0
native_active_cancel:
 bx lr
