#include <3ds.h>
#include <cassert>
extern "C" {u32 __ctru_heap_size=0,__ctru_linear_heap_size=0;int calls=0;void __real___system_allocateHeaps(){calls++;}void __wrap___system_allocateHeaps();}
int main(){const u32 M=1024*1024;
 heapfake::maximum=64*M;heapfake::used=5*M;__wrap___system_allocateHeaps();assert(__ctru_heap_size==48*M&&__ctru_linear_heap_size==4*M);
 heapfake::maximum=124*M;__wrap___system_allocateHeaps();assert(__ctru_heap_size==80*M&&__ctru_linear_heap_size==4*M);
 heapfake::maximum=54*M;__wrap___system_allocateHeaps();assert(__ctru_heap_size+__ctru_linear_heap_size<=(54-5)*M);
 heapfake::fail=true;__wrap___system_allocateHeaps();assert(__ctru_heap_size==0&&__ctru_linear_heap_size==0&&calls==4);
 puts("CIA_HEAP_PASS Old64 New124 tight-limit fallback");
}
