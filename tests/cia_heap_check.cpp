#include <3ds.h>
#include <cassert>
extern "C" {u32 __ctru_heap_size=0,__ctru_linear_heap_size=0;int calls=0;void __real___system_allocateHeaps(){
 calls++;
 // The installed SDK does not clamp an explicitly split heap to its 96 MiB
 // address area. Check the actual request passed to it, not just the policy.
 assert(__ctru_heap_size<=96*1024*1024);
 if(__ctru_heap_size||__ctru_linear_heap_size)assert(__ctru_heap_size+__ctru_linear_heap_size<=heapfake::maximum-heapfake::used);
}void __wrap___system_allocateHeaps();}
int main(){const u32 M=1024*1024;
 heapfake::maximum=64*M;heapfake::used=5*M;__wrap___system_allocateHeaps();assert(__ctru_heap_size==48*M&&__ctru_linear_heap_size==4*M);
 heapfake::maximum=124*M;__wrap___system_allocateHeaps();assert(__ctru_heap_size==80*M&&__ctru_linear_heap_size==4*M);
 // Dump 31: a 124 MiB HBL process, 0x69000 already committed, fixed 2 MiB
 // linear heap. The unwrapped SDK requests 0x7997000 regular bytes (>96 MiB).
 heapfake::used=0x69000;__ctru_heap_size=0;__ctru_linear_heap_size=2*M;
 assert(((heapfake::maximum-heapfake::used)&~0xfffLL)-__ctru_linear_heap_size==0x7997000);
 __wrap___system_allocateHeaps();assert(__ctru_heap_size==80*M&&__ctru_linear_heap_size==4*M);
 heapfake::used=5*M;
 heapfake::maximum=54*M;__wrap___system_allocateHeaps();assert(__ctru_heap_size+__ctru_linear_heap_size<=(54-5)*M);
 heapfake::fail=true;__wrap___system_allocateHeaps();assert(__ctru_heap_size==0&&__ctru_linear_heap_size==0&&calls==5);
 puts("STARTUP_HEAP_PASS Old64 New124 dump31_request bounded_SDK_request tight_limit fallback");
}
