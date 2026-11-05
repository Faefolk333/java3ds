#include "image_probe.h"
#include "e32_session.h"
#include "rom_image.h"
#include <set>
bool symbianImageProbe(const char*folder,FILE*report,bool(*keepRunning)()){
 using namespace symbian;std::string root=folder,path=root+"/test.app",error;
 FILE*input=std::fopen(path.c_str(),"rb");if(!input){if(report)std::fprintf(report,"INPUT_MISSING: copy test.app into /3ds/java-3ds/symbian before pressing Y\n");return false;}std::fclose(input);
 if(!report)return false;
 std::fprintf(report,"Java3DS EKA1 loader diagnostic; no phone OS/gameplay\n");std::fflush(report);
 E32Image image;if(!image.open(path.c_str(),error,keepRunning)){std::fprintf(report,"E32_REJECT %s\n",error.c_str());return true;}
 const auto&h=image.header();
 std::fprintf(report,"E32_VALID uid3=%08lx code=%lu data=%lu bss=%lu imports=%zu relocations=%zu\n",(unsigned long)h.uid3,(unsigned long)h.codeSize,(unsigned long)h.dataSize,(unsigned long)h.bssSize,image.importCount(),image.relocationCount());std::fflush(report);
 if(keepRunning&&!keepRunning())return true;
 E32Session session;if(!session.load(image,0x12000000,0x02000000,error)){std::fprintf(report,"E32_LOAD_FAIL %s\n",error.c_str());return true;}
 std::fprintf(report,"E32_MAPPED bytes=%zu\n",session.memory().used());std::fflush(report);
 auto rom=root+"/SYM.ROM";std::size_t total=0;
 if(session.mountRom(rom.c_str(),error,keepRunning)){
  std::set<std::string> libraries;for(const auto&i:session.module().imports)libraries.insert(i.library);
  for(const auto&name:libraries){
   if(keepRunning&&!keepRunning())return true;
   std::string filename;std::uint32_t uid=0;if(!e32LibraryIdentity(name,filename,uid))continue;
   std::size_t count=0;bool linked=session.linkRomLibrary(name,(root+"/rom-libs/"+filename).c_str(),count,error);total+=count;
   std::fprintf(report,"ROM_LINK %s resolved=%zu status=%s\n",name.c_str(),count,linked?"linked":error.c_str());
  }
  std::fprintf(report,"ROM_MAPPED guest_bytes=%zu imports_resolved=%zu/%zu\n",session.memory().used(),total,session.module().imports.size());
 }else std::fprintf(report,"ROM_NOT_MAPPED %s\n",error.c_str());
 if(keepRunning&&!keepRunning())return true;
 const auto&m=session.module();
 for(auto entry:{m.entry,m.exports.empty()?0:m.exports[0]}){
  if(!entry)continue;
  auto r=session.call(entry,0,100000,keepRunning);
  std::fprintf(report,"E32_CALL entry=%08lx result=%s r0=%08lx pc=%08lx instructions=%llu import=%s#%lu svc=%08lx fault=%08lx access=%u\n",(unsigned long)entry,stopName(r.reason),(unsigned long)r.r0,(unsigned long)r.pc,(unsigned long long)r.instructions,r.library.c_str(),(unsigned long)r.ordinal,(unsigned long)r.systemCall,(unsigned long)r.address,r.access);std::fflush(report);
 }
 std::fprintf(report,"DIAGNOSTIC_END no kernel/services/gameplay implemented\n");return true;
}
