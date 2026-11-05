#pragma once
// libctru's APT event handler runs at 0x31 (lower numbers run first).
// Both the VM entry worker and its Java/GC pthreads must be below APT and
// at the same priority, otherwise a spin waiter can starve its lock owner.
#define CRIMSON_VM_PRIORITY 0x34
