#include "e32_session.h"
#include "rom_image.h"
#include "eka1_kernel.h"
#include "file_server.h"
#include "graphics_server.h"
#include "runtime_guest.h"
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
 bool dependenciesReady=false,romInitAttempted=false,romInitReady=false,cleanupAttempted=false;
 std::vector<RomInitEntry> romRoots;
 std::vector<std::uint32_t> initializedRomEntries;bool romProgramAttempted=false;
 RomImage rom;
 FileServer fileServer{memory,rom};
 GraphicsServer graphicsServer{memory};
 std::string testApplicationPath;bool filesMounted=false;
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
  kernel.fileService=[this](auto handle,auto operation,const auto&args){return fileServer.request(handle,operation,args);};
  kernel.systemService=[this](const auto&name,auto handle,auto operation,const auto&args){
   if(name==u"Fontbitmapserver"&&operation==-1&&!kernel.initializeGraphicsResources())return KernelResult{KernelResult::Kind::Returned,std::uint32_t(-4),0,0};
   return graphicsServer.request(name,handle,operation,args);
  };
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
      if(handled.value>=0xffffff00u&&svc!=0x80&&svc!=0x8d&&svc!=0x8e){result.kernelErrorCall=svc;result.kernelErrorValue=handled.value;result.kernelErrorArguments=result.arguments;}
      cpu.set_reg(0,handled.value);auto lr=cpu.get_lr();cpu.set_cpsr((cpu.get_cpsr()&~0x20u)|(lr&1?0x20:0));cpu.set_pc(lr&~1u);return;
     }
     if(handled.kind==KernelResult::Kind::Exited){cpu.set_reg(0,handled.value);result.reason=StopReason::ThreadExited;cpu.stop();return;}
     if(handled.kind==KernelResult::Kind::Fault){fault(handled.address,handled.access);return;}
     if(handled.kind==KernelResult::Kind::Blocked){result.reason=StopReason::ThreadBlocked;cpu.stop();return;}
     if(svc==0xc00076&&((result.arguments[0]&255)==0x16||(result.arguments[0]&255)==2||(result.arguments[0]&255)==0x18)){
      std::uint32_t args[4];std::u16string name;
      if(memory.read(result.arguments[1],args,sizeof(args))&&kernel.descriptor16(args[1],name))for(auto c:name)result.serviceName+=c>=32&&c<127?char(c):'?';
     }
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
 case StopReason::ThreadExited:return "guest thread exited";
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
bool E32Session::mountRom(const char*path,std::string&error,TaskControl*control){
 if(!impl->rom.mount(impl->memory,path,error,control))return false;
 RomDisplay display;if(impl->rom.display(impl->memory,display))impl->graphicsServer.window.configure(display);return true;
}
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
 std::vector<RomInitEntry> romRoots;
 std::size_t exportCount=0;
 std::uint32_t codeBase=0x13000000,dataBase=0x03000000;
 const std::string root=folder;
 p.testApplicationPath=root+"/test.app";
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
   romRoots.push_back({rom.uid3,rom.entry,rom.references});
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
 p.romRoots=std::move(romRoots);p.dependenciesReady=true;
 p.traps=std::move(traps);p.cpu.clear_instruction_cache();p.cpu.flush_tlb();return true;
}
RomStartResult E32Session::initializeRomLibraries(TaskControl*control){
 auto&p=*impl;RomStartResult result;result.call.reason=StopReason::InvalidEntry;
 if(!p.dependenciesReady||!p.kernel.heap()||p.romInitAttempted){result.error="ROM initialization requires linked dependencies and a fresh native thread heap";return result;}
 p.romInitAttempted=true;std::vector<RomInitEntry> order;
 if(!p.rom.initializationOrder(p.memory,p.romRoots,order,result.error,control))return result;
 result.planned=order.size();
 for(const auto&library:order){
  result.uid=library.uid;result.entry=library.entry;
  if(control)control->report("Initializing ROM DLL",result.completed,result.planned);
  result.call=call(library.entry,1,100000,control);
  if(result.call.reason!=StopReason::Returned||result.call.r0){result.error="ROM DLL thread attach did not complete";return result;}
  result.completed++;
 }
 for(const auto&library:order)p.initializedRomEntries.push_back(library.entry);
 p.romInitReady=true;result.ready=true;return result;
}
CleanupStartResult E32Session::initializeCleanup(const char*descriptor,TaskControl*control){
 auto&p=*impl;CleanupStartResult result;result.call.reason=StopReason::InvalidEntry;
 if(!p.romInitReady||p.cleanupAttempted){result.error="Cleanup bootstrap requires initialized ROM libraries and a fresh thread";return result;}
 p.cleanupAttempted=true;std::uint32_t create=0;
 // EKA1 EUSER ordinal 746: CTrapCleanup::New (pinned epoc6_n.def).
 if(!romExport(descriptor,0x100039e5,746,create,result.error))return result;
 if(control)control->report("Creating native cleanup stack");result.call=call(create,0,100000,control);
 result.address=result.call.r0;result.handler=p.kernel.trap();
 if(result.call.reason!=StopReason::Returned||!p.memory.permits(result.address,4,Read|Write)||!p.memory.permits(result.handler,4,Read|Write)){
  result.error="Native cleanup stack/handler did not initialize";return result;
 }
 result.ready=true;return result;
}
NativeRuntimeResult E32Session::checkNativeRuntime(const char*descriptor,unsigned repetitions,TaskControl*control){
 auto&p=*impl;NativeRuntimeResult result;result.call.reason=StopReason::InvalidEntry;
 if(!p.cleanupAttempted||!p.kernel.trap()||p.kernel.trapDepth()||!repetitions||repetitions>512){result.error="Native TRAP test requires initialized cleanup and 1..512 repetitions";return result;}
 constexpr std::uint32_t code=0x30010000,data=0x31010000,out=data+128;
 std::array<std::uint32_t,10> functions{};const unsigned ordinals[]={1148,1164,630,857,824,45,476,859};
 for(unsigned n=0;n<8;n++)if(!romExport(descriptor,0x100039e5,ordinals[n],functions[n],result.error))return result;
 if(!p.memory.map(code,sizeof(symbian_runtime_guest),Read|Execute)){result.error="Cannot map native TRAP diagnostic code";return result;}
 if(!p.memory.map(data,4096,Read|Write)){p.memory.unmap(code);result.error="Cannot map native TRAP diagnostic data";return result;}
 p.memory.initialize(code,symbian_runtime_guest,sizeof(symbian_runtime_guest));functions[9]=out;
 auto handler=p.kernel.trap();std::uint32_t firstAllocation=0;bool reuse=true;
 auto run=[&](unsigned mode,bool nested){
  std::array<std::uint32_t,12> output{};output[6]=out;output[7]=1;output[8]=out;output[9]=2;
  functions[8]=mode;p.memory.write(data,functions.data(),sizeof(functions));p.memory.write(out,output.data(),sizeof(output));
  result.call=call(code+(nested?0x200:0),data,100000,control);
  p.memory.read(out,output.data(),sizeof(output));
  bool returned=result.call.reason==StopReason::Returned&&p.kernel.trapDepth()==0&&p.kernel.trap()==handler;
  if(!returned){result.error="Native TRAP/cleanup did not return with balanced state";return false;}
  if(nested)return output[0]==std::uint32_t(-42)&&output[1]==2&&output[3]==0&&output[10]==21&&output[11]==std::uint32_t(-43);
  if(!output[5]||!p.memory.permits(output[5],256,Read|Write))return false;
  if(!firstAllocation)firstAllocation=output[5];else reuse=reuse&&firstAllocation==output[5];
  return output[0]==(mode?std::uint32_t(-42):0)&&output[1]==1&&output[2]==1&&output[3]==0&&output[10]==1&&(!mode||output[4]==123);
 };
 if(control)control->report("Testing native TRAP / cleanup",0,repetitions+2);
 result.normal=run(0,false);
 if(result.normal)result.leave=run(1,false);
 if(result.leave)result.nested=run(0,true);
 if(result.nested){
  result.cleanup=true;
  for(unsigned n=0;n<repetitions;n++){
   if(control)control->report("Testing native TRAP / cleanup",n+2,repetitions+2);
   if(!run(n&1,false)){result.cleanup=false;break;}result.iterations++;
  }
 }
 result.heapReuse=result.cleanup&&reuse;
 if(!result.passed()&&result.error.empty())result.error="Native TRAP result/cleanup order/register or allocation-reuse mismatch";
 p.memory.unmap(data);p.memory.unmap(code);return result;
}
NativeContextResult E32Session::checkNativeContext(const char*descriptor,TaskControl*control){
 auto&p=*impl;NativeContextResult result;result.call.reason=StopReason::InvalidEntry;
 if(!p.cleanupAttempted||!p.kernel.trap()||p.kernel.trapDepth()||p.kernel.scheduler()){result.error="Native context test requires initialized cleanup and no active scheduler";return result;}
 constexpr std::uint32_t code=0x30010000,scheduler=0x31020000,low=0x31021000,high=0x31022000,data=0x31023000;
 const unsigned ordinals[]={335,336,328,1343,569,294,963,1259,1395,13,979,1265,630};std::uint32_t fn[13];
 for(unsigned n=0;n<13;n++)if(!romExport(descriptor,0x100039e5,ordinals[n],fn[n],result.error))return result;
 std::vector<std::uint32_t> mapped;
 auto map=[&](std::uint32_t at,std::uint32_t bytes,unsigned access){if(!p.memory.map(at,bytes,access))return false;mapped.push_back(at);return true;};
 auto invoke=[&](unsigned n,std::array<std::uint32_t,4> args){result.call=callArgs(fn[n],args,100000,control);return result.call.reason==StopReason::Returned;};
 do{
  if(!map(code,sizeof(symbian_runtime_guest),Read|Execute)||!map(scheduler,64,Read|Write)||!map(low,64,Read|Write)||!map(high,64,Read|Write)||!map(data,256,Read|Write)){result.error="Cannot map native context fixture";break;}
  p.memory.initialize(code,symbian_runtime_guest,sizeof(symbian_runtime_guest));
  if(control)control->report("Testing native DLL TLS");
  constexpr std::uint32_t key=0xe0001234;
  if(!invoke(1,{key,0,0,0})||result.call.r0||!invoke(0,{key,data,0,0})||result.call.r0||!invoke(1,{key,0,0,0})||result.call.r0!=data||!invoke(0,{key,data+4,0,0})||result.call.r0||!invoke(1,{key,0,0,0})||result.call.r0!=data+4||!invoke(2,{key,0,0,0})||!invoke(1,{key,0,0,0})||result.call.r0)break;
  result.tls=true;if(control)control->report("Testing native active scheduler");
  if(!invoke(3,{scheduler,0,0,0})||result.call.r0!=scheduler||!invoke(4,{scheduler,0,0,0})||!invoke(5,{})||result.call.r0!=scheduler)break;
  result.scheduler=true;
  std::uint32_t table[]={0,0,code+0x4c0,code+0x4c0,code+0x400,code+0x480};p.memory.write(data+128,table,sizeof(table));
  bool objectsReady=true;
  for(auto object:{low,high}){
   std::uint32_t expected=object==low?1u:10u,priority=0;
   if(!invoke(8,{object,expected,0,0})||result.call.r0!=object||!p.memory.read(object+20,&priority)||priority!=expected){objectsReady=false;break;}
   std::uint32_t vtable=data+128,fields[]={data,object==low?1u:2u,fn[12],0};p.memory.write(object,&vtable);p.memory.write(object+24,fields,sizeof(fields));
   if(!invoke(9,{object,0,0,0})||!invoke(10,{object,0,0,0})){objectsReady=false;break;}
   std::uint32_t complete=0;p.memory.write(object+4,&complete);
  }
  if(!objectsReady)break;
  std::uint32_t output[4]={};
  if(!invoke(6,{data+64,0,0,0})||result.call.r0!=1||!invoke(6,{data+64,0,0,0})||result.call.r0!=1||!invoke(6,{data+64,0,0,0})||result.call.r0!=0)break;
  p.memory.read(data,output,sizeof(output));result.events=output[0]==2&&output[1]==21;
  if(!result.events)break;
  std::uint32_t leave=std::uint32_t(-42),complete=0;p.memory.write(low+36,&leave);p.memory.write(low+4,&complete);
  if(!invoke(10,{low,0,0,0})||!invoke(6,{data+64,0,0,0})||result.call.r0!=1)break;
  p.memory.read(data,output,sizeof(output));std::uint32_t dispatchError=1;p.memory.read(data+64,&dispatchError);
  result.runError=output[0]==3&&output[1]==211&&output[2]==std::uint32_t(-42)&&output[3]==1&&dispatchError==0&&p.kernel.trapDepth()==0;
  if(!result.runError)break;
  if(!invoke(11,{low,0,0,0})||!invoke(11,{high,0,0,0})||!invoke(7,{scheduler,0,0,0})||!invoke(5,{})||result.call.r0)break;
  result.teardown=p.kernel.scheduler()==0&&p.kernel.trapDepth()==0;
 }while(false);
 if(!result.passed()&&result.error.empty())result.error="Native TLS/scheduler/RunL/RunError/teardown check did not complete";
 for(auto at:mapped)p.memory.unmap(at);return result;
}
NativeFilesResult E32Session::checkNativeFiles(const char*efsrv,const char*euser,TaskControl*control){
 auto&p=*impl;NativeFilesResult result;
 if(!p.romInitReady||!p.kernel.trap()||p.kernel.trapDepth()||p.kernel.scheduler()||p.romProgramAttempted){result.error="Native files require initialized idle thread context";return result;}
 if(!p.filesMounted){if(!p.fileServer.mountApplication(p.testApplicationPath)){result.error="Cannot mount diagnostic application file";return result;}p.filesMounted=true;}
 std::uint32_t functions[12]{};const unsigned ordinals[]={18,163,178,121,185,142,162,15,139};
 for(unsigned n=0;n<9;n++)if(!romExport(efsrv,0x100039e4,ordinals[n],functions[n],result.error))return result;
 if(!romExport(euser,0x100039e5,172,functions[9],result.error))return result;
 if(!romExport(efsrv,0x100039e4,39,functions[10],result.error)||!romExport(efsrv,0x100039e4,68,functions[11],result.error))return result;
 constexpr std::uint32_t data=0x31030000;
 if(!p.memory.map(data,4096,Read|Write)){result.error="Cannot allocate native file diagnostic storage";return result;}
 struct Unmap {AddressSpace&m;~Unmap(){m.unmap(data);}} unmap{p.memory};
 auto word=[&](unsigned offset,std::uint32_t value){p.memory.write(data+offset,&value);};
 auto get=[&](unsigned offset){std::uint32_t value=0;p.memory.read(data+offset,&value);return value;};
 auto string=[&](unsigned offset,const std::u16string&value){word(offset,value.size());p.memory.write(data+offset+4,value.data(),value.size()*2);};
 auto buffer=[&](unsigned offset,unsigned pointer,unsigned maximum){word(offset,0x20000000);word(offset+4,maximum);word(offset+8,data+pointer);};
 auto invoke=[&](unsigned fn,const std::array<std::uint32_t,4>&args,const char*stage,bool zero=true){
  if(control)control->report(stage);result.call=callArgs(functions[fn],args,100000,control);
  if(result.call.reason!=StopReason::Returned||(zero&&result.call.r0)){result.error=stage;return false;}return true;
 };
 const auto fs=data,file=data+64;buffer(128,160,256);buffer(768,800,64);
 if(!invoke(0,{fs,4,0,0},"Connecting native RFs"))return result;result.connected=get(0)!=0;
 if(!invoke(1,{fs,data+128,0,0},"Reading native session path"))return result;
 result.path=get(128)==0x20000003;
 string(400,u"E:\\System\\Apps\\Java3DS\\");
 if(!invoke(2,{fs,data+400,0,0},"Setting native session path"))return result;
 string(400,u"test.app");
 if(!invoke(10,{fs,data+400,data+1100,0},"Reading native file entry"))return result;
 result.entry=get(1116)==0x10000079&&get(1128)==8;
 if(!invoke(3,{file,fs,data+400,0},"Opening native application file"))return result;result.opened=true;
 if(!invoke(4,{file,data+960,0,0},"Reading native file size"))return result;auto size=get(960);if(size<12){result.error="Mounted application file is truncated";return result;}
 if(!invoke(5,{file,0,data+768,12},"Reading native E32 UID header"))return result;result.read=(get(768)&0xffffff)==12&&get(800)==0x10000079;
 word(960,std::uint32_t(-4));
 if(!invoke(6,{file,3,data+960,0},"Seeking native file from end"))return result;result.seek=get(960)==size-4;
 if(!invoke(5,{file,size+1,data+768,12},"Reading native end of file"))return result;result.eof=(get(768)&0xffffff)==0;
 // Async RFile::Read must complete the guest TRequestStatus and signal once.
 word(960,std::uint32_t(-4));if(!invoke(6,{file,3,data+960,0},"Restoring native file position"))return result;
 word(976,0x80000001);if(!invoke(8,{file,data+768,4,data+976},"Completing native asynchronous file read",false))return result;
 result.read=result.read&&get(976)==0&&(get(768)&0xffffff)==4;
 if(!invoke(7,{file,0,0,0},"Closing native RFile",false))return result;
 string(400,u"missing-java3ds-test.file");
 if(!invoke(3,{file,fs,data+400,0},"Checking native missing file",false))return result;result.missing=result.call.r0==std::uint32_t(-1);
 string(400,u"Z:\\System\\Programs\\AppRun.exe");
 if(!invoke(11,{fs,data+400,0,0},"Locating native ROM file",false))return result;auto romAddress=result.call.r0;
 if(!invoke(3,{file,fs,data+400,0},"Opening native ROM file")||!invoke(5,{file,0,data+768,12},"Reading native ROM UID header"))return result;
 result.rom=romAddress>=0x50000000&&(get(768)&0xffffff)==12&&get(800)==0x1000007a;
 if(!invoke(7,{file,0,0,0},"Closing native ROM file",false))return result;
 if(!invoke(9,{fs,0,0,0},"Closing native RFs",false))return result;result.closed=get(0)==0;
 if(!result.passed())result.error="Native file result mismatch";return result;
}
NativeGraphicsResult E32Session::checkNativeGraphics(const char*descriptor,TaskControl*control){
 auto&p=*impl;NativeGraphicsResult result;
 if(!p.romInitReady||!p.kernel.trap()||p.kernel.trapDepth()||p.kernel.scheduler()||p.romProgramAttempted){result.error="Native graphics require initialized idle thread context";return result;}
 std::uint32_t functions[4]{};const unsigned ordinals[]={13,48,56,20};
 for(unsigned n=0;n<4;n++)if(!romExport(descriptor,0x10003a15,ordinals[n],functions[n],result.error))return result;
 auto invoke=[&](unsigned fn,std::uint32_t arg,const char*stage,bool zero=false){
  if(control)control->report(stage);result.call=call(functions[fn],arg,100000,control);
  if(result.call.reason!=StopReason::Returned||(zero&&result.call.r0)){result.error=stage;return false;}return true;
 };
 if(!invoke(0,0,"Connecting native RFbsSession",true)||!invoke(1,0,"Reading native FBS session"))return result;
 auto session=result.call.r0;result.connected=session&&p.memory.permits(session,48,Read|Write);
 if(!result.connected){result.error="Invalid native FBS session storage";return result;}
 if(!invoke(2,session,"Checking native FBS shared chunk"))return result;
 result.shared=result.call.r0&&p.memory.permits(result.call.r0,65536,Read|Write);
 if(!invoke(0,0,"Retaining native FBS session",true)||!invoke(3,0,"Releasing native FBS reference")||!invoke(1,0,"Checking retained native FBS session"))return result;result.retained=result.call.r0==session;
 if(!invoke(3,0,"Closing native FBS session")||!invoke(1,0,"Checking closed native FBS session"))return result;result.closed=result.call.r0==0;
 if(!result.passed())result.error="Native FBS connection result mismatch";return result;
}
NativeWindowResult E32Session::checkNativeWindow(const char*ws32,const char*fbscli,const char*euser,TaskControl*control){
 auto&p=*impl;NativeWindowResult result;RomDisplay display;
 if(!p.romInitReady||!p.kernel.trap()||p.kernel.trapDepth()||p.kernel.scheduler()||p.romProgramAttempted||!p.rom.display(p.memory,display)){result.error="Native window test requires initialized idle EKA1 display context";return result;}
 std::uint32_t functions[18]{};const unsigned ordinals[]={268,43,276,47,243,244,75,272,46,139,210,151,35,263,74};
 for(unsigned n=0;n<15;n++)if(!romExport(ws32,0x10003b21,ordinals[n],functions[n],result.error))return result;
 if(!romExport(fbscli,0x10003a15,13,functions[15],result.error)||!romExport(fbscli,0x10003a15,20,functions[16],result.error)||!romExport(euser,0x100039e5,45,functions[17],result.error))return result;
 constexpr std::uint32_t data=0x31040000;
 if(!p.memory.map(data,4096,Read|Write)){result.error="Cannot allocate native window diagnostic storage";return result;}
 struct Unmap {AddressSpace&m;~Unmap(){m.unmap(data);}} unmap{p.memory};
 auto word=[&](unsigned offset,std::uint32_t value){p.memory.write(data+offset,&value);};auto get=[&](unsigned offset){std::uint32_t value=0;p.memory.read(data+offset,&value);return value;};
 auto invoke=[&](unsigned fn,const std::array<std::uint32_t,4>&args,const char*stage,bool zero=false){
  if(control)control->report(stage);result.call=callArgs(functions[fn],args,200000,control);
  if(result.call.reason!=StopReason::Returned||(zero&&result.call.r0)){result.error=std::string(stage)+" command="+std::to_string(result.call.windowCommand)+" object="+std::to_string(result.call.windowObject);return false;}return true;
 };
 if(!invoke(15,{},"Connecting FBS for native screen",true)||!invoke(0,{data,0,0,0},"Constructing native RWsSession")||!invoke(1,{data,0,0,0},"Connecting native RWsSession",true))return result;result.connected=true;
 if(!invoke(17,{512,0,0,0},"Allocating native screen device"))return result;auto device=result.call.r0;if(!device||!p.memory.permits(device,512,Read|Write)){result.error="Cannot allocate native screen device";return result;}
 if(!invoke(2,{device,data,0,0},"Constructing native CWsScreenDevice")||!invoke(3,{device,0,0,0},"Creating native screen device",true))return result;result.device=true;
 if(!invoke(4,{data+128,device,0,0},"Querying native screen pixels"))return result;result.pixels=get(128)==display.width&&get(132)==display.height;
 if(!invoke(5,{data+128,device,0,0},"Querying native screen twips"))return result;result.twips=get(128)==display.width*15&&get(132)==display.height*15;
 if(!invoke(6,{device,0,0,0},"Querying native screen color"))return result;
 std::uint32_t expectedMode=display.bits==1?1:display.bits==2?2:display.bits==4?5:display.bits==8?6:display.bits==12?10:display.bits==16?7:display.bits==24?8:12;result.color=result.call.r0==expectedMode;
 if(!invoke(7,{data+64,data,0,0},"Constructing native RWindowGroup")||!invoke(8,{data+64,123,1,0},"Creating native window group",true)||!invoke(9,{data+64,0,0,0},"Querying native group identifier"))return result;result.group=result.call.r0>0&&result.call.r0<0x7fffffff;
 const std::u16string name=u"Java3DS native screen";word(256,name.size());p.memory.write(data+260,name.data(),name.size()*2);word(384,0x20000000);word(388,64);word(392,data+400);
 if(!invoke(10,{data+64,data+256,0,0},"Setting native window group name",true)||!invoke(11,{data+64,data+384,0,0},"Reading native window group name",true))return result;
 std::u16string actual(name.size(),0);result.name=get(384)==(0x20000000|name.size())&&p.memory.read(data+400,actual.data(),actual.size()*2)&&actual==name;
 if(!invoke(12,{data+64,0,0,0},"Closing native window group")||!invoke(13,{device,0,0,0},"Destroying native screen device")||!invoke(14,{data,0,0,0},"Closing native RWsSession")||!invoke(16,{},"Closing native screen FBS reference"))return result;
 std::uint32_t free=0;if(!romExport(euser,0x100039e5,476,free,result.error))return result;result.call=call(free,device,100000,control);result.closed=result.call.reason==StopReason::Returned;
 if(!result.passed())result.error="Native window result mismatch";return result;
}
ExecutionResult E32Session::runRomProgram(const char*path,std::string&error,TaskControl*control){
 auto&p=*impl;ExecutionResult result;result.reason=StopReason::InvalidEntry;
 if(!p.romInitReady||!p.kernel.trap()||p.kernel.trapDepth()||p.kernel.scheduler()||p.romProgramAttempted){error="ROM process entry requires initialized, fresh idle thread context";return result;}
 RomFile file;if(!p.rom.findFile(p.memory,path,file,error,control))return result;
 // ROM directory file length is a logical file length, not the size of the
 // executable's resident code extent. Validate that extent against the ROM.
 std::uint32_t h[25];if(file.size<100||!p.memory.read(file.address,h,sizeof(h))||h[0]!=0x1000007a||h[5]!=file.address+100||(h[4]&~1u)!=h[5]||!h[7]||h[8]>h[7]||h[5]<0x50000000||std::uint64_t(h[5])+h[7]>0x50000000ULL+p.rom.bytes()||h[9]||h[10]){error="Unsupported ROM executable header/data layout";return result;}
 std::vector<RomInitEntry> roots,order;
 if(h[14]){
  std::uint32_t refs=0;if(h[14]<0x50000000||std::uint64_t(h[14])+4>0x50000000ULL+p.rom.bytes()||!p.memory.read(h[14],&refs)||(refs&0xffff)!=1||(refs>>16)>128||std::uint64_t(h[14])+4+(refs>>16)*8ULL>0x50000000ULL+p.rom.bytes()){error="Invalid ROM executable dependency table";return result;}
  for(unsigned n=0;n<(refs>>16);n++){std::uint32_t pair[2];p.memory.read(h[14]+4+n*8,pair,8);roots.push_back({0,pair[0],pair[1]});}
 }
 if(!p.rom.initializationOrder(p.memory,roots,order,error,control))return result;
 for(const auto&library:order)if(std::find(p.initializedRomEntries.begin(),p.initializedRomEntries.end(),library.entry)==p.initializedRomEntries.end()){error="ROM executable depends on an unattached DLL";return result;}
 std::u16string filename;for(const char*c=path;*c;c++)filename+=*c=='/'?'\\':char16_t(*c);
 if(!p.kernel.configureProcess(filename,u"\"E:\\System\\Apps\\Java3DS\\test.app\" R\"test\" ")){error="Cannot configure bounded diagnostic process metadata";return result;}
 if(!p.filesMounted){if(!p.fileServer.mountApplication(p.testApplicationPath)){error="Cannot mount diagnostic application file";return result;}p.filesMounted=true;}
 p.romProgramAttempted=true;if(control)control->report("Entering ROM AppRun");return call(h[4],0,1000000,control);
}
ExecutionResult E32Session::callArgs(std::uint32_t entry,const std::array<std::uint32_t,4>&arguments,std::uint64_t budget,TaskControl*control){
 auto&p=*impl;p.result={};
 if(p.kernelEnabled&&!p.kernel.alive()){p.result.reason=StopReason::ThreadExited;p.result.r0=p.kernel.exitCode();return p.result;}
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
 p.result.pc=p.cpu.get_pc();p.result.r0=p.cpu.get_reg(0);
 const auto&t=p.fileServer.trace;p.result.fileCalls=t.calls;p.result.fileOpens=t.opens;p.result.fileReads=t.reads;p.result.fileBytes=t.bytes;p.result.fileOperation=t.operation;p.result.fileResult=t.result;p.result.fileUnsupported=t.unsupported;
 for(auto c:t.path)p.result.filePath+=c>=32&&c<127?char(c):'?';
 p.result.graphicsCalls=p.graphicsServer.calls;p.result.graphicsInitializations=p.graphicsServer.initializations;p.result.graphicsOperation=p.graphicsServer.lastOperation;
 p.result.windowCalls=p.graphicsServer.window.calls;p.result.windowInitializations=p.graphicsServer.window.initializations;p.result.windowOperation=p.graphicsServer.window.lastOperation;p.result.windowCommand=p.graphicsServer.window.lastCommand;p.result.windowObject=p.graphicsServer.window.lastObject;
 p.result.screenDevices=p.graphicsServer.window.created;p.result.screenQueries=p.graphicsServer.window.queries;p.result.windowGroups=p.graphicsServer.window.groupsCreated;auto screen=p.graphicsServer.window.screen();p.result.screenWidth=screen.width;p.result.screenHeight=screen.height;p.result.screenBits=screen.bits;return p.result;
}
}
