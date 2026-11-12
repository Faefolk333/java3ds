#include "../native/symbian/loader_probe.h"
#include "../native/symbian/e32_session.h"
#include "../native/symbian/e32_guest.h"
#include "../native/symbian/rom_image.h"
#include "../native/symbian/image_probe.h"
#include "../native/symbian/kernel_probe.h"
#include "../native/symbian/eka1_kernel.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <set>
#include <filesystem>
#include <fstream>
using namespace symbian;
static void put(std::vector<std::uint8_t>&b,unsigned at,std::uint32_t word){for(unsigned n=0;n<4;n++)b[at+n]=word>>(n*8);}
static std::uint32_t imageRead=0;static unsigned imageCancelSignals=0;
static void imageProgress(const char*stage,std::uint32_t done,std::uint32_t){if(!std::strcmp(stage,"Reading ROM"))imageRead=done;}
static bool cancelImageOnce(){if(imageRead>=65536&&!imageCancelSignals){imageCancelSignals++;return false;}return true;}
static void kernelNegativeChecks(){
 AddressSpace memory(16384);assert(memory.map(0x10000,4096,Read|Write));Eka1Kernel kernel(memory);
 std::uint32_t out[]={0x20000000,4,0x10100},desc[]={0x3000000c,12,4096,65536,0},args[]={0x10000,0xffff0000,0x10020,0};
 memory.write(0x10000,out,sizeof(out));memory.write(0x10020,desc,sizeof(desc));memory.write(0x10040,args,sizeof(args));
 auto exec=[&](unsigned op){return kernel.dispatch(0xc00076,{op,0x10040,0x10080,0});};
 auto code=[](KernelResult r,int expected){return r.kind==KernelResult::Kind::Returned&&r.value==std::uint32_t(expected);};
 auto before=memory.used();assert(code(exec(0),-4)&&memory.used()==before);
 assert(kernel.dispatch(0xc00076,{0,0x10040,0xffffffff,0}).kind==KernelResult::Kind::Fault&&memory.used()==before);
 assert(code(kernel.dispatch(0x800083,{9,0x10100,0,0}),-5));
 assert(kernel.dispatch(0x800083,{8,0xffffffff,0,0}).kind==KernelResult::Kind::Fault);
 assert(code(kernel.dispatch(1,{0x999,0,0,0}),-8));
 std::vector<std::uint32_t> handles;
 for(unsigned i=0;i<64;i++){assert(code(exec(0x19),0));std::uint32_t h;memory.read(0x10100,&h);handles.push_back(h);}
 assert(code(exec(0x19),-4)&&memory.used()==before);
 for(auto h:handles){args[0]=h;memory.write(0x10040,args,sizeof(args));assert(code(exec(0x10),0));assert(code(exec(0x10),-8));}
 args[0]=0x10000;memory.write(0x10040,args,sizeof(args));assert(code(exec(0x19),0));std::uint32_t fresh;memory.read(0x10100,&fresh);
 assert(fresh>handles.back()&&code(kernel.dispatch(0x2a,{handles.front(),0,0,0}),-8));
 args[3]=0xffffffff;memory.write(0x10040,args,sizeof(args));assert(code(exec(0x19),-6));
 out[1]=0;memory.write(0x10000,out,sizeof(out));args[3]=0;memory.write(0x10040,args,sizeof(args));assert(code(exec(0x19),-6));
 assert(kernel.dispatch(0x123456,{}).kind==KernelResult::Kind::Unhandled);
 std::uint32_t word=0xffffffff;memory.write(0x10200,&word);assert(code(kernel.dispatch(0x8d,{0x10200,0,0,0}),-1));memory.read(0x10200,&word);assert(word==0);
 assert(code(kernel.dispatch(0x8e,{0x10200,0,0,0}),0));memory.read(0x10200,&word);assert(word==0xffffffff);
 puts("KERNEL_NEGATIVE_PASS descriptor_capacity invalid_memory stale_handles table_exhaustion no_memory unsupported_calls atomic_wrap");
}
static void romChecks(const std::vector<std::uint8_t>&e32){
 namespace fs=std::filesystem;const fs::path folder="work/symbian-rom-check";fs::create_directories(folder);
 auto save=[&](const char*name,const std::vector<std::uint8_t>&bytes){std::ofstream out(folder/name,std::ios::binary);out.write(reinterpret_cast<const char*>(bytes.data()),bytes.size());assert(out.good());return (folder/name).string();};
 std::vector<std::uint8_t> rom(8192);put(rom,0,0xea000000);put(rom,140,0x50000000);put(rom,144,rom.size());
 constexpr unsigned descriptor=0x1000-100;put(rom,descriptor,0x10000079);put(rom,descriptor+8,0x10000001);
 put(rom,descriptor+20,0x50001000);put(rom,descriptor+28,0x100);put(rom,descriptor+60,7);put(rom,descriptor+64,0x50001080);
 put(rom,0x1000,0xe3a0004d);put(rom,0x1004,0xe12fff1e); // Original ARM function: return 77.
 put(rom,0x1080+6*4,0x50001000);
 auto path=save("original-test.rom",rom);auto dll=save("PROBE.DLL",{rom.begin()+descriptor,rom.begin()+descriptor+100});
 E32Image image;std::string error;assert(image.parse(e32,error));E32Session session;assert(session.load(image,0x12000000,0x02000000,error));
 assert(session.mountRom(path.c_str(),error));std::size_t resolved=0;
 assert(!session.linkRomLibrary("../PROBE[10000001].DLL",dll.c_str(),resolved,error));
 assert(!session.linkRomLibrary("PROBE[10000002].DLL",dll.c_str(),resolved,error));
 assert(session.linkRomLibrary("PROBE[10000001].DLL",dll.c_str(),resolved,error)&&resolved==1);
 auto r=session.call(session.module().exports[1]);assert(r.reason==StopReason::Returned&&r.r0==77);
 auto rejectedHeap=session.initializeHeap(dll.c_str());assert(!rejectedHeap.ready&&!rejectedHeap.error.empty());
 assert(!session.initializeHeap(dll.c_str()).ready);
 assert(!session.mountRom(path.c_str(),error));
 AddressSpace small(4096);RomImage limit;assert(!limit.mount(small,path.c_str(),error)&&small.used()==0);
 AddressSpace cancelled;RomImage cancelledRom;TaskControl stop([](){return false;});
 assert(!cancelledRom.mount(cancelled,path.c_str(),error,&stop)&&cancelled.used()==0&&!cancelledRom.mounted()&&stop.cancelled());
 auto large=rom;large.resize(3*65536);put(large,144,large.size());auto largePath=save("large-original-test.rom",large);
 unsigned polls=0,previous=0,progressEvents=0;TaskControl transient([&](){return ++polls!=3;},[&](const char*stage,std::uint32_t done,std::uint32_t total){
  if(!std::strcmp(stage,"Reading ROM")){assert(done>=previous&&done<=total);previous=done;progressEvents++;}
 });
 RomImage interrupted;assert(!interrupted.mount(cancelled,largePath.c_str(),error,&transient));
 assert(transient.cancelled()&&transient.done()==65536&&cancelled.used()==0&&!interrupted.mounted());
 assert(!transient.checkpoint()&&polls==3&&progressEvents==2);
 unsigned finished=0;TaskControl retry({},[&](const char*stage,std::uint32_t done,std::uint32_t total){if(!std::strcmp(stage,"Reading ROM")){assert(done>=finished&&done<=total);finished=done;}});
 assert(interrupted.mount(cancelled,largePath.c_str(),error,&retry)&&interrupted.mounted()&&finished==large.size()&&!retry.cancelled());
 assert(cancelled.unmap(0x50000000)&&cancelled.used()==0);
 // Reproduce the console report: a false poll during ROM reading followed by
 // true polls must never fall through to entry/export calls without the ROM.
 save("test.app",e32);save("SYM.ROM",large);
 FILE*report=std::tmpfile();assert(report);assert(symbianImageProbe(folder.string().c_str(),report,cancelImageOnce,imageProgress));
 std::rewind(report);char text[4096]={};auto length=std::fread(text,1,sizeof(text)-1,report);std::fclose(report);
 assert(length&&std::strstr(text,"DIAGNOSTIC_CANCELLED")&&!std::strstr(text,"E32_CALL")&&!std::strstr(text,"DIAGNOSTIC_END")&&imageCancelSignals==1);
 puts("IMAGE_CANCEL_PASS transient_cancel_latched ROM_rollback no_guest_call_after_cancel monotonic_progress");
 auto bad=rom;put(bad,144,8193);auto broken=save("truncated.rom",bad);AddressSpace memory;RomImage rejected;
 assert(!rejected.mount(memory,broken.c_str(),error)&&memory.used()==0);
 bad=rom;put(bad,140,0x80000000);broken=save("eka2.rom",bad);assert(!rejected.mount(memory,broken.c_str(),error)&&memory.used()==0);
 RomImage good;assert(good.mount(memory,path.c_str(),error));RomLibrary library;
 auto header=std::vector<std::uint8_t>(rom.begin()+descriptor,rom.begin()+descriptor+100);header[8]^=1;broken=save("wrong-device.dll",header);
 assert(!good.library(memory,broken.c_str(),library,error));assert(good.library(memory,dll.c_str(),library,error)&&library.exports.size()==7);
 std::uint32_t value=0;assert(!memory.write(0x50001000,&value));
 puts("EKA1_ROM_PASS original_fixture direct_ordinal_link real_guest_call exact_ROM_pairing bounds budget mismatch readonly unsupported_EKA2");
}
int main(int argc,char**argv){
 if(argc==3&&!std::strcmp(argv[1],"--folder"))return symbianImageProbe(argv[2],stdout)?0:5;
 if(argc>1){
  E32Image image;std::string error;
  if(!image.open(argv[1],error)){printf("E32_REJECT %s\n",error.c_str());return 2;}
  const auto&h=image.header();
  printf("E32_VALID uid3=%08x code=%u data=%u bss=%u stack=%u heap_min=%u heap_max=%u imports=%zu relocations=%zu\n",h.uid3,h.codeSize,h.dataSize,h.bssSize,h.stack,h.heapMin,h.heapMax,image.importCount(),image.relocationCount());
  E32Session session;if(!session.load(image,0x12000000,0x02000000,error)){printf("E32_LOAD_FAIL %s\n",error.c_str());return 3;}
  const auto&m=session.module();printf("E32_MAPPED code=%08x data=%08x entry=%08x bytes=%zu exports=%zu\n",m.codeBase,m.dataBase,m.entry,session.memory().used(),m.exports.size());
  if(argc>=4){
   if(!session.mountRom(argv[2],error)){printf("ROM_LOAD_FAIL %s\n",error.c_str());return 4;}
   std::set<std::string> libraries;std::size_t total=0;
   for(const auto&i:m.imports)libraries.insert(i.library);
   for(const auto&name:libraries){
    std::string filename;std::uint32_t uid=0;if(!e32LibraryIdentity(name,filename,uid))continue;
    auto path=std::string(argv[3])+"/"+filename;std::size_t resolved=0;
    bool okay=session.linkRomLibrary(name,path.c_str(),resolved,error);total+=resolved;
    printf("ROM_LINK %s resolved=%zu status=%s\n",name.c_str(),resolved,okay?"linked":error.c_str());
   }
   printf("ROM_MAPPED guest_bytes=%zu imports_resolved=%zu/%zu\n",session.memory().used(),total,m.imports.size());
   auto heap=session.initializeHeap((std::string(argv[3])+"/EUSER.DLL").c_str());printf("THREAD_HEAP ready=%d address=%08x error=%s\n",heap.ready,heap.address,heap.error.c_str());if(!heap.ready)return 6;
  }
  for(auto entry:{m.entry,m.exports.empty()?0:m.exports[0]}){
   if(!entry)continue;auto r=session.call(entry,0,100000);
   printf("E32_CALL entry=%08x result=%s r0=%08x pc=%08x instructions=%llu import=%s#%u svc=%08x fault=%08x access=%u\n",entry,stopName(r.reason),r.r0,r.pc,(unsigned long long)r.instructions,r.library.c_str(),r.ordinal,r.systemCall,r.address,r.access);
  }
  if(argc==5&&!std::strcmp(argv[4],"--stress")){
   assert(h.uid3==0x101fd3f6&&!m.exports.empty());std::set<std::uint32_t> objects;std::uint32_t table=0;
   for(unsigned n=0;n<512;n++){
    auto r=session.call(m.exports[0]);assert(r.reason==StopReason::Returned&&session.memory().permits(r.r0,556,Read|Write));
    std::uint32_t vtable=0;assert(session.memory().read(r.r0,&vtable)&&session.memory().permits(vtable,64,Read));
    if(!n)table=vtable;auto position=objects.lower_bound(r.r0);
    if(position!=objects.end())assert(std::uint64_t(r.r0)+556<=*position);
    if(position!=objects.begin()){auto before=position;--before;assert(std::uint64_t(*before)+556<=r.r0);}
    assert(vtable==table&&objects.insert(r.r0).second);
   }
   printf("NATIVE_HEAP_STRESS_PASS application_objects=%zu first=%08x last=%08x vtable=%08x reserved_guest_bytes=%zu\n",objects.size(),*objects.begin(),*objects.rbegin(),table,session.memory().used());
  }
  return 0;
 }
 auto result=symbianLoaderProbe();
 printf("E32_PROBE parsed=%d relocated=%d returned=%d imports=%d svc=%d protected_memory=%d bounded=%d cancelled=%d\n",result.parsed,result.relocated,result.returned,result.imports,result.systemCall,result.protectedMemory,result.bounded,result.cancelled);fflush(stdout);
 assert(result.passed());
 auto kernel=symbianKernelProbe();printf("KERNEL_PROBE chunks=%d handles=%d sync=%d heap=%d abi=%d thumb=%d\n",kernel.chunks,kernel.handles,kernel.sync,kernel.heap,kernel.abi,kernel.thumb);fflush(stdout);assert(kernel.passed());
 kernelNegativeChecks();
 AddressSpace memory(8192);assert(!memory.map(0,4096,Read));assert(!memory.map(0xfffff000,8192,Read));
 assert(memory.map(0x1000,4096,Read|Write));assert(!memory.map(0x1800,4096,Read));
 assert(!memory.map(0x5000,8192,Read));assert(memory.used()==4096);
 std::uint32_t value=123,out=0;assert(memory.write(0x1ffc,&value));assert(!memory.write(0x1ffd,&value));assert(!memory.read(0xffffffff,&out));
 assert(!memory.permits(0x1000,4,Execute));assert(memory.unmap(0x1000)&&memory.used()==0);
 E32Image image;std::string error;
 const std::vector<std::uint8_t> original(std::begin(symbian_e32_guest),std::end(symbian_e32_guest));
 romChecks(original);
 unsigned rejected=0;
 for(auto change:{0u,12u,16u,20u,48u,52u,64u,68u,72u,84u,88u,92u,96u,100u,104u,108u,112u,116u}){
  auto b=original;put(b,change,0xffffffff);assert(!image.parse(std::move(b),error));rejected++;
 }
 for(std::size_t length=0;length<original.size();length++){
  auto b=original;b.resize(length);assert(!image.parse(std::move(b),error));rejected++;
 }
 // Every mutated import/relocation field below must reject before any map occurs.
 for(auto field:{332u,336u,340u,344u,360u,364u,368u,372u}){
  auto b=original;put(b,field,0xffffffff);assert(!image.parse(std::move(b),error));rejected++;
 }
 assert(image.parse(original,error));AddressSpace small(128);LoadedE32 loaded;
 TaskControl stop([](){return false;});assert(!image.open("work/symbian-e32/original-e32-test.app",error,&stop)&&stop.cancelled());
 assert(!image.load(memory,0x12000000,0x02000000,loaded,error)&&memory.used()==0);
 assert(image.parse(original,error));
 assert(!image.load(small,0x12000000,0x02000000,loaded,error)&&small.used()==0);
 AddressSpace rollback(205);assert(!image.load(rollback,0x12000000,0x02000000,loaded,error)&&rollback.used()==0);
 assert(!image.parse({},error));assert(!image.load(memory,0x12000000,0x02000000,loaded,error)&&memory.used()==0);
 printf("E32_LOADER_PASS negative_cases=%u bounds UID imports relocations memory_budget rollback RX_RW NX cancel no_fabricated_system_services\n",rejected);
}
