#include "loader_probe.h"
#include "e32_session.h"
#include "e32_guest.h"
SymbianLoaderResult symbianLoaderProbe(){
 using namespace symbian;SymbianLoaderResult result;std::string error;
 E32Image image;result.parsed=image.parse({std::begin(symbian_e32_guest),std::end(symbian_e32_guest)},error);
 if(!result.parsed)return result;
 E32Session session;if(!session.load(image,0x12000000,0x02000000,error))return result;
 const auto&module=session.module();auto&memory=session.memory();
 std::uint32_t pointer=0,bss=1;memory.read(0x12000080,&pointer);memory.read(0x02000008,&bss);
 result.relocated=pointer==0x02000000&&bss==0&&module.exports.size()==5&&module.exports[0]==module.entry;
 auto r=session.call(module.entry);memory.read(0x02000008,&bss);
 result.returned=r.reason==StopReason::Returned&&r.r0==42&&bss==42;
 r=session.call(module.exports[1]);result.imports=r.reason==StopReason::ImportRequired&&r.library=="PROBE[10000001].DLL"&&r.ordinal==7;
 r=session.call(module.exports[2]);result.systemCall=r.reason==StopReason::SystemCallRequired&&r.systemCall==0xc00000;
 std::uint32_t before=0,after=0;memory.read(module.codeBase,&before);
 r=session.call(module.exports[3]);memory.read(module.codeBase,&after);
 result.protectedMemory=r.reason==StopReason::MemoryFault&&r.access==Write&&before==after&&
  session.call(0x02000000).reason==StopReason::InvalidEntry&&session.call(0).reason==StopReason::InvalidEntry;
 r=session.call(module.exports[4],0,5000);result.bounded=r.reason==StopReason::BudgetExhausted&&r.instructions>=5000&&r.instructions<5100;
 TaskControl stop([](){return false;});r=session.call(module.exports[4],0,500000,&stop);result.cancelled=r.reason==StopReason::Cancelled&&r.instructions==0&&stop.cancelled();
 return result;
}
