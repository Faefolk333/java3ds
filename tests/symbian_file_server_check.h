#pragma once
#include "../native/symbian/file_server.h"
#include "../native/symbian/guest_descriptor.h"
#include "../native/symbian/graphics_server.h"
static void fileServerChecks(){
 using namespace symbian;namespace fs=std::filesystem;
 const fs::path folder="work/symbian-file-server-check";fs::create_directories(folder);auto path=(folder/"original.bin").string();
 {std::ofstream f(path,std::ios::binary);for(unsigned n=0;n<513;n++)f.put(char(n));assert(f.good());}
 AddressSpace m;assert(m.map(0x1000,8192,Read|Write));RomImage rom;FileServer server(m,rom);Eka1Kernel kernel(m);
 assert(server.mountApplication(path));assert(!server.mountApplication(path));
 kernel.fileService=[&](auto h,auto op,const auto&a){return server.request(h,op,a);};
 auto word=[&](unsigned at,unsigned value){assert(m.write(at,&value));};
 auto get=[&](unsigned at){unsigned value=0;assert(m.read(at,&value));return value;};
 auto text=[&](unsigned at,const std::u16string&value){word(at,value.size());assert(m.write(at+4,value.data(),value.size()*2));};
 auto buf=[&](unsigned at,unsigned pointer,unsigned size){word(at,0x20000000);word(at+4,size);word(at+8,pointer);};
 auto exec=[&](unsigned op,std::array<unsigned,4>a){assert(m.write(0x1100,a.data(),16));return kernel.dispatch(0xc00076,{op,0x1100,0x1120,0});};
 text(0x1200,u"FileServer");buf(0x1300,0x1320,4);
 assert(exec(0x16,{0x1300,0x1200,4,0}).value==0);auto session=get(0x1320);assert(session&&get(0x1120)==0);
 text(0x1400,u"e:/system/apps/java3ds/test.app");buf(0x1500,0x1520,4);buf(0x1600,0x1700,600);
 auto ipc=[&](unsigned op,std::array<unsigned,4>a,unsigned status=0x1180){assert(m.write(0x1140,a.data(),16));word(0x1180,0x80000001);auto result=kernel.dispatch(0xc000bf,{session,op,0x1140,status});assert(result.kind==KernelResult::Kind::Returned);return status?get(status):result.value;};
 assert(ipc(27,{0x1400,0,0,0x1500})==0);auto file=get(0x1520);assert(file);
 assert(ipc(37,{0x1500,0,0,file})==0&&get(0x1520)==513);
 assert(ipc(31,{0x1600,12,0xffffffff,file})==0&&(get(0x1600)&0xffffff)==12&&get(0x1700)==0x03020100);
 assert(ipc(35,{std::uint32_t(-3),3,0x1500,file})==0&&get(0x1520)==510);
 assert(ipc(31,{0x1600,20,0x80000000,file})==0&&(get(0x1600)&0xffffff)==3);
 assert(ipc(31,{0x1600,20,999,file})==0&&(get(0x1600)&0xffffff)==0);
 assert(ipc(31,{0x1600,0xffffffff,0,file})==std::uint32_t(-6));
 // Overflow and wrapped guest pointers must not modify the output header/data.
 buf(0x1600,0x1700,2);word(0x1700,0x11223344);assert(ipc(31,{0x1600,3,0,file})==std::uint32_t(-22));assert(get(0x1600)==0x20000000&&get(0x1700)==0x11223344);
 buf(0x1600,0xfffffffe,12);assert(ipc(31,{0x1600,12,0,file})==std::uint32_t(-22));
 buf(0x1600,0x1700,600);assert(ipc(31,{0x1600,4,0,file},0)==0);
 assert(ipc(26,{0,0,0,file})==0);assert(ipc(31,{0x1600,4,0,file})==std::uint32_t(-8));
 for(const auto*bad:{u"E:/../test.app",u"E:/system/apps/java3ds/*.app",u"E:/system/apps/java3ds/C:test.app",u"E:/system//test.app"}){text(0x1400,bad);assert(ipc(27,{0x1400,0,0,0x1500})==std::uint32_t(-28));}
 text(0x1400,u"E:/system/apps/java3ds/test.app");assert(ipc(27,{0x1400,0x200,0,0x1500})==std::uint32_t(-21));
 assert(ipc(64,{0x1400,0x1500,0,0})==0&&get(0x1520)==0);
 text(0x1400,u"Z:/missing.file");assert(ipc(64,{0x1400,0x1500,0,0})==0&&get(0x1520)==0);text(0x1400,u"E:/system/apps/java3ds/test.app");
 std::array<unsigned,4> args{};assert(m.write(0x1140,args.data(),16));word(0x1180,0x12345678);auto unsupported=kernel.dispatch(0xc000bf,{session,255,0x1140,0x1180});assert(unsupported.kind==KernelResult::Kind::Unhandled&&get(0x1180)==0x12345678&&server.trace.unsupported);
 // Invalid completion pointer is rejected before opening any file.
 args={0x1400,0,0,0x1500};m.write(0x1140,args.data(),16);auto opens=server.trace.opens;assert(kernel.dispatch(0xc00031,{session,27,0x1140,0xffffffff}).kind==KernelResult::Kind::Fault&&server.trace.opens==opens);
 auto pending=kernel.dispatch(0x3c,{}).value;
 assert(kernel.dispatch(0xc00031,{session,27,0x1140,0x1180}).value==0&&get(0x1180)==0&&kernel.dispatch(0x3c,{}).value==pending+1);file=get(0x1520);assert(ipc(26,{0,0,0,file})==0);
 // File and session quotas are reclaimable; closing a session closes its files.
 for(unsigned n=0;n<32;n++)assert(ipc(27,{0x1400,0,0,0x1500})==0);
 assert(ipc(27,{0x1400,0,0,0x1500})==std::uint32_t(-4));
 assert(exec(0x10,{session,0,0,0}).value==0);assert(kernel.dispatch(0xc000bf,{session,31,0x1140,0x1180}).value==std::uint32_t(-8));
 assert(exec(0x16,{0x1300,0x1200,4,0}).value==0);session=get(0x1320);assert(ipc(27,{0x1400,0,0,0x1500})==0);
 for(unsigned n=1;n<8;n++)assert(exec(0x16,{0x1300,0x1200,4,0}).value==0);
 assert(exec(0x16,{0x1300,0x1200,4,0}).value==std::uint32_t(-4));
 // HBufC-backed writable descriptor updates its backing length too.
 word(0x1800,0x40000000);word(0x1804,4);word(0x1808,0x1820);word(0x1820,0);GuestDescriptor d(m);assert(d.view(0x1800,1,true));unsigned value=0x12345678;assert(d.write(&value,4)&&get(0x1820)==4&&get(0x1824)==value);
 puts("FILESERVER_PASS old_IPC_four_words completion readonly_mount read seek EOF bounds path_guards stale_handles quotas session_teardown HBufC_length");
}
static void graphicsServerChecks(){
 using namespace symbian;
 AddressSpace m;assert(m.map(0x1000,8192,Read|Write));Eka1Kernel kernel(m);GraphicsServer graphics(m);
 kernel.systemService=[&](const auto&name,auto handle,auto op,const auto&a){
  if(name==u"Fontbitmapserver"&&op==-1)assert(kernel.initializeGraphicsResources());return graphics.request(name,handle,op,a);
 };
 auto word=[&](unsigned at,unsigned v){assert(m.write(at,&v));};auto get=[&](unsigned at){unsigned v=0;assert(m.read(at,&v));return v;};
 auto text=[&](unsigned at,const std::u16string&v){word(at,v.size());assert(m.write(at+4,v.data(),v.size()*2));};
 word(0x1100,0x20000000);word(0x1104,4);word(0x1108,0x1120);
 auto exec=[&](unsigned op,std::array<unsigned,4>a){m.write(0x1140,a.data(),16);return kernel.dispatch(0xc00076,{op,0x1140,0x1160,0});};
 text(0x1200,u"Fontbitmapserver");assert(exec(0x16,{0x1100,0x1200,0,0}).value==0);auto fbs=get(0x1120);
 // Nested creation must reserve the session's slot before its server creates
 // two named chunks and a mutex; their aliases must identify distinct objects.
 text(0x1200,u"FbsSharedChunk");assert(exec(2,{0x1100,0x1200,0,0}).value==0);auto shared=get(0x1120);assert(shared!=fbs);
 auto base=kernel.dispatch(1,{shared,0,0,0}).value;assert(base&&m.permits(base,65536,Read|Write));
 auto used=m.used();assert(kernel.initializeGraphicsResources()&&m.used()==used);
 assert(exec(0x10,{shared,0,0,0}).value==0);assert(exec(2,{0x1100,0x1200,0,0}).value==0);assert(kernel.dispatch(1,{get(0x1120),0,0,0}).value==base);
 text(0x1200,u"FbsLargeBitmapAccess");assert(exec(0x18,{0x1100,0x1200,0,0}).value==0);auto mutex=get(0x1120);assert(kernel.dispatch(0x19,{mutex,0,0,0}).value==0&&kernel.dispatch(0x1a,{mutex,0,0,0}).value==0);
 assert(graphics.request(u"Fontbitmapserver",fbs,11,{1,1,0,0}).value==std::uint32_t(-18));assert(graphics.request(u"Fontbitmapserver",fbs,0,{}).value==0);assert(graphics.request(u"Fontbitmapserver",fbs,11,{12,13,0,0}).value==0&&graphics.twipsX==12&&graphics.twipsY==13);
 assert(graphics.request(u"Fontbitmapserver",fbs,14,{}).kind==KernelResult::Kind::Unhandled);
 text(0x1200,u"Windowserver");assert(exec(0x16,{0x1100,0x1200,0,0}).value==0);auto ws=get(0x1120);
 assert(graphics.request(u"Windowserver",ws,2,{}).value==ws);
 word(0x1300,0x10000008);word(0x1304,0x1400);word(0x1400,0x00008019);word(0x1404,ws);
 assert(graphics.request(u"Windowserver",ws,4,{0x1300,0,0,0}).kind==KernelResult::Kind::Unhandled&&graphics.windowCommand==25&&graphics.windowObject==ws);
 word(0x1400,0x10008000);assert(graphics.request(u"Windowserver",ws,4,{0x1300,0,0,0}).value==std::uint32_t(-6));
 word(0x1400,0x00008000);assert(graphics.request(u"Windowserver",ws,4,{0x1300,0,0,0}).value==0);assert(graphics.request(u"Windowserver",ws,4,{0x1300,0,0,0}).value==std::uint32_t(-18));
 GuestDescriptor d(m);assert(d.view(0x1300,1,false));unsigned data=0;assert(!d.write(&data,4));word(0x1300,0x31000000);assert(!d.view(0x1300,1,true));
 // Failed service setup must roll back only regions it actually mapped,
 // preserve an unrelated collision and release reservations for later use.
 AddressSpace collision(3*1024*1024);assert(collision.map(0x20000000,4096,Read));Eka1Kernel blocked(collision);
 for(unsigned n=0;n<80;n++)assert(!blocked.initializeGraphicsResources()&&collision.used()==4096&&collision.permits(0x20000000,4096,Read));
 AddressSpace small(1536*1024);Eka1Kernel limited(small);for(unsigned n=0;n<80;n++)assert(!limited.initializeGraphicsResources()&&small.used()==0);
 puts("GRAPHICS_SERVICE_PASS nested_object_reservation shared_chunks aliases mutex FBS_state WindowServer_init bounded_command_parser fail_closed rollback");
}
