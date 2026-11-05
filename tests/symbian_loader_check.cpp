#include "../native/symbian/loader_probe.h"
#include "../native/symbian/e32_session.h"
#include "../native/symbian/e32_guest.h"
#include "../native/symbian/rom_image.h"
#include "../native/symbian/image_probe.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <set>
#include <filesystem>
#include <fstream>
using namespace symbian;
static void put(std::vector<std::uint8_t>&b,unsigned at,std::uint32_t word){for(unsigned n=0;n<4;n++)b[at+n]=word>>(n*8);}
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
 assert(!session.mountRom(path.c_str(),error));
 AddressSpace small(4096);RomImage limit;assert(!limit.mount(small,path.c_str(),error)&&small.used()==0);
 AddressSpace cancelled;RomImage cancelledRom;assert(!cancelledRom.mount(cancelled,path.c_str(),error,[](){return false;})&&cancelled.used()==0&&!cancelledRom.mounted());
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
  if(argc==4){
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
  }
  for(auto entry:{m.entry,m.exports.empty()?0:m.exports[0]}){
   if(!entry)continue;auto r=session.call(entry,0,100000);
   printf("E32_CALL entry=%08x result=%s r0=%08x pc=%08x instructions=%llu import=%s#%u svc=%08x fault=%08x access=%u\n",entry,stopName(r.reason),r.r0,r.pc,(unsigned long long)r.instructions,r.library.c_str(),r.ordinal,r.systemCall,r.address,r.access);
  }
  return 0;
 }
 auto result=symbianLoaderProbe();
 printf("E32_PROBE parsed=%d relocated=%d returned=%d imports=%d svc=%d protected_memory=%d bounded=%d cancelled=%d\n",result.parsed,result.relocated,result.returned,result.imports,result.systemCall,result.protectedMemory,result.bounded,result.cancelled);fflush(stdout);
 assert(result.passed());
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
 assert(!image.open("work/symbian-e32/original-e32-test.app",error,[](){return false;}));
 assert(!image.load(memory,0x12000000,0x02000000,loaded,error)&&memory.used()==0);
 assert(image.parse(original,error));
 assert(!image.load(small,0x12000000,0x02000000,loaded,error)&&small.used()==0);
 AddressSpace rollback(205);assert(!image.load(rollback,0x12000000,0x02000000,loaded,error)&&rollback.used()==0);
 assert(!image.parse({},error));assert(!image.load(memory,0x12000000,0x02000000,loaded,error)&&memory.used()==0);
 printf("E32_LOADER_PASS negative_cases=%u bounds UID imports relocations memory_budget rollback RX_RW NX cancel no_fabricated_system_services\n",rejected);
}
