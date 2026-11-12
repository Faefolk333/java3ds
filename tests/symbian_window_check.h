#pragma once
#include "../native/symbian/window_server.h"
static void windowServerChecks(){
 using namespace symbian;AddressSpace memory;assert(memory.map(0x1000,8192,Read|Write));WindowServer server(memory);
 assert(!server.configure({0,208,12})&&!server.configure({176,208,13}));assert(server.configure({176,208,12}));
 auto word=[&](unsigned at,unsigned v){assert(memory.write(at,&v));};auto get=[&](unsigned at){unsigned v=0;assert(memory.read(at,&v));return v;};
 const unsigned session=0x101;assert(server.request(session,-1,{}).value==0&&server.request(session,2,{}).value==session);assert(!server.configure({320,240,16}));
 auto output=[&](unsigned capacity){word(0x1400,0x20000000);word(0x1404,capacity);word(0x1408,0x1500);};output(64);
 auto command=[&](unsigned op,unsigned object,const std::vector<std::int32_t>&args,bool explicitObject=true){
  std::vector<std::uint32_t> data{(op|(explicitObject?0x8000:0))|std::uint32_t(args.size()*4)<<16};if(explicitObject)data.push_back(object);for(auto v:args)data.push_back(v);
  word(0x1100,0x10000000|data.size()*4);word(0x1104,0x1200);assert(memory.write(0x1200,data.data(),data.size()*4));return server.request(session,4,{0x1100,0x1400,0,0});
 };
 auto made=command(25,session,{});assert(made.kind==KernelResult::Kind::Returned&&made.value==0x10001);auto screen=made.value;
 assert(command(3,screen,{}).value==0&&get(0x1500)==176&&get(0x1504)==208&&(get(0x1400)&0xffffff)==8);
 assert(command(2,screen,{},false).value==0&&get(0x1500)==2640&&get(0x1504)==3120);
 assert(command(16,screen,{}).value==0&&(get(0x1400)&0xffffff)==20&&get(0x1508)==2640&&get(0x1510)==0);
 assert(command(17,screen,{}).value==0&&(get(0x1400)&0xffffff)==12&&get(0x1508)==0);
 assert(command(9,screen,{}).value==10&&command(26,screen,{}).value==4096&&command(18,screen,{}).value==1);
 assert(command(19,screen,{0}).value==0&&command(19,screen,{1}).value==std::uint32_t(-6));
 assert(command(22,screen,{2}).value==0&&command(23,screen,{}).value==2);
 assert(command(5,screen,{-30}).value==std::uint32_t(-2)&&command(8,screen,{-2}).value==std::uint32_t(-30));assert(command(8,screen,{INT32_MAX}).value==std::uint32_t(-6));
 assert(command(15,screen,{176,208,0}).value==0&&command(15,screen,{208,176,0}).value==std::uint32_t(-5));
 assert(command(20,screen,{0}).value==0&&command(21,screen,{2}).value==std::uint32_t(-6));
 assert(command(40,screen,{}).value==1&&get(0x1500)==0);assert(command(25,screen,{0}).value==1);
 assert(command(32,screen,{0}).value==0&&get(0x1500)==1&&get(0x1504)==1);assert(command(30,screen,{}).value==0&&get(0x1500)==0);
 auto queries=server.queries;output(4);word(0x1500,0x11223344);assert(command(3,screen,{}).value==std::uint32_t(-6)&&server.queries==queries&&get(0x1500)==0x11223344&&get(0x1400)==0x20000000);output(64);
 assert(command(1,screen,{}).kind==KernelResult::Kind::Unhandled);assert(command(3,screen,{1}).value==std::uint32_t(-6));
 // A valid free followed by an unsupported command is one unapplied batch.
 std::uint32_t batch[]={0x8004,screen,0x8063,session};word(0x1100,0x10000010);memory.write(0x1200,batch,sizeof(batch));assert(server.request(session,0,{0x1100,0x1400,0,0}).kind==KernelResult::Kind::Unhandled);assert(command(3,screen,{}).value==0);
 // A malformed trailing command must not apply an earlier valid free either.
 batch[2]=0x10008004;word(0x1100,0x10000010);memory.write(0x1200,batch,sizeof(batch));assert(server.request(session,0,{0x1100,0x1400,0,0}).value==std::uint32_t(-6));assert(command(3,screen,{}).value==0);
 assert(server.request(0x102,-1,{}).value==0&&server.request(0x102,2,{}).value==0x102);word(0x1100,0x10000010);assert(server.request(0x102,0,{0x1100,0x1400,0,0}).value==std::uint32_t(-6));
 command(3,screen,{});assert(server.request(0x102,0,{0x1100,0x1400,0,0}).value==std::uint32_t(-8));
 // The original EPOC6 RWindowTreeNode table omits AbsPosition. Names use
 // UTF-16 reply descriptors and the legacy pointer/remote-descriptor forms.
 auto madeGroup=command(21,session,{123,1}).value;assert(madeGroup>screen&&command(87,madeGroup,{}).value==madeGroup);assert(command(38,session,{}).value==madeGroup);
 std::u16string title=u"Screen test";word(0x1700,title.size());memory.write(0x1704,title.data(),title.size()*2);
 assert(command(61,madeGroup,{int(title.size()),0x1700}).value==0);assert(command(60,madeGroup,{64}).value==0&&(get(0x1400)&0xffffff)==title.size());std::u16string readback(title.size(),0);memory.read(0x1500,readback.data(),readback.size()*2);assert(readback==title);
 assert(command(61,madeGroup,{500,0x1700}).value==std::uint32_t(-6));assert(command(60,madeGroup,{}).value==0&&(get(0x1400)&0xffffff)==title.size());
 assert(command(63,madeGroup,{}).value==0&&command(39,session,{}).value==madeGroup);assert(command(0,madeGroup,{}).value==0&&command(87,madeGroup,{}).value==std::uint32_t(-8));
 for(unsigned n=1;n<32;n++)assert(command(25,session,{}).value>=0x10001);
 assert(command(25,session,{}).value==std::uint32_t(-4));assert(command(4,screen,{}).value==0);auto newer=command(25,session,{}).value;assert(newer>screen&&newer<0x7fffffff);assert(command(3,screen,{}).value==std::uint32_t(-8));
 assert(server.request(session,-2,{}).value==0&&server.request(session,2,{}).value==std::uint32_t(-8));assert(server.request(session+2,-1,{}).value==0&&server.request(session+2,2,{}).value==session+2);
 puts("WINDOW_SCREEN_PASS ROM_config pixel_twips_color_mode native_handle_lifetime ownership quotas stale_ids descriptor_bounds integer_overflow atomic_batches fail_closed");
}
