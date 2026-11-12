#pragma once
#include "e32_image.h"
#include <memory>
namespace symbian {
enum class StopReason { Running,Returned,ImportRequired,SystemCallRequired,MemoryFault,BudgetExhausted,Cancelled,InvalidEntry,ThreadBlocked };
const char*stopName(StopReason reason);
struct ExecutionResult {
 StopReason reason=StopReason::Running;
 std::uint32_t r0=0,pc=0,address=0,systemCall=0;
 unsigned access=0;
 std::uint64_t instructions=0;
 std::string library;
 std::uint32_t ordinal=0;
 std::array<std::uint32_t,4> arguments{};
};
struct HeapStartResult {
 bool ready=false;std::uint32_t address=0;ExecutionResult create,switchHeap;std::string error;
};
struct E32Library {std::string filename;std::uint32_t uid=0;LoadedE32 image;};
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
 ExecutionResult call(std::uint32_t entry,std::uint32_t argument=0,std::uint64_t budget=100000,TaskControl*control=nullptr);
 ExecutionResult callArgs(std::uint32_t entry,const std::array<std::uint32_t,4>&arguments,std::uint64_t budget=100000,TaskControl*control=nullptr);
};
}
