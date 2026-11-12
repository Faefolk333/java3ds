#include "e32_session.h"
#include "rom_image.h"
#include "eka1_kernel.h"
#include <cpu/dyncom/arm_dyncom.h>
#include <cpu/12l1r/exclusive_monitor.h>
#include <algorithm>
#include <functional>
#include <map>
namespace symbian {
namespace {
constexpr std::uint32_t importBase=0x6f000000,returnBase=0x6fff0000,stackBase=0x70000000;
constexpr std::uint32_t returnSvc=0x7fffff,importSvc=0x700000;
}
struct E32Session::Impl {
 AddressSpace memory;
 Eka1Kernel kernel{memory};bool kernelEnabled=false;
 eka2l1::arm::r12l1::exclusive_monitor monitor{1};
 eka2l1::arm::dyncom_core cpu{&monitor,12};
 LoadedE32 image;
 std::vector<E32Import> traps;
 std::vector<E32Library> libraries;
 bool dependenciesAttempted=false;
 RomImage rom;
 ExecutionResult result;
 std::uint32_t stackSize=0;
 bool loaded=false;
 bool heapAttempted=false;std::uint32_t heapMin=0,heapMax=0;
 void fault(std::uint32_t address,unsigned access){
  if(result.reason==StopReason::Running){result.reason=StopReason::MemoryFault;result.address=address;result.access=access;}
  cpu.stop();
 }
 template<class T>bool read(std::uint32_t a,T*out){*out=0;if(memory.read(a,out))return true;fault(a,Read);return false;}
 template<class T>bool write(std::uint32_t a,const T*in){if(memory.write(a,in))return true;fault(a,Write);return false;}
 template<class T>std::int32_t exclusive(std::uint32_t a,T value,T expected){T previous;if(!read(a,&previous))return -1;if(previous!=expected)return 0;return write(a,&value)?1:-1;}
 Impl(){
  cpu.read_8bit=[this](auto a,auto p){return read(a,p);};cpu.read_16bit=[this](auto a,auto p){return read(a,p);};cpu.read_32bit=[this](auto a,auto p){return read(a,p);};cpu.read_64bit=[this](auto a,auto p){return read(a,p);};
  cpu.read_code=[this](auto a,auto p){*p=0;if(memory.read(a,p,sizeof(*p),Read|Execute))return true;fault(a,Execute);return false;};
  cpu.write_8bit=[this](auto a,auto p){return write(a,p);};cpu.write_16bit=[this](auto a,auto p){return write(a,p);};cpu.write_32bit=[this](auto a,auto p){return write(a,p);};cpu.write_64bit=[this](auto a,auto p){return write(a,p);};
  cpu.exclusive_write_8bit=[this](auto a,auto v,auto e){return exclusive(a,v,e);};cpu.exclusive_write_16bit=[this](auto a,auto v,auto e){return exclusive(a,v,e);};cpu.exclusive_write_32bit=[this](auto a,auto v,auto e){return exclusive(a,v,e);};cpu.exclusive_write_64bit=[this](auto a,auto v,auto e){return exclusive(a,v,e);};
  monitor.read_8bit=[this](auto,auto a,auto p){return read(a,p);};monitor.read_16bit=[this](auto,auto a,auto p){return read(a,p);};monitor.read_32bit=[this](auto,auto a,auto p){return read(a,p);};monitor.read_64bit=[this](auto,auto a,auto p){return read(a,p);};
  monitor.write_8bit=[this](auto,auto a,auto v,auto e){return exclusive(a,v,e);};monitor.write_16bit=[this](auto,auto a,auto v,auto e){return exclusive(a,v,e);};monitor.write_32bit=[this](auto,auto a,auto v,auto e){return exclusive(a,v,e);};monitor.write_64bit=[this](auto,auto a,auto v,auto e){return exclusive(a,v,e);};
  cpu.exception_handler=[this](auto,auto a){fault(a,Read);return false;};
  cpu.system_call_handler=[this](std::uint32_t svc){
   if(result.reason!=StopReason::Running){cpu.stop();return;}
   auto pc=cpu.get_pc();result.systemCall=svc;
   for(unsigned n=0;n<4;n++)result.arguments[n]=cpu.get_reg(n);
   if(pc==returnBase+4&&svc==returnSvc)result.reason=StopReason::Returned;
   else if(pc>=importBase+4&&pc<importBase+traps.size()*8&&
    (pc-importBase-4)%8==0&&svc==importSvc+(pc-importBase-4)/8){
    const auto&i=traps[(pc-importBase-4)/8];result.reason=StopReason::ImportRequired;result.library=i.library;result.ordinal=i.ordinal;
   }else{
    if(kernelEnabled){
     auto handled=kernel.dispatch(svc,result.arguments);
     if(handled.kind==KernelResult::Kind::Returned){
      cpu.set_reg(0,handled.value);auto lr=cpu.get_lr();cpu.set_cpsr((cpu.get_cpsr()&~0x20u)|(lr&1?0x20:0));cpu.set_pc(lr&~1u);return;
     }
     if(handled.kind==KernelResult::Kind::Fault){fault(handled.address,handled.access);return;}
     if(handled.kind==KernelResult::Kind::Blocked){result.reason=StopReason::ThreadBlocked;cpu.stop();return;}
    }
    result.reason=StopReason::SystemCallRequired;
   }
   cpu.stop();
  };
 }
};
const char*stopName(StopReason r){switch(r){
 case StopReason::Running:return "running";case StopReason::Returned:return "returned";
 case StopReason::ImportRequired:return "unimplemented import";case StopReason::SystemCallRequired:return "unimplemented system call";
 case StopReason::MemoryFault:return "guest memory fault";case StopReason::BudgetExhausted:return "instruction budget reached";
 case StopReason::Cancelled:return "cancelled";default:return "invalid entry point";
 case StopReason::ThreadBlocked:return "thread blocked (scheduler required)";
}}
E32Session::E32Session():impl(new Impl){}
E32Session::~E32Session()=default;
bool E32Session::load(const E32Image&image,std::uint32_t codeBase,std::uint32_t dataBase,std::string&error){
 // A session can be initialized once; no live memory/cache is reused on failure.
 if(impl->loaded||impl->memory.used()){error="Use a fresh session for each E32 image";return false;}
 auto&p=*impl;
 if(!image.load(p.memory,codeBase,dataBase,p.image,error))return false;
 p.heapMin=image.header().heapMin;p.heapMax=image.header().heapMax;
 p.stackSize=std::max<std::uint32_t>(8192,image.header().stack);
 if(!p.memory.map(stackBase,p.stackSize,Read|Write)||!p.memory.map(returnBase,8,Read|Execute)||
  (!p.image.imports.empty()&&!p.memory.map(importBase,p.image.imports.size()*8,Read|Execute))){error="Guest stack/import region mapping failed";return false;}
 std::uint32_t stub[]={0xef000000|returnSvc,0xeafffffe};p.memory.initialize(returnBase,stub,sizeof(stub));
 for(std::size_t n=0;n<p.image.imports.size();n++){
  stub[0]=0xef000000|static_cast<std::uint32_t>(importSvc+n);p.memory.initialize(importBase+n*8,stub,sizeof(stub));
  std::uint32_t address=importBase+n*8;p.memory.initialize(codeBase+p.image.imports[n].codeOffset,&address,sizeof(address));
 }
 p.traps=p.image.imports;p.loaded=true;return true;
}
const LoadedE32&E32Session::module()const{return impl->image;}
const std::vector<E32Library>&E32Session::libraries()const{return impl->libraries;}
AddressSpace&E32Session::memory(){return impl->memory;}
void E32Session::enableEka1Kernel(){impl->kernelEnabled=true;}
HeapStartResult E32Session::initializeHeap(const char*descriptor,TaskControl*control){
 auto&p=*impl;HeapStartResult result;result.create.reason=StopReason::InvalidEntry;result.switchHeap.reason=StopReason::InvalidEntry;
 if(!p.loaded||p.heapAttempted){result.error="Heap bootstrap requires a fresh loaded session";return result;}
 p.heapAttempted=true;std::uint32_t create=0,switchHeap=0;
 if(!romExport(descriptor,0x100039e5,166,create,result.error)||!romExport(descriptor,0x100039e5,1127,switchHeap,result.error))return result;
 p.kernelEnabled=true;if(control)control->report("Creating EKA1 heap");
 result.create=callArgs(create,{0,p.heapMin,p.heapMax,4096},100000,control);
 if(result.create.reason!=StopReason::Returned||!result.create.r0){result.error="Native ChunkHeap did not return an allocator";return result;}
 result.address=result.create.r0;
 if(!p.memory.permits(result.address,64,Read|Write)){result.error="Native heap allocator is outside committed guest RAM";return result;}
 result.switchHeap=call(switchHeap,result.address,100000,control);
 if(result.switchHeap.reason!=StopReason::Returned||p.kernel.heap()!=result.address){result.error="Native HeapSwitch did not select the allocator";return result;}
 result.ready=true;return result;
}
bool E32Session::mountRom(const char*path,std::string&error,TaskControl*control){return impl->rom.mount(impl->memory,path,error,control);}
bool E32Session::linkRomLibrary(const std::string&importName,const char*descriptor,std::size_t&resolved,std::string&error){
 resolved=0;std::string filename;std::uint32_t uid=0;
 if(!impl->loaded||!e32LibraryIdentity(importName,filename,uid)){error="Unsupported EKA1 library identity";return false;}
 RomLibrary library;if(!impl->rom.library(impl->memory,descriptor,library,error))return false;
 if(uid&&library.uid3!=uid){error="Imported DLL UID does not match the ROM library";return false;}
 // Validate the whole library's requested exports before changing any IAT slot.
 for(const auto&i:impl->image.imports)if(i.library==importName){
  if(!i.ordinal||i.ordinal>library.exports.size()||!impl->memory.permits(library.exports[i.ordinal-1]&~1u,4,Read|Execute)){
   error="Requested ordinal is missing/non-executable in the mounted ROM library";return false;
  }
 }
 for(const auto&i:impl->image.imports)if(i.library==importName){
  auto address=library.exports[i.ordinal-1];impl->memory.initialize(impl->image.codeBase+i.codeOffset,&address,sizeof(address));resolved++;
 }
 impl->cpu.clear_instruction_cache();impl->cpu.flush_tlb();return true;
}
ExecutionResult E32Session::call(std::uint32_t entry,std::uint32_t argument,std::uint64_t budget,TaskControl*control){
 return callArgs(entry,{argument,0,0,0},budget,control);
}
bool E32Session::romExport(const char*descriptor,std::uint32_t uid,std::uint32_t ordinal,std::uint32_t&target,std::string&error){
 RomLibrary library;if(!impl->rom.library(impl->memory,descriptor,library,error))return false;
 if(library.uid3!=uid||!ordinal||ordinal>library.exports.size()){error="ROM export UID/ordinal mismatch";return false;}
 auto address=library.exports[ordinal-1];if(!impl->memory.permits(address&~1u,address&1?2:4,Read|Execute)){error="ROM export is not executable";return false;}
 target=address;return true;
}
bool E32Session::linkDependencies(const char*folder,std::string&error,TaskControl*control){
 auto&p=*impl;error.clear();
 if(!p.loaded||!p.rom.mounted()||p.heapAttempted||p.dependenciesAttempted){error="Dependency linking requires a fresh ROM-backed session before heap bootstrap";return false;}
 p.dependenciesAttempted=true;
 struct Candidate {E32Library library;};
 struct Provider {std::uint32_t uid;std::vector<std::uint32_t> exports;};
 std::vector<Candidate> candidates;std::vector<std::uint32_t> mappings;
 std::vector<E32Import> traps=p.traps;
 std::map<std::string,Provider> providers;
 std::vector<std::pair<std::uint32_t,std::uint32_t>> patches;
 std::vector<unsigned> order;
 std::size_t exportCount=0;
 std::uint32_t codeBase=0x13000000,dataBase=0x03000000;
 const std::string root=folder;
 auto rollback=[&](){for(auto base:mappings)p.memory.unmap(base);return false;};
 auto checkpoint=[&](){if(!control||control->checkpoint())return true;error="Dependency linking cancelled";return false;};
 auto target=[&](const Provider&provider,const E32Import&import,std::uint32_t slot){
  if(!import.ordinal||import.ordinal>provider.exports.size()){error="Missing DLL ordinal: "+import.library+"#"+std::to_string(import.ordinal);return false;}
  auto address=provider.exports[import.ordinal-1];
  if(!p.memory.permits(address&~1u,address&1?2:4,Read|Execute)||(!(address&1)&&(address&3))){error="DLL export is not executable: "+import.library;return false;}
  patches.emplace_back(slot,address);return true;
 };
 std::function<bool(const E32Import&,std::uint32_t)> resolve;
 resolve=[&](const E32Import&import,std::uint32_t slot)->bool{
  if(!checkpoint())return false;
  std::string name;std::uint32_t uid=0;
  if(!e32LibraryIdentity(import.library,name,uid)){error="Unsupported DLL identity: "+import.library;return false;}
  for(char&c:name)if(c>='a'&&c<='z')c-=32;
  auto existing=providers.find(name);
  if(existing!=providers.end()){
   if(uid&&uid!=existing->second.uid){error="Conflicting DLL UID: "+name;return false;}
   return target(existing->second,import,slot);
  }
  if(providers.size()>=64){error="DLL provider limit (64) exceeded";return false;}
  RomLibrary rom;std::string romError;
  if(p.rom.library(p.memory,(root+"/rom-libs/"+name).c_str(),rom,romError)){
   if(uid&&rom.uid3!=uid){error="ROM DLL UID mismatch: "+name;return false;}
   if(rom.exports.size()>131072-exportCount){error="DLL export budget exceeded";return false;}exportCount+=rom.exports.size();
   auto provider=providers.emplace(name,Provider{rom.uid3,std::move(rom.exports)}).first;
   return target(provider->second,import,slot);
  }
  if(candidates.size()>=32){error="Game DLL dependency limit (32) exceeded";return false;}
  E32Library library;
  // Release the file buffer before descending into dependencies: 32 nested
  // inputs must not retain 32 * 8 MiB on the console's limited host heap.
  {
  E32Image image;
  if(!image.open((root+"/game-libs/"+name).c_str(),error,control)){error=name+": "+error+"; "+romError;return false;}
  const auto&h=image.header();
  if(h.uid1!=0x10000079||(uid&&h.uid3!=uid)){error="E32 DLL UID/type mismatch: "+name;return false;}
  auto codeEnd=std::uint64_t(codeBase)+((std::uint64_t(h.codeSize)+4095)&~4095ULL);
  auto dataBytes=std::uint64_t(h.dataSize)+h.bssSize;
  auto dataEnd=std::uint64_t(dataBase)+((dataBytes+4095)&~4095ULL);
  if(codeEnd>0x1f000000||dataEnd>0x0f000000||image.importCount()>8192-traps.size()){
   error="DLL addresses/import count exceed runtime limits";return false;
  }
  library.filename=name;library.uid=h.uid3;
  if(h.exportsCount>131072-exportCount){error="DLL export budget exceeded";return false;}exportCount+=h.exportsCount;
  if(control)control->report("Mapping game DLL",candidates.size(),32);
  if(!checkpoint()||!image.load(p.memory,codeBase,dataBase,library.image,error))return false;
  mappings.push_back(codeBase);if(dataBytes)mappings.push_back(dataBase);
  auto stub=importBase+static_cast<std::uint32_t>(traps.size()*8);
  if(image.importCount()){
   if(!p.memory.map(stub,image.importCount()*8,Read|Execute)){error="Cannot map DLL import guards";return false;}
   mappings.push_back(stub);
  }
  // Guard every unresolved slot, including DLLs involved in a cycle. No guest
  // code is run until the complete graph has validated and patches commit.
  for(const auto&i:library.image.imports){
   std::uint32_t words[]={0xef000000|static_cast<std::uint32_t>(importSvc+traps.size()),0xeafffffe};
   p.memory.initialize(stub,words,sizeof(words));p.memory.initialize(codeBase+i.codeOffset,&stub,4);
   traps.push_back(i);stub+=8;
  }
  codeBase=static_cast<std::uint32_t>(codeEnd);dataBase=static_cast<std::uint32_t>(dataEnd);
  }
  unsigned index=candidates.size();candidates.push_back({std::move(library)});
  const auto module=candidates[index].library.image; // recursion can reallocate candidates
  auto provider=providers.emplace(name,Provider{candidates[index].library.uid,module.exports}).first;
  if(!target(provider->second,import,slot))return false;
  for(const auto&i:module.imports)if(!resolve(i,module.codeBase+i.codeOffset))return false;
  order.push_back(index);return true;
 };
 for(const auto&i:p.image.imports)if(!resolve(i,p.image.codeBase+i.codeOffset))return rollback();
 if(!checkpoint())return rollback();
 for(const auto&patch:patches)p.memory.initialize(patch.first,&patch.second,4);
 for(auto index:order)p.libraries.push_back(std::move(candidates[index].library));
 p.traps=std::move(traps);p.cpu.clear_instruction_cache();p.cpu.flush_tlb();return true;
}
ExecutionResult E32Session::callArgs(std::uint32_t entry,const std::array<std::uint32_t,4>&arguments,std::uint64_t budget,TaskControl*control){
 auto&p=*impl;p.result={};
 if(!p.loaded||!p.memory.permits(entry&~1u,entry&1?2:4,Read|Execute)||(!(entry&1)&&(entry&3))){p.result.reason=StopReason::InvalidEntry;return p.result;}
 p.cpu.clear_instruction_cache();p.cpu.flush_tlb();p.monitor.clear_exclusive();
 for(unsigned r=0;r<16;r++)p.cpu.set_reg(r,0);
 for(unsigned r=0;r<4;r++)p.cpu.set_reg(r,arguments[r]);p.cpu.set_sp((stackBase+p.stackSize)&~7u);p.cpu.set_lr(returnBase);
 p.cpu.set_cpsr(entry&1?0x30:0x10);p.cpu.set_pc(entry&~1u);
 while(p.result.reason==StopReason::Running&&p.result.instructions<budget){
  if(control&&!control->checkpoint()){p.result.reason=StopReason::Cancelled;break;}
  p.cpu.run(static_cast<std::uint32_t>(std::min<std::uint64_t>(1000,budget-p.result.instructions)));
  auto executed=p.cpu.get_num_instruction_executed();p.result.instructions+=executed;
  if(!executed&&p.result.reason==StopReason::Running){p.result.reason=StopReason::BudgetExhausted;break;}
 }
 if(p.result.reason==StopReason::Running)p.result.reason=StopReason::BudgetExhausted;
 p.result.pc=p.cpu.get_pc();p.result.r0=p.cpu.get_reg(0);return p.result;
}
}
