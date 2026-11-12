#pragma once
#include "e32_image.h"
#include <memory>
namespace symbian {
enum class StopReason { Running,Returned,ImportRequired,SystemCallRequired,MemoryFault,BudgetExhausted,Cancelled,InvalidEntry,ThreadBlocked,ThreadExited };
const char*stopName(StopReason reason);
struct ExecutionResult {
 StopReason reason=StopReason::Running;
 std::uint32_t r0=0,pc=0,address=0,systemCall=0;
 unsigned access=0;
 std::uint64_t instructions=0;
 std::string library;
 std::string serviceName;
 std::uint32_t ordinal=0;
 std::array<std::uint32_t,4> arguments{};
 std::uint32_t kernelErrorCall=0,kernelErrorValue=0;
 std::array<std::uint32_t,4> kernelErrorArguments{};
 std::uint32_t fileCalls=0,fileOpens=0,fileReads=0,fileBytes=0;
 std::int32_t fileOperation=-1,fileResult=0;bool fileUnsupported=false;
 std::string filePath;
 std::uint32_t graphicsCalls=0,graphicsInitializations=0;std::int32_t graphicsOperation=-1;
 std::uint32_t windowCalls=0,windowInitializations=0,windowCommand=0,windowObject=0;std::int32_t windowOperation=-1;
 std::uint32_t screenDevices=0,screenQueries=0,screenWidth=0,screenHeight=0,screenBits=0,windowGroups=0;
};
struct HeapStartResult {
 bool ready=false;std::uint32_t address=0;ExecutionResult create,switchHeap;std::string error;
};
struct E32Library {std::string filename;std::uint32_t uid=0;LoadedE32 image;};
struct RomStartResult {
 bool ready=false;std::size_t planned=0,completed=0;std::uint32_t uid=0,entry=0;ExecutionResult call;std::string error;
};
struct CleanupStartResult {bool ready=false;std::uint32_t address=0,handler=0;ExecutionResult call;std::string error;};
struct NativeRuntimeResult {
 bool normal=false,leave=false,nested=false,cleanup=false,heapReuse=false;
 unsigned iterations=0;ExecutionResult call;std::string error;
 bool passed()const{return normal&&leave&&nested&&cleanup&&heapReuse;}
};
struct NativeContextResult {
 bool tls=false,scheduler=false,events=false,runError=false,teardown=false;
 ExecutionResult call;std::string error;
 bool passed()const{return tls&&scheduler&&events&&runError&&teardown;}
};
struct NativeFilesResult {
 bool connected=false,path=false,opened=false,read=false,seek=false,eof=false,missing=false,closed=false,rom=false,entry=false;
 ExecutionResult call;std::string error;
 bool passed()const{return connected&&path&&opened&&read&&seek&&eof&&missing&&closed&&rom&&entry;}
};
struct NativeGraphicsResult {
 bool connected=false,shared=false,retained=false,closed=false;
 ExecutionResult call;std::string error;
 bool passed()const{return connected&&shared&&retained&&closed;}
};
struct NativeWindowResult {
 bool connected=false,device=false,pixels=false,twips=false,color=false,group=false,name=false,closed=false;
 ExecutionResult call;std::string error;
 bool passed()const{return connected&&device&&pixels&&twips&&color&&group&&name&&closed;}
};
// One bounded E32 call, not a Symbian OS. Unimplemented imports/SVCs stop with
// their identities instead of returning fabricated success to an application.
class E32Session {
 struct Impl;std::unique_ptr<Impl> impl;
public:
 E32Session();~E32Session();
 bool load(const E32Image&image,std::uint32_t codeBase,std::uint32_t dataBase,std::string&error);
 bool mountRom(const char*path,std::string&error,TaskControl*control=nullptr);
 bool linkRomLibrary(const std::string&importName,const char*descriptor,std::size_t&resolved,std::string&error);
 bool romExport(const char*descriptor,std::uint32_t uid,std::uint32_t ordinal,std::uint32_t&target,std::string&error);
 // Resolve the entire main/game-DLL import graph atomically, before execution.
 // ROM headers live in rom-libs; owner-supplied E32 DLLs live in game-libs.
 bool linkDependencies(const char*folder,std::string&error,TaskControl*control=nullptr);
 const std::vector<E32Library>&libraries()const;
 const LoadedE32&module()const;
 AddressSpace&memory();
 void enableEka1Kernel();
 HeapStartResult initializeHeap(const char*euserDescriptor,TaskControl*control=nullptr);
 RomStartResult initializeRomLibraries(TaskControl*control=nullptr);
 CleanupStartResult initializeCleanup(const char*euserDescriptor,TaskControl*control=nullptr);
 NativeRuntimeResult checkNativeRuntime(const char*euserDescriptor,unsigned repetitions=16,TaskControl*control=nullptr);
 NativeContextResult checkNativeContext(const char*euserDescriptor,TaskControl*control=nullptr);
 NativeFilesResult checkNativeFiles(const char*efsrvDescriptor,const char*euserDescriptor,TaskControl*control=nullptr);
 NativeGraphicsResult checkNativeGraphics(const char*fbscliDescriptor,TaskControl*control=nullptr);
 NativeWindowResult checkNativeWindow(const char*ws32Descriptor,const char*fbscliDescriptor,const char*euserDescriptor,TaskControl*control=nullptr);
 ExecutionResult runRomProgram(const char*path,std::string&error,TaskControl*control=nullptr);
 ExecutionResult call(std::uint32_t entry,std::uint32_t argument=0,std::uint64_t budget=100000,TaskControl*control=nullptr);
 ExecutionResult callArgs(std::uint32_t entry,const std::array<std::uint32_t,4>&arguments,std::uint64_t budget=100000,TaskControl*control=nullptr);
};
}
