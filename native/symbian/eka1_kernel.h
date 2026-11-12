#pragma once
#include "address_space.h"
namespace symbian {
struct KernelResult {
 enum class Kind {Unhandled,Returned,Fault,Blocked};
 Kind kind=Kind::Unhandled;std::uint32_t value=0,address=0;unsigned access=0;
};
// Initial EPOC6 executive model for one emulated thread. No host pointers or OS
// handles escape to the guest. Unsupported calls stop instead of inventing success.
class Eka1Kernel {
 AddressSpace&memory;
 std::uint32_t currentHeap=0;
 std::uint32_t nextHandle=0x100;
 unsigned pendingRequests=0;
 struct Object {enum {Empty,Chunk,Mutex,Semaphore};unsigned type=0;std::uint32_t handle=0,base=0,maximum=0,size=0;int depth=0;};
 std::array<Object,64> objects{};
 KernelResult executor(std::uint32_t attribute,std::uint32_t info,std::uint32_t status);
 KernelResult finish(std::uint32_t status,std::int32_t result);
 bool package(std::uint32_t descriptor,std::uint32_t length,bool writable,std::uint32_t&data);
 Object* object(std::uint32_t handle,unsigned type);
 Object* newObject();
 std::uint32_t chunkAddress(std::uint32_t size)const;
public:
 explicit Eka1Kernel(AddressSpace&m):memory(m){}
 KernelResult dispatch(std::uint32_t svc,const std::array<std::uint32_t,4>&args);
 std::uint32_t heap()const{return currentHeap;}
};
}
