#include "image_probe.h"
#include "e32_session.h"
#include "rom_image.h"
#include "version.h"
#include <set>
#include <chrono>
bool symbianImageProbe(const char*folder,FILE*report,bool(*keepRunning)(),void(*progress)(const char*,std::uint32_t,std::uint32_t)){
 using namespace symbian;std::string root=folder,path=root+"/test.app",error;
 TaskControl control(keepRunning,progress);const auto started=std::chrono::steady_clock::now();
 auto checkpoint=[&](){if(control.checkpoint())return true;auto ms=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-started).count();std::fprintf(report,"DIAGNOSTIC_CANCELLED stage=%s completed=%lu total=%lu elapsed_ms=%lld\n",control.stage(),(unsigned long)control.done(),(unsigned long)control.total(),(long long)ms);std::fflush(report);return false;};
 FILE*input=std::fopen(path.c_str(),"rb");if(!input){if(report)std::fprintf(report,"INPUT_MISSING: copy test.app into /3ds/java-3ds/symbian before pressing Y\n");return false;}std::fclose(input);
 if(!report)return false;
 std::fprintf(report,"Java3DS EKA1 loader diagnostic " SYMBIAN_PROBE_VERSION "; no phone OS/gameplay\n");std::fflush(report);
 E32Image image;if(!image.open(path.c_str(),error,&control)){if(!checkpoint())return true;std::fprintf(report,"E32_REJECT %s\n",error.c_str());return true;}
 const auto&h=image.header();
 std::fprintf(report,"E32_VALID uid3=%08lx code=%lu data=%lu bss=%lu imports=%zu relocations=%zu\n",(unsigned long)h.uid3,(unsigned long)h.codeSize,(unsigned long)h.dataSize,(unsigned long)h.bssSize,image.importCount(),image.relocationCount());std::fflush(report);
 if(!checkpoint())return true;control.report("Mapping program");
 E32Session session;if(!session.load(image,0x12000000,0x02000000,error)){std::fprintf(report,"E32_LOAD_FAIL %s\n",error.c_str());return true;}
 std::fprintf(report,"E32_MAPPED bytes=%zu\n",session.memory().used());std::fflush(report);
 auto rom=root+"/SYM.ROM";std::size_t total=0;
 if(session.mountRom(rom.c_str(),error,&control)){
  std::set<std::string> libraries;for(const auto&i:session.module().imports)libraries.insert(i.library);
  std::uint32_t linkedLibraries=0;
  for(const auto&name:libraries){
   if(!checkpoint())return true;control.report("Linking libraries",linkedLibraries++,libraries.size());
   std::string filename;std::uint32_t uid=0;if(!e32LibraryIdentity(name,filename,uid))continue;
   std::size_t count=0;bool linked=session.linkRomLibrary(name,(root+"/rom-libs/"+filename).c_str(),count,error);total+=count;
   std::fprintf(report,"ROM_LINK %s resolved=%zu status=%s\n",name.c_str(),count,linked?"linked":error.c_str());
  }
  control.report("Linking libraries",libraries.size(),libraries.size());
  std::fprintf(report,"ROM_MAPPED guest_bytes=%zu imports_resolved=%zu/%zu\n",session.memory().used(),total,session.module().imports.size());
 }else{if(!checkpoint())return true;std::fprintf(report,"ROM_NOT_MAPPED %s\n",error.c_str());}
 if(!checkpoint())return true;
 const auto&m=session.module();
 for(auto entry:{m.entry,m.exports.empty()?0:m.exports[0]}){
  if(!entry)continue;
  control.report("Executing diagnostic");auto r=session.call(entry,0,100000,&control);
  std::fprintf(report,"E32_CALL entry=%08lx result=%s r0=%08lx pc=%08lx instructions=%llu import=%s#%lu svc=%08lx fault=%08lx access=%u\n",(unsigned long)entry,stopName(r.reason),(unsigned long)r.r0,(unsigned long)r.pc,(unsigned long long)r.instructions,r.library.c_str(),(unsigned long)r.ordinal,(unsigned long)r.systemCall,(unsigned long)r.address,r.access);std::fflush(report);
  if(!checkpoint())return true;
 }
 auto elapsed=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-started).count();
 control.report("Diagnostic finished");std::fprintf(report,"DIAGNOSTIC_END elapsed_ms=%lld no kernel/services/gameplay implemented\n",(long long)elapsed);return true;
}
