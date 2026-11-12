#include "image_probe.h"
#include "e32_session.h"
#include "rom_image.h"
#include "version.h"
#include <set>
#include <chrono>
bool symbianImageProbe(const char*folder,FILE*report,bool(*keepRunning)(),void(*progress)(const char*,std::uint32_t,std::uint32_t),unsigned nativeRepetitions){
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
  bool dependencies=session.linkDependencies(folder,error,&control);
  std::size_t dllImports=0;for(const auto&library:session.libraries())dllImports+=library.image.imports.size();
  std::fprintf(report,"DEPENDENCY_GRAPH ready=%d game_libraries=%zu main_imports_resolved=%zu/%zu DLL_imports=%zu guest_bytes=%zu error=%s\n",dependencies,session.libraries().size(),dependencies?session.module().imports.size():total,session.module().imports.size(),dllImports,session.memory().used(),error.c_str());std::fflush(report);
  if(!checkpoint())return true;
  if(!dependencies){std::fprintf(report,"DIAGNOSTIC_STOP dependency_graph_incomplete\n");return true;}
  auto heap=session.initializeHeap((root+"/rom-libs/EUSER.DLL").c_str(),&control);
  std::fprintf(report,"THREAD_HEAP ready=%d address=%08lx guest_bytes=%zu create=%s switch=%s error=%s\n",heap.ready,(unsigned long)heap.address,session.memory().used(),stopName(heap.create.reason),stopName(heap.switchHeap.reason),heap.error.c_str());std::fflush(report);
  if(!checkpoint())return true;
  if(!heap.ready){std::fprintf(report,"DIAGNOSTIC_STOP heap_initialization_incomplete svc=%08lx\n",(unsigned long)heap.create.systemCall);return true;}
  auto initialized=session.initializeRomLibraries(&control);
  std::fprintf(report,"ROM_INITIALIZATION ready=%d completed=%zu/%zu uid=%08lx entry=%08lx result=%s r0=%08lx svc=%08lx fault=%08lx args=%08lx,%08lx,%08lx,%08lx error=%s\n",initialized.ready,initialized.completed,initialized.planned,(unsigned long)initialized.uid,(unsigned long)initialized.entry,stopName(initialized.call.reason),(unsigned long)initialized.call.r0,(unsigned long)initialized.call.systemCall,(unsigned long)initialized.call.address,(unsigned long)initialized.call.arguments[0],(unsigned long)initialized.call.arguments[1],(unsigned long)initialized.call.arguments[2],(unsigned long)initialized.call.arguments[3],initialized.error.c_str());std::fflush(report);
  if(!checkpoint())return true;
  if(!initialized.ready){std::fprintf(report,"DIAGNOSTIC_STOP ROM_DLL_initialization_incomplete\n");return true;}
  for(const auto&library:session.libraries()){
   control.report("Initializing game DLL");auto r=session.call(library.image.entry,1,100000,&control);
   std::fprintf(report,"DLL_THREAD_ATTACH name=%s uid=%08lx entry=%08lx result=%s r0=%08lx svc=%08lx fault=%08lx\n",library.filename.c_str(),(unsigned long)library.uid,(unsigned long)library.image.entry,stopName(r.reason),(unsigned long)r.r0,(unsigned long)r.systemCall,(unsigned long)r.address);std::fflush(report);
   if(!checkpoint())return true;
   if(r.reason!=StopReason::Returned||r.r0){std::fprintf(report,"DIAGNOSTIC_STOP game_DLL_initialization_incomplete\n");return true;}
  }
 }else{if(!checkpoint())return true;std::fprintf(report,"ROM_NOT_MAPPED %s\n",error.c_str());return true;}
 if(!checkpoint())return true;
 const auto&m=session.module();
 for(auto entry:{m.entry,m.exports.empty()?0:m.exports[0]}){
  if(!entry)continue;
  control.report("Executing diagnostic");auto r=session.call(entry,entry==m.entry?1:0,100000,&control);
  std::fprintf(report,"E32_CALL entry=%08lx result=%s r0=%08lx pc=%08lx instructions=%llu import=%s#%lu svc=%08lx fault=%08lx access=%u\n",(unsigned long)entry,stopName(r.reason),(unsigned long)r.r0,(unsigned long)r.pc,(unsigned long long)r.instructions,r.library.c_str(),(unsigned long)r.ordinal,(unsigned long)r.systemCall,(unsigned long)r.address,r.access);std::fflush(report);
  if(!checkpoint())return true;
  if(entry==m.entry){
   if(r.reason!=StopReason::Returned||r.r0){std::fprintf(report,"DIAGNOSTIC_STOP application_DLL_initialization_incomplete\n");return true;}
   auto cleanup=session.initializeCleanup((root+"/rom-libs/EUSER.DLL").c_str(),&control);
   std::fprintf(report,"NATIVE_CLEANUP ready=%d address=%08lx handler=%08lx result=%s svc=%08lx fault=%08lx error=%s\n",cleanup.ready,(unsigned long)cleanup.address,(unsigned long)cleanup.handler,stopName(cleanup.call.reason),(unsigned long)cleanup.call.systemCall,(unsigned long)cleanup.call.address,cleanup.error.c_str());std::fflush(report);
   if(!checkpoint())return true;
   if(!cleanup.ready){std::fprintf(report,"DIAGNOSTIC_STOP cleanup_initialization_incomplete\n");return true;}
   auto native=session.checkNativeRuntime((root+"/rom-libs/EUSER.DLL").c_str(),nativeRepetitions,&control);
   std::fprintf(report,"NATIVE_TRAP normal=%d leave=%d nested=%d cleanup=%d heap_reuse=%d iterations=%u result=%s svc=%08lx fault=%08lx error=%s\n",native.normal,native.leave,native.nested,native.cleanup,native.heapReuse,native.iterations,stopName(native.call.reason),(unsigned long)native.call.systemCall,(unsigned long)native.call.address,native.error.c_str());std::fflush(report);
   if(!checkpoint())return true;
   if(!native.passed()){std::fprintf(report,"DIAGNOSTIC_STOP native_TRAP_cleanup_incomplete\n");return true;}
   auto context=session.checkNativeContext((root+"/rom-libs/EUSER.DLL").c_str(),&control);
   std::fprintf(report,"NATIVE_CONTEXT tls=%d scheduler=%d events=%d run_error=%d teardown=%d result=%s svc=%08lx fault=%08lx error=%s\n",context.tls,context.scheduler,context.events,context.runError,context.teardown,stopName(context.call.reason),(unsigned long)context.call.systemCall,(unsigned long)context.call.address,context.error.c_str());std::fflush(report);
   if(!checkpoint())return true;
   if(!context.passed()){std::fprintf(report,"DIAGNOSTIC_STOP native_thread_context_incomplete\n");return true;}
  }
  if(entry!=m.entry&&r.reason==StopReason::Returned&&r.r0){
   std::uint32_t vtable=0;
   if(session.memory().permits(r.r0,4,Read|Write)&&session.memory().read(r.r0,&vtable)&&session.memory().permits(vtable,4,Read)){
    std::fprintf(report,"APPLICATION_OBJECT address=%08lx vtable=%08lx mapped_vtable=1\n",(unsigned long)r.r0,(unsigned long)vtable);
    // EKA1 GCC2 primary CApaApplication ABI: two-word vtable prefix,
    // CBase destructor, PreDocConstructL, CreateDocumentL, AppDllUid.
    // Only inspect a single-inheritance primary table; never guess for EKA2.
    std::uint32_t table[6]={};
    if(h.uid2==0x100039ce&&session.memory().read(vtable,table,sizeof(table))&&table[0]==0&&table[1]==0&&session.memory().permits(table[5]&~1u,table[5]&1?2:4,Read|Execute)){
     control.report("Checking application UID");auto uid=session.call(table[5],r.r0,100000,&control);
     std::fprintf(report,"APPLICATION_UID result=%s value=%08lx expected=%08lx matches=%d svc=%08lx fault=%08lx\n",stopName(uid.reason),(unsigned long)uid.r0,(unsigned long)h.uid3,uid.reason==StopReason::Returned&&uid.r0==h.uid3,(unsigned long)uid.systemCall,(unsigned long)uid.address);std::fflush(report);
    }
   }
  }
  if(!checkpoint())return true;
 }
 if(checkpoint()){
  auto run=session.runRomProgram("Z:/System/Programs/AppRun.exe",error,&control);
  std::fprintf(report,"ROM_APPRUN result=%s r0=%08lx pc=%08lx instructions=%llu svc=%08lx fault=%08lx access=%u args=%08lx,%08lx,%08lx,%08lx error=%s\n",stopName(run.reason),(unsigned long)run.r0,(unsigned long)run.pc,(unsigned long long)run.instructions,(unsigned long)run.systemCall,(unsigned long)run.address,run.access,(unsigned long)run.arguments[0],(unsigned long)run.arguments[1],(unsigned long)run.arguments[2],(unsigned long)run.arguments[3],error.c_str());std::fflush(report);
  if(run.systemCall==0xc00076){std::uint32_t args[4];if(session.memory().read(run.arguments[1],args,sizeof(args)))std::fprintf(report,"ROM_APPRUN_EXECUTOR operation=%08lx args=%08lx,%08lx,%08lx,%08lx\n",(unsigned long)run.arguments[0],(unsigned long)args[0],(unsigned long)args[1],(unsigned long)args[2],(unsigned long)args[3]);}
  if(run.kernelErrorValue)std::fprintf(report,"ROM_APPRUN_KERNEL_ERROR svc=%08lx value=%08lx args=%08lx,%08lx,%08lx,%08lx\n",(unsigned long)run.kernelErrorCall,(unsigned long)run.kernelErrorValue,(unsigned long)run.kernelErrorArguments[0],(unsigned long)run.kernelErrorArguments[1],(unsigned long)run.kernelErrorArguments[2],(unsigned long)run.kernelErrorArguments[3]);
  if(!run.serviceName.empty())std::fprintf(report,"ROM_APPRUN_SERVICE name=%s stage=session_creation_unimplemented\n",run.serviceName.c_str());
 }
 if(!checkpoint())return true;
 auto elapsed=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-started).count();
 control.report("Diagnostic finished");std::fprintf(report,"DIAGNOSTIC_END elapsed_ms=%lld partial_EPOC6_kernel_no_phone_OS_or_gameplay\n",(long long)elapsed);return true;
}
