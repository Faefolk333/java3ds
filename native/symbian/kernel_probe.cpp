#include "kernel_probe.h"
#include "eka1_kernel.h"
#include "e32_session.h"
#include "e32_guest.h"
#include "kernel_guest.h"
SymbianKernelResult symbianKernelProbe(){
 using namespace symbian;SymbianKernelResult result;
 AddressSpace memory(1024*1024);if(!memory.map(0x10000,4096,Read|Write))return result;
 Eka1Kernel kernel(memory);constexpr std::uint32_t output=0x10000,description=0x10020,info=0x10040,status=0x10080,valueAt=0x10100;
 std::uint32_t out[]={0x20000000,4,valueAt},desc[]={0x3000000c,12,4096,8192,0},args[]={output,0xffff0000,description,0};
 memory.write(output,out,sizeof(out));memory.write(description,desc,sizeof(desc));memory.write(info,args,sizeof(args));
 auto exec=[&](unsigned op){return kernel.dispatch(0xc00076,{op,info,status,0});};
 auto returned=[&](KernelResult r,std::int32_t value){return r.kind==KernelResult::Kind::Returned&&r.value==std::uint32_t(value);};
 result.trap=returned(kernel.dispatch(0x81,{}),0)&&returned(kernel.dispatch(0x82,{valueAt,0,0,0}),0)&&
  returned(kernel.dispatch(0x81,{}),valueAt)&&returned(kernel.dispatch(0x82,{0xffffffff,0,0,0}),-6)&&kernel.trap()==valueAt&&
  returned(kernel.dispatch(0x82,{}),valueAt)&&returned(kernel.dispatch(0x81,{}),0);
 if(!returned(exec(0),0))return result;std::uint32_t h=0;memory.read(valueAt,&h);
 auto base=kernel.dispatch(1,{h,0,0,0}).value;std::uint32_t word=0x12345678,zero=1;
 bool initial=memory.permits(base,4096,Read|Write)&&!memory.permits(base+4096,4,Read);
 args[0]=h;args[1]=8192;memory.write(info,args,sizeof(args));bool grown=returned(exec(3),0)&&memory.write(base+4096,&word);
 args[1]=4096;memory.write(info,args,sizeof(args));bool shrunk=returned(exec(3),0)&&!memory.permits(base+4096,4,Read);
 args[1]=8192;memory.write(info,args,sizeof(args));bool regrown=returned(exec(3),0)&&memory.read(base+4096,&zero)&&zero==0;
 args[1]=16384;memory.write(info,args,sizeof(args));bool limit=returned(exec(3),-4)&&kernel.dispatch(2,{h,0,0,0}).value==8192;
 result.chunks=initial&&grown&&shrunk&&regrown&&limit;
 result.heap=returned(kernel.dispatch(0x6c,{}),0)&&returned(kernel.dispatch(0xc0006d,{base,0,0,0}),0)&&
  kernel.dispatch(0x6c,{}).value==base&&returned(kernel.dispatch(0xc0006d,{0xffffffff,0,0,0}),-6)&&kernel.heap()==base;
 kernel.dispatch(0xc0006d,{});
 args[0]=h;memory.write(info,args,sizeof(args));bool closed=returned(exec(0x10),0)&&!memory.permits(base,4,Read);
 args[0]=output;args[1]=0xffff0000;memory.write(info,args,sizeof(args));bool recreated=returned(exec(0),0);std::uint32_t fresh=0;memory.read(valueAt,&fresh);
 result.handles=closed&&recreated&&fresh!=h&&returned(kernel.dispatch(1,{h,0,0,0}),-8)&&kernel.dispatch(1,{fresh,0,0,0}).value==base;
 args[0]=fresh;memory.write(info,args,sizeof(args));exec(0x10);
 args[0]=output;args[1]=0xffff0000;args[3]=1;memory.write(info,args,sizeof(args));bool created=returned(exec(0x19),0);memory.read(valueAt,&h);
 bool sem=created&&returned(kernel.dispatch(0x2a,{h,0,0,0}),0)&&kernel.dispatch(0x2a,{h,0,0,0}).kind==KernelResult::Kind::Blocked&&
  returned(kernel.dispatch(0xc0002b,{h,0,0,0}),0)&&returned(kernel.dispatch(0x29,{h,0,0,0}),1);
 args[0]=h;memory.write(info,args,sizeof(args));exec(0x10);
 args[0]=output;memory.write(info,args,sizeof(args));bool mutex=returned(exec(0x17),0);memory.read(valueAt,&h);
 mutex=mutex&&returned(kernel.dispatch(0x19,{h,0,0,0}),0)&&returned(kernel.dispatch(0x19,{h,0,0,0}),0)&&
  returned(kernel.dispatch(0x18,{h,0,0,0}),0)&&returned(kernel.dispatch(0x1a,{h,0,0,0}),0)&&
  returned(kernel.dispatch(0x18,{h,0,0,0}),0)&&returned(kernel.dispatch(0x1a,{h,0,0,0}),0)&&returned(kernel.dispatch(0x18,{h,0,0,0}),1);
 unsigned consumed=0;while(consumed<128&&kernel.dispatch(0x4d,{}).kind==KernelResult::Kind::Returned)consumed++;
 result.sync=sem&&mutex&&consumed>0&&consumed<128&&kernel.dispatch(0x4d,{}).kind==KernelResult::Kind::Blocked;
 std::uint32_t resultPointer=valueAt;memory.write(0x10300+68,&resultPointer);memory.write(0x10380+68,&resultPointer);
 result.frames=returned(kernel.dispatch(0x72,{0x10300,0,0,0}),0)&&returned(kernel.dispatch(0x72,{0x10380,0,0,0}),0)&&
  kernel.dispatch(0x72,{0x10300,0,0,0}).kind==KernelResult::Kind::Fault&&returned(kernel.dispatch(0x73,{}),0x10380)&&returned(kernel.dispatch(0x73,{}),0x10300)&&kernel.trapDepth()==0;
 args[0]=0x101f1234;args[1]=valueAt;memory.write(info,args,sizeof(args));
 result.tls=returned(exec(0x34),0)&&returned(kernel.dispatch(0x80,{args[0],0,0,0}),valueAt)&&returned(exec(0x35),0)&&returned(kernel.dispatch(0x80,{args[0],0,0,0}),0);
 result.scheduler=returned(kernel.dispatch(0x74,{}),0)&&returned(kernel.dispatch(0x75,{valueAt,0,0,0}),valueAt)&&
  returned(kernel.dispatch(0x74,{}),valueAt)&&returned(kernel.dispatch(0x75,{0xffffffff,0,0,0}),-6)&&kernel.scheduler()==valueAt;
 kernel.dispatch(0x75,{});
 std::uint32_t name[]={0x30000004,4,0x00450054,0x00540053};memory.write(0x10400,name,sizeof(name));args[0]=output;args[1]=0;args[2]=0x10400;args[3]=1;memory.write(info,args,sizeof(args));
 bool named=returned(exec(0x19),0);std::uint32_t primary=0,alias=0;memory.read(valueAt,&primary);named=named&&returned(exec(0x19),-11);
 args[1]=0x10400;memory.write(info,args,sizeof(args));named=named&&returned(exec(0x1a),0);memory.read(valueAt,&alias);
 args[0]=primary;memory.write(info,args,sizeof(args));named=named&&returned(exec(0x10),0)&&returned(kernel.dispatch(0x29,{alias,0,0,0}),1)&&returned(kernel.dispatch(0x29,{primary,0,0,0}),-8);
 args[0]=alias;memory.write(info,args,sizeof(args));result.namedSync=named&&returned(exec(0x10),0)&&returned(kernel.dispatch(0x29,{alias,0,0,0}),-8);
 E32Image image;std::string error;if(!image.parse({std::begin(symbian_e32_guest),std::end(symbian_e32_guest)},error))return result;
 E32Session session;if(!session.load(image,0x12000000,0x02000000,error)||!session.memory().map(0x30000000,sizeof(symbian_kernel_guest),Read|Execute)||!session.memory().map(0x31000000,4096,Read|Write))return result;
 session.memory().initialize(0x30000000,symbian_kernel_guest,sizeof(symbian_kernel_guest));session.enableEka1Kernel();
 auto switched=session.call(0x30000200,0x31000000);auto r=session.call(0x30000100);
 auto page=session.callArgs(0x30000300,{8,0x31000004,0,0});std::uint32_t pageSize=0;session.memory().read(0x31000004,&pageSize);
 auto unsupported=session.call(0x30000004);
 result.abi=switched.reason==StopReason::Returned&&switched.r0==0&&r.reason==StopReason::Returned&&r.r0==0x31000000&&
  page.reason==StopReason::Returned&&page.r0==0&&pageSize==4096&&unsupported.reason==StopReason::SystemCallRequired&&unsupported.systemCall==0xeeeeee;
 r=session.call(0x30000601);result.thumb=r.reason==StopReason::Returned&&r.r0==0x31000000;
 std::uint32_t semOut[]={0x20000000,4,0x31000180},semInfo[]={0x31000100,0xffff0000,0,0};
 session.memory().write(0x31000100,semOut,sizeof(semOut));session.memory().write(0x31000140,semInfo,sizeof(semInfo));
 auto semCreate=session.callArgs(0x30000500,{0x19,0x31000140,0x31000200,0});std::uint32_t semHandle=0;session.memory().read(0x31000180,&semHandle);
 auto wait=session.call(0x30000400,semHandle);result.sync=result.sync&&semCreate.reason==StopReason::Returned&&wait.reason==StopReason::ThreadBlocked;
 std::uint32_t exitArgs[]={0xffff8001,static_cast<std::uint32_t>(-42),0,0};session.memory().write(0x31000140,exitArgs,sizeof(exitArgs));
 auto exit=session.callArgs(0x30000500,{0x2d,0x31000140,0x31000200,0});auto afterExit=session.call(0x30000100);
 result.lifecycle=exit.reason==StopReason::ThreadExited&&exit.r0==std::uint32_t(-42)&&afterExit.reason==StopReason::ThreadExited&&afterExit.r0==exit.r0&&!afterExit.instructions;
 return result;
}
