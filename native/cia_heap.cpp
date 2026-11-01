// The startup hook runs before constructors and before any service connection.
#include <3ds.h>
extern "C" {
extern u32 __ctru_heap_size;
extern u32 __ctru_linear_heap_size;
void __real___system_allocateHeaps(void);
void __wrap___system_allocateHeaps(void){
 Handle limit=0;s64 maximum=0,used=0;ResourceLimitType kind=RESLIMIT_COMMIT;
 Result rc=svcGetResourceLimit(&limit,CUR_PROCESS_HANDLE);
 if(R_SUCCEEDED(rc)){
  rc=svcGetResourceLimitLimitValues(&maximum,limit,&kind,1);
  if(R_SUCCEEDED(rc))rc=svcGetResourceLimitCurrentValues(&used,limit,&kind,1);
  svcCloseHandle(limit);
 }
 // Keep a bounded regular heap. Consuming all but 4 MiB can fail at startup
 // with the 124 MiB New 3DS profile (the supplied dumps fail at this SVC).
 if(R_SUCCEEDED(rc)&&maximum>used+8*1024*1024){
  u32 remaining=(u32)(maximum-used)&~0xfffu;
  u32 cap=maximum>64*1024*1024?80*1024*1024:48*1024*1024;
  __ctru_linear_heap_size=4*1024*1024;
  u32 available=remaining-6*1024*1024;
  __ctru_heap_size=available<cap?available:cap;
 }else{
  // Restore libctru's default split if resource information is unavailable.
  __ctru_heap_size=0;__ctru_linear_heap_size=0;
 }
 __real___system_allocateHeaps();
}
}
