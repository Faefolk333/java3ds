#pragma once
#include "address_space.h"
#include <string>
namespace symbian {
struct KernelResult {
 enum class Kind {Unhandled,Returned,Fault,Blocked,Exited};
 Kind kind=Kind::Unhandled;std::uint32_t value=0,address=0;unsigned access=0;
};
// Initial EPOC6 executive model for one emulated thread. No host pointers or OS
// handles escape to the guest. Unsupported calls stop instead of inventing success.
class Eka1Kernel {
 AddressSpace&memory;
 std::uint32_t currentHeap=0;
 std::uint32_t trapHandler=0;
 std::array<std::uint32_t,64> trapFrames{};
 unsigned trapCount=0;
 std::uint32_t activeScheduler=0;
 struct TlsSlot {bool used=false;std::uint32_t key=0,value=0;};
 std::array<TlsSlot,64> tls{};
 std::u16string processFilename,commandLine;
 std::u16string threadName,processName;
 bool threadExited=false;std::uint32_t threadExitCode=0;
 std::uint32_t nextHandle=0x100;
 unsigned pendingRequests=0;
 struct Object {enum {Empty,Chunk,Mutex,Semaphore};unsigned type=0,references=0;std::uint32_t handle=0,base=0,maximum=0,size=0;int depth=0;bool global=false;std::u16string name;};
 std::array<Object,64> objects{};
 struct Alias {std::uint32_t handle=0;unsigned index=0;};
 std::array<Alias,64> aliases{};
 KernelResult executor(std::uint32_t attribute,std::uint32_t info,std::uint32_t status);
 KernelResult finish(std::uint32_t status,std::int32_t result);
 bool package(std::uint32_t descriptor,std::uint32_t length,bool writable,std::uint32_t&data);
 KernelResult text16(std::uint32_t descriptor,const std::u16string&text);
 bool readText16(std::uint32_t descriptor,std::u16string&text);
 Object* object(std::uint32_t handle,unsigned type);
 Object* newObject();
 std::uint32_t newAlias(Object&object);
 std::uint32_t chunkAddress(std::uint32_t size)const;
public:
 explicit Eka1Kernel(AddressSpace&m):memory(m){}
 KernelResult dispatch(std::uint32_t svc,const std::array<std::uint32_t,4>&args);
 std::uint32_t heap()const{return currentHeap;}
 std::uint32_t trap()const{return trapHandler;}
 unsigned trapDepth()const{return trapCount;}
 std::uint32_t scheduler()const{return activeScheduler;}
 bool configureProcess(const std::u16string&filename,const std::u16string&command);
 const std::u16string&name(bool process)const{return process?processName:threadName;}
 bool alive()const{return !threadExited;}
 std::uint32_t exitCode()const{return threadExitCode;}
 bool descriptor16(std::uint32_t at,std::u16string&text){return readText16(at,text);}
};
}
