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
#include "symbian_file_server_check.h"
#include "symbian_window_check.h"
static void put(std::vector<std::uint8_t>&b,unsigned at,std::uint32_t word){for(unsigned n=0;n<4;n++)b[at+n]=word>>(n*8);}
static std::uint32_t imageRead=0;static unsigned imageCancelSignals=0;
static void imageProgress(const char*stage,std::uint32_t done,std::uint32_t){if(!std::strcmp(stage,"Reading ROM"))imageRead=done;}
static bool cancelImageOnce(){if(imageRead>=65536&&!imageCancelSignals){imageCancelSignals++;return false;}return true;}
static bool cancelNative=false;
static const char*cancelServiceStage=nullptr;
static bool nativeKeepRunning(){return !cancelNative;}
static void nativeCancelProgress(const char*stage,std::uint32_t done,std::uint32_t){if(cancelServiceStage?!std::strcmp(stage,cancelServiceStage):(!std::strcmp(stage,"Testing native TRAP / cleanup")&&done>=3))cancelNative=true;}
static void romLookupChecks(){
 namespace fs=std::filesystem;const fs::path folder="work/symbian-rom-lookup-check";fs::create_directories(folder);const auto path=(folder/"original.rom").string();
 std::vector<std::uint8_t> original(8192);put(original,0,0xea000000);put(original,140,0x50000000);put(original,144,original.size());put(original,148,0x50000200);put(original,0x200,1);put(original,0x208,0x50000300);
 auto directory=[&](auto&b,unsigned at,const char*name,unsigned target,unsigned size,bool isDirectory){
  unsigned length=std::strlen(name),bytes=(10+length*2+3)&~3u;put(b,at,bytes);put(b,at+4,size);put(b,at+8,target);b[at+12]=isDirectory?16:1;b[at+13]=length;
  for(unsigned n=0;n<length;n++){b[at+14+n*2]=name[n];b[at+15+n*2]=0;}
 };
 directory(original,0x300,"System",0x50000400,0,true);directory(original,0x400,"Programs",0x50000500,0,true);directory(original,0x500,"AppRun.exe",0x50001000,100,false);
 auto lookup=[&](const auto&b,const char*name,RomFile&file,TaskControl*control=nullptr){
  {std::ofstream f(path,std::ios::binary);f.write(reinterpret_cast<const char*>(b.data()),b.size());assert(f.good());}
  AddressSpace memory;RomImage rom;std::string error;assert(rom.mount(memory,path.c_str(),error));return rom.findFile(memory,name,file,error,control);
 };
 RomFile file;assert(lookup(original,"z:\\system/PROGRAMS/apprun.EXE",file)&&file.address==0x50001000&&file.size==100);
 for(const char*name:{"Z:/System/Programs/missing.exe","C:/System/Programs/AppRun.exe","Z:/../System/Programs/AppRun.exe","Z:/System/Programs/*","Z:/System/Programs"}){
  assert(!lookup(original,name,file)&&file.address==0x50001000&&file.size==100);
 }
 for(unsigned which=0;which<5;which++){
  auto b=original;
  if(which==0)b[0x50d]=255;
  if(which==1)put(b,0x308,0x50000300);
  if(which==2)put(b,0x508,0xffffff00);
  if(which==3)put(b,0x200,17);
  if(which==4){put(b,0x200,2);put(b,0x210,0x50000600);directory(b,0x600,"System",0x50000700,0,true);directory(b,0x700,"Programs",0x50000800,0,true);directory(b,0x800,"AppRun.exe",0x50001100,100,false);}
  assert(!lookup(b,"Z:/System/Programs/AppRun.exe",file)&&file.address==0x50001000);
 }
 unsigned polls=0;TaskControl cancel([&](){return ++polls<2;});assert(!lookup(original,"Z:/System/Programs/AppRun.exe",file,&cancel)&&cancel.cancelled());
 puts("ROM_LOOKUP_PASS UTF16_casefold last_entry_length_word path_guards bounds cycles variants cancellation atomic_output");
}
static void romInitializationChecks(){
 namespace fs=std::filesystem;const fs::path folder="work/symbian-rom-init-check";fs::create_directories(folder);const auto path=(folder/"original.rom").string();
 std::vector<std::uint8_t> original(16384);put(original,0,0xea000000);put(original,140,0x50000000);put(original,144,original.size());
 auto header=[&](auto&bytes,unsigned code,unsigned uid,bool thumb,unsigned refs){auto h=code-100;put(bytes,h,0x10000079);put(bytes,h+8,uid);put(bytes,h+16,0x50000000+code+(thumb?1:0));put(bytes,h+20,0x50000000+code);put(bytes,h+28,8);put(bytes,h+56,refs);};
 header(original,0x1000,1,false,0x50001100);header(original,0x2000,2,true,0x50002100);header(original,0x3000,3,false,0);
 put(original,0x1100,0x10001);put(original,0x1104,0x50002001);put(original,0x1108,0x50002100);
 put(original,0x2100,0x20001);put(original,0x2104,0x50001000);put(original,0x2108,0x50001100);put(original,0x210c,0x50003000);put(original,0x2110,0);
 auto plan=[&](const auto&bytes,std::vector<RomInitEntry>&order,TaskControl*control=nullptr){
  {std::ofstream out(path,std::ios::binary);out.write(reinterpret_cast<const char*>(bytes.data()),bytes.size());assert(out.good());}
  AddressSpace memory;RomImage rom;std::string error;assert(rom.mount(memory,path.c_str(),error));return rom.initializationOrder(memory,{{1,0x50001000,0x50001100}},order,error,control);
 };
 std::vector<RomInitEntry> order;assert(plan(original,order)&&order.size()==3&&order[0].uid==3&&order[1].uid==2&&order[2].uid==1);
 for(unsigned which=0;which<6;which++){
  auto invalid=original;
  if(which==0)put(invalid,0x2108,0x50001104); // identity mismatch even on an already visited node
  if(which==1)put(invalid,0x1100,0x810001); // more than 128 references
  if(which==2)put(invalid,0x2000-100+36,4); // static writable data needs a process mapping
  if(which==3)put(invalid,0x1100,0x10002); // unsupported flags
  if(which==4)put(invalid,0x1104,0xffffffff); // outside ROM
  if(which==5)put(invalid,0x3000-100+20,0x50003004); // unsupported entry/header layout
  auto previous=order;assert(!plan(invalid,order)&&order.size()==previous.size()&&order[0].entry==previous[0].entry);
 }
 unsigned polls=0;TaskControl cancel([&](){return ++polls<3;});assert(!plan(original,order,&cancel)&&cancel.cancelled()&&order.size()==3);
 std::vector<std::uint8_t> large(0x22000);put(large,0,0xea000000);put(large,140,0x50000000);put(large,144,large.size());
 for(unsigned n=0;n<257;n++){
  unsigned code=0x1000+n*0x200,ref=n==256?0:0x50000000+code+0x100;header(large,code,n+1,false,ref);
  if(ref){put(large,code+0x100,0x10001);put(large,code+0x104,0x50000000+code+0x200);put(large,code+0x108,n==255?0:ref+0x200);}
 }
 assert(!plan(large,order)&&order.size()==3);
 E32Session session;assert(!session.initializeRomLibraries().ready&&!session.initializeCleanup("missing").ready);
 puts("ROM_INIT_PLAN_PASS dependency_order cycle visited_identity bounds flags static_data_rejection 256_DLL_limit atomic_output cancellation bootstrap_order");
}
static void dependencyChecks(const std::vector<std::uint8_t>&original){
 namespace fs=std::filesystem;const fs::path root="work/symbian-dependency-check";
 fs::create_directories(root/"rom-libs");fs::create_directories(root/"game-libs");
 auto save=[&](const fs::path&name,const auto&bytes){std::ofstream f(root/name,std::ios::binary);f.write(reinterpret_cast<const char*>(bytes.data()),bytes.size());assert(f.good());};
 auto uid=[&](std::vector<std::uint8_t>&b,std::uint32_t value){
  put(b,8,value);std::uint16_t halves[2]={};
  for(unsigned s=0;s<2;s++)for(unsigned n=s;n<12;n+=2){halves[s]^=std::uint16_t(b[n])<<8;for(unsigned k=0;k<8;k++)halves[s]=(halves[s]&0x8000)?(halves[s]<<1)^0x1021:halves[s]<<1;}
  put(b,12,std::uint32_t(halves[1])<<16|halves[0]);
 };
 auto child=original;uid(child,0x10000001);put(child,92,7);put(child,124+0xa0+5*4,0);put(child,124+0xa0+6*4,0x40);
 const char dependency[]="EUSER[100039e5].DLL";std::memcpy(child.data()+348,dependency,sizeof(dependency));
 save("game-libs/PROBE.DLL",child);
 std::vector<std::uint8_t> rom(8192);put(rom,0,0xea000000);put(rom,140,0x50000000);put(rom,144,rom.size());
 constexpr unsigned descriptor=0x1000-100;put(rom,descriptor,0x10000079);put(rom,descriptor+8,0x100039e5);
 put(rom,descriptor+20,0x50001000);put(rom,descriptor+28,0x100);put(rom,descriptor+60,7);put(rom,descriptor+64,0x50001080);
 put(rom,0x1000,0xe3a0004d);put(rom,0x1004,0xe12fff1e);put(rom,0x1080+6*4,0x50001000);
 save("SYM.ROM",rom);save("rom-libs/EUSER.DLL",std::vector<std::uint8_t>(rom.begin()+descriptor,rom.begin()+descriptor+100));
 E32Image image;std::string error;assert(image.parse(original,error));
 auto initialize=[&](E32Session&s){assert(s.load(image,0x12000000,0x02000000,error));assert(s.mountRom((root/"SYM.ROM").string().c_str(),error));};
 E32Session linked;initialize(linked);assert(linked.linkDependencies(root.string().c_str(),error));assert(linked.libraries().size()==1);
 assert(linked.libraries()[0].uid==0x10000001);auto r=linked.call(linked.module().exports[1]);assert(r.reason==StopReason::Returned&&r.r0==77);
 assert(!linked.linkDependencies(root.string().c_str(),error));
 auto invalid=child;uid(invalid,0x10000002);save("game-libs/PROBE.DLL",invalid);
 E32Session wrong;initialize(wrong);auto used=wrong.memory().used();assert(!wrong.linkDependencies(root.string().c_str(),error));assert(wrong.memory().used()==used&&wrong.libraries().empty());
 auto shortExports=child;put(shortExports,92,6);save("game-libs/PROBE.DLL",shortExports);
 E32Session ordinal;initialize(ordinal);used=ordinal.memory().used();assert(!ordinal.linkDependencies(root.string().c_str(),error)&&ordinal.memory().used()==used);
 save("game-libs/PROBE.DLL",child);fs::remove(root/"rom-libs/EUSER.DLL");
 E32Session missing;initialize(missing);used=missing.memory().used();assert(!missing.linkDependencies(root.string().c_str(),error)&&missing.memory().used()==used);
 r=missing.call(missing.module().exports[1]);assert(r.reason==StopReason::ImportRequired&&r.library=="PROBE[10000001].DLL");
 // A cyclic graph links only one copy and remains guarded by bounded calls.
 auto cyclic=child;const char self[]="PROBE[10000001].DLL";std::memcpy(cyclic.data()+348,self,sizeof(self));save("game-libs/PROBE.DLL",cyclic);
 E32Session cycle;initialize(cycle);assert(cycle.linkDependencies(root.string().c_str(),error)&&cycle.libraries().size()==1);
 // Every node imports the next. Reject the 33rd DLL and roll back all 32
 // existing code/data/import mappings, leaving the main IAT untouched.
 for(unsigned n=0;n<33;n++){
  auto node=child;char next[32];std::snprintf(next,sizeof(next),"P%04u[10000001].DLL",n+1);std::memcpy(node.data()+348,next,std::strlen(next)+1);
  char file[32];std::snprintf(file,sizeof(file),"game-libs/P%04u.DLL",n);save(file,node);
 }
 auto chain=child;const char next[]="P0000[10000001].DLL";std::memcpy(chain.data()+348,next,sizeof(next));save("game-libs/PROBE.DLL",chain);
 E32Session limited;initialize(limited);used=limited.memory().used();assert(!limited.linkDependencies(root.string().c_str(),error)&&limited.memory().used()==used&&limited.libraries().empty());assert(error.find("limit (32)")!=std::string::npos);
 unsigned mappings=0;TaskControl midCancel([&](){return mappings<2;},[&](const char*stage,std::uint32_t,std::uint32_t){if(!std::strcmp(stage,"Mapping game DLL"))mappings++;});
 E32Session partial;initialize(partial);used=partial.memory().used();assert(!partial.linkDependencies(root.string().c_str(),error,&midCancel)&&midCancel.cancelled()&&partial.memory().used()==used&&partial.libraries().empty());
 bool mapped=false;TaskControl cancel([&](){return !mapped;},[&](const char*stage,std::uint32_t,std::uint32_t){if(!std::strcmp(stage,"Mapping game DLL"))mapped=true;});
 E32Session cancelled;initialize(cancelled);used=cancelled.memory().used();assert(!cancelled.linkDependencies(root.string().c_str(),error,&cancel)&&cancel.cancelled()&&cancelled.memory().used()==used);
 std::string file;std::uint32_t expected;
 assert(e32LibraryIdentity("BLUETOOTH.DLL",file,expected)&&file=="BLUETOOTH.DLL"&&expected==0);
 for(const char*name:{"../PROBE.DLL","C:PROBE.DLL","foo/bar.DLL",".DLL","PROBE[garbage].DLL"})assert(!e32LibraryIdentity(name,file,expected));
 puts("DEPENDENCY_GRAPH_PASS nested_native_call UID_identity ordinal cycle 32_DLL_limit rollback mid_graph_cancel path_guards repeat_rejected");
}
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
 assert(memory.map(0x20000,8192,Read|Write));
 assert(kernel.dispatch(0x72,{0xffffffff,0,0,0}).kind==KernelResult::Kind::Fault&&kernel.trapDepth()==0);
 std::uint32_t resultPointer=0x10100;
 for(unsigned n=0;n<64;n++){auto frame=0x20000+n*80;memory.write(frame+68,&resultPointer);assert(code(kernel.dispatch(0x72,{frame,0,0,0}),0));}
 memory.write(0x21400+68,&resultPointer);assert(kernel.dispatch(0x72,{0x21400,0,0,0}).kind==KernelResult::Kind::Fault&&kernel.trapDepth()==64);
 assert(kernel.trapDepth()==64&&kernel.dispatch(0x72,{0x20000,0,0,0}).kind==KernelResult::Kind::Fault);
 auto last=0x20000+63*80;word=0;memory.write(last+64,&word);assert(kernel.dispatch(0x73,{}).kind==KernelResult::Kind::Fault&&kernel.trapDepth()==64);
 word=last-80;memory.write(last+64,&word);
 for(unsigned n=64;n;n--)assert(code(kernel.dispatch(0x73,{}),0x20000+(n-1)*80));
 assert(code(kernel.dispatch(0x73,{}),0)&&kernel.trapDepth()==0);
 for(unsigned n=0;n<64;n++){args[0]=0x1000+n;args[1]=0xffff0000+n;memory.write(0x10040,args,sizeof(args));assert(code(exec(0x34),0));assert(code(kernel.dispatch(0x80,{args[0],0,0,0}),args[1]));}
 args[0]=0x2000;memory.write(0x10040,args,sizeof(args));assert(code(exec(0x34),-4));
 args[0]=0x1000;args[1]=123;memory.write(0x10040,args,sizeof(args));assert(code(exec(0x34),0)&&code(kernel.dispatch(0x80,{args[0],0,0,0}),123));assert(code(exec(0x35),0)&&code(kernel.dispatch(0x80,{args[0],0,0,0}),0));
 args[0]=0x2000;memory.write(0x10040,args,sizeof(args));assert(code(exec(0x34),0));
 assert(code(kernel.dispatch(0x74,{}),0));kernel.dispatch(0x75,{0x10100,0,0,0});assert(code(kernel.dispatch(0x74,{}),0x10100));assert(code(kernel.dispatch(0x75,{0xffffffff,0,0,0}),-6)&&kernel.scheduler()==0x10100);kernel.dispatch(0x75,{});
 assert(code(kernel.dispatch(0x4f,{'E',0,0,0}),'e')&&code(kernel.dispatch(0x51,{'Z',0,0,0}),'z')&&code(kernel.dispatch(0x52,{'b',0,0,0}),'B')&&kernel.dispatch(0x4f,{0x100,0,0,0}).kind==KernelResult::Kind::Unhandled);
 assert(kernel.configureProcess(u"Z:\\System\\AppRun.exe",u"test"));assert(code(kernel.dispatch(0x8000c0,{0xffff0000,0,0,0}),4)&&code(kernel.dispatch(0x8000c0,{123,0,0,0}),-8));
 std::uint32_t textDescriptor[]={0x20000000,4,0x10500};memory.write(0x10400,textDescriptor,sizeof(textDescriptor));assert(code(kernel.dispatch(0x80001f,{0x10400,0xffff0000,0,0}),0));std::uint32_t length=0;memory.read(0x10400,&length);char16_t text[4];memory.read(0x10500,text,sizeof(text));assert(length==0x20000004&&std::u16string(text,4)==u"test");
 textDescriptor[1]=3;memory.write(0x10400,textDescriptor,sizeof(textDescriptor));assert(code(kernel.dispatch(0x80001f,{0x10400,0xffff0000,0,0}),-22));memory.read(0x10400,&length);assert(length==0x20000000);
 textDescriptor[0]=0x40000000;textDescriptor[1]=4;textDescriptor[2]=0x10600;word=0;memory.write(0x10600,&word);memory.write(0x10400,textDescriptor,sizeof(textDescriptor));assert(code(kernel.dispatch(0x80001f,{0x10400,0xffff8000,0,0}),0));memory.read(0x10600,&length);memory.read(0x10604,text,sizeof(text));assert(length==4&&std::u16string(text,4)==u"test");
 textDescriptor[2]=0xfffffffe;memory.write(0x10400,textDescriptor,sizeof(textDescriptor));assert(code(kernel.dispatch(0x80001f,{0x10400,0xffff0000,0,0}),-6));
 std::uint32_t named[]={0x30000004,4,0x00450054,0x00540053};memory.write(0x10700,named,sizeof(named));out[1]=4;memory.write(0x10000,out,sizeof(out));
 args[0]=0x10000;args[1]=0;args[2]=0x10700;args[3]=1;memory.write(0x10040,args,sizeof(args));assert(code(exec(0x19),0));std::uint32_t namedPrimary=0;memory.read(0x10100,&namedPrimary);assert(code(exec(0x19),-11));
 std::vector<std::uint32_t> aliases;args[1]=0x10700;memory.write(0x10040,args,sizeof(args));
 for(unsigned n=0;n<64;n++){assert(code(exec(0x1a),0));std::uint32_t alias;memory.read(0x10100,&alias);assert(alias!=namedPrimary);aliases.push_back(alias);}
 assert(code(exec(0x1a),-4));args[0]=namedPrimary;memory.write(0x10040,args,sizeof(args));assert(code(exec(0x10),0));assert(code(kernel.dispatch(0x29,{namedPrimary,0,0,0}),-8)&&code(kernel.dispatch(0x29,{aliases.back(),0,0,0}),1));
 args[0]=0;memory.write(0x10040,args,sizeof(args));assert(code(exec(0x10),-8));
 for(auto alias:aliases){args[0]=alias;memory.write(0x10040,args,sizeof(args));assert(code(exec(0x10),0)&&code(kernel.dispatch(0x29,{alias,0,0,0}),-8));}
 args[0]=0x10000;args[1]=0x10700;memory.write(0x10040,args,sizeof(args));assert(code(exec(0x1a),-1));args[1]=0;memory.write(0x10040,args,sizeof(args));assert(code(exec(0x19),0));
 args[0]=0xffff0001;args[1]=0x10700;memory.write(0x10040,args,sizeof(args));assert(code(exec(0x2c),0)&&kernel.name(false)==u"TEST");
 assert(code(kernel.dispatch(0x53,{'a',0,0,0}),1)&&code(kernel.dispatch(0x53,{'A',0,0,0}),0)&&code(kernel.dispatch(0x53,{' ',0,0,0}),0x70)&&kernel.dispatch(0x53,{0x100,0,0,0}).kind==KernelResult::Kind::Unhandled);
 puts("KERNEL_NEGATIVE_PASS descriptor_capacity invalid_memory stale_handles table_exhaustion no_memory unsupported_calls atomic_wrap");
 puts("THREAD_CONTEXT_NEGATIVE_PASS trap_64_frames duplicate_link_guard TLS_64_limit_overwrite_free scheduler_state process_UTF16_descriptors overflow_ptr_to_buf ASCII_scope");
 puts("NAMED_SYNC_PASS duplicate_create shared_object 64_alias_limit primary_close last_close stale_handle zero_handle namespace_lifetime thread_rename");
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
 if(argc==4&&!std::strcmp(argv[1],"--folder")&&!std::strcmp(argv[3],"--stress-runtime"))return symbianImageProbe(argv[2],stdout,nullptr,nullptr,512)?0:5;
 if(argc==4&&!std::strcmp(argv[1],"--folder")&&(!std::strcmp(argv[3],"--cancel-native")||!std::strcmp(argv[3],"--cancel-files")||!std::strcmp(argv[3],"--cancel-graphics")||!std::strcmp(argv[3],"--cancel-window"))){
  if(!std::strcmp(argv[3],"--cancel-files"))cancelServiceStage="Reading native file size";
  if(!std::strcmp(argv[3],"--cancel-graphics"))cancelServiceStage="Checking native FBS shared chunk";
  if(!std::strcmp(argv[3],"--cancel-window"))cancelServiceStage="Querying native screen pixels";
  FILE*f=std::tmpfile();assert(f);assert(symbianImageProbe(argv[2],f,nativeKeepRunning,nativeCancelProgress));std::rewind(f);char text[16384]={};auto bytes=std::fread(text,1,sizeof(text)-1,f);std::fclose(f);
  assert(bytes&&cancelNative&&std::strstr(text,"DIAGNOSTIC_CANCELLED")&&!std::strstr(text,"APPLICATION_OBJECT")&&!std::strstr(text,"ROM_APPRUN")&&!std::strstr(text,"DIAGNOSTIC_END"));printf("NATIVE_CANCEL_PASS stage=%s no_app_factory no_ROM_process_after_cancel\n",cancelServiceStage?cancelServiceStage:"TRAP cleanup");return 0;
 }
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
 auto kernel=symbianKernelProbe();printf("KERNEL_PROBE chunks=%d handles=%d sync=%d heap=%d abi=%d thumb=%d trap=%d trap_frames=%d tls=%d scheduler=%d named_sync=%d guest_exit=%d\n",kernel.chunks,kernel.handles,kernel.sync,kernel.heap,kernel.abi,kernel.thumb,kernel.trap,kernel.frames,kernel.tls,kernel.scheduler,kernel.namedSync,kernel.lifecycle);fflush(stdout);assert(kernel.passed());
 kernelNegativeChecks();
 fileServerChecks();
 graphicsServerChecks();
 windowServerChecks();
 AddressSpace memory(8192);assert(!memory.map(0,4096,Read));assert(!memory.map(0xfffff000,8192,Read));
 assert(memory.map(0x1000,4096,Read|Write));assert(!memory.map(0x1800,4096,Read));
 assert(!memory.map(0x5000,8192,Read));assert(memory.used()==4096);
 std::uint32_t value=123,out=0;assert(memory.write(0x1ffc,&value));assert(!memory.write(0x1ffd,&value));assert(!memory.read(0xffffffff,&out));
 assert(!memory.permits(0x1000,4,Execute));assert(memory.unmap(0x1000)&&memory.used()==0);
 E32Image image;std::string error;
 const std::vector<std::uint8_t> original(std::begin(symbian_e32_guest),std::end(symbian_e32_guest));
 romLookupChecks();
 romInitializationChecks();
 dependencyChecks(original);
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
