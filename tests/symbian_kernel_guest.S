.syntax unified
.arch armv4t
.text
.arm
.global kernel_heap_stub
kernel_heap_stub:
 svc #0x6c
 svc #0xeeeeee
.org 0x20
kernel_switch_stub:
 svc #0xc0006d
 svc #0xeeeeee
.org 0x40
kernel_hal_stub:
 svc #0x800083
 svc #0xeeeeee
.org 0x60
kernel_wait_stub:
 svc #0x2a
 svc #0xeeeeee
.org 0x80
kernel_executor_stub:
 svc #0xc00076
 svc #0xeeeeee
.macro CALL_STUB name
 push {r4,lr}
 ldr r4, =\name
 mov lr, pc
 bx r4
 pop {r4,lr}
 bx lr
 .ltorg
.endm
.org 0x100
 CALL_STUB kernel_heap_stub
.org 0x200
 CALL_STUB kernel_switch_stub
.org 0x300
 CALL_STUB kernel_hal_stub
.org 0x400
 CALL_STUB kernel_wait_stub
.org 0x500
 CALL_STUB kernel_executor_stub
.org 0x600
.thumb
.thumb_func
kernel_thumb:
 push {r4,lr}
 ldr r3, =kernel_heap_stub
 adr r4, thumb_return
 adds r4, #1
 mov lr, r4
 bx r3
.balign 4
thumb_return:
 pop {r4}
 pop {r1}
 bx r1
.ltorg
