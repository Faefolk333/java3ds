#include "jvm.h"
#include "jvm_util.h"
#include "bytebuf.h"
#include <assert.h>
#include <string.h>
extern Utf8String*__real_utf8_create(void);extern void*__real_calloc(size_t,size_t);extern void*__real_malloc(size_t);
static int failString,failStruct,failGrow,vmFault;
extern void*_parseCPString(JClass*,ByteBuf*,s32);
void crimson_arm_cp_failure(void){vmFault=1;}
Utf8String*__wrap_utf8_create(void){uintptr_t caller=(uintptr_t)__builtin_return_address(0),parser=(uintptr_t)_parseCPString;if(vmFault&&caller>=parser&&caller<parser+1024){vmFault=0;puts("CONSTANT_POOL_VM_FAULT injected");return NULL;}if(failString){failString=0;return NULL;}return __real_utf8_create();}
void*__wrap_calloc(size_t n,size_t z){if(failStruct&&n*z==sizeof(ConstantUTF8)){failStruct=0;return NULL;}return __real_calloc(n,z);}
void*__wrap_malloc(size_t n){if(failGrow&&n==128){failGrow=0;return NULL;}return __real_malloc(n);}
int crimson_constant_pool_test(void){
 unsigned char bytes[83]={0xca,0xfe,0xba,0xbe,0,0,0,52,0,2,1,0,70};memset(bytes+13,'C',70);
 for(int test=0;test<3;test++){JClass c={0};c.constantPool.utf8CP=arraylist_create(0);ByteBuf*b=bytebuf_create(83);bytebuf_write_batch(b,(char*)bytes,83);failString=test==0;failStruct=test==1;failGrow=test==2;assert(_LOAD_CLASS_FROM_BYTES(&c,b)!=0);assert(!failString&&!failStruct&&!failGrow);assert(c.constantPool.utf8CP->length==0);jvm_free(c.constant_item_ptr);arraylist_destroy(c.constantPool.utf8CP);bytebuf_destroy(b);}
 puts("CONSTANT_POOL_OOM_PASS NULL_string NULL_record failed_growth propagated_parse_error");return 0;
}
