#pragma once
#include "e32_image.h"
#include <memory>
namespace symbian {
enum class StopReason { Running,Returned,ImportRequired,SystemCallRequired,MemoryFault,BudgetExhausted,Cancelled,InvalidEntry };
const char*stopName(StopReason reason);
struct ExecutionResult {
 StopReason reason=StopReason::Running;
 std::uint32_t r0=0,pc=0,address=0,systemCall=0;
 unsigned access=0;
 std::uint64_t instructions=0;
 std::string library;
 std::uint32_t ordinal=0;
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
 const LoadedE32&module()const;
 AddressSpace&memory();
 ExecutionResult call(std::uint32_t entry,std::uint32_t argument=0,std::uint64_t budget=100000,TaskControl*control=nullptr);
};
}
