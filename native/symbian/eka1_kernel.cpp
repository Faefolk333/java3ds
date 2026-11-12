// ABI references: pinned EKA2L1 78dbf9fcc kernel/svc.{h,cpp}, system/hal.cpp.
#include "eka1_kernel.h"
namespace symbian {
namespace {
KernelResult returned(std::int32_t value){return {KernelResult::Kind::Returned,static_cast<std::uint32_t>(value),0,0};}
KernelResult fault(std::uint32_t at,unsigned access){return {KernelResult::Kind::Fault,0,at,access};}
constexpr std::int32_t argument=-6,badHandle=-8,noMemory=-4,notSupported=-5;
bool pageRound(std::uint32_t size,std::uint32_t&out){if(size>4*1024*1024)return false;out=(size+4095)&~4095u;return true;}
}
Eka1Kernel::Object* Eka1Kernel::object(std::uint32_t h,unsigned type){
 for(auto&o:objects)if(o.type==type&&o.handle==h)return &o;return nullptr;
}
Eka1Kernel::Object* Eka1Kernel::newObject(){
 if(nextHandle>=0x7fffffff)return nullptr;
 for(auto&o:objects)if(!o.type){o.handle=nextHandle++;return &o;}return nullptr;
}
std::uint32_t Eka1Kernel::chunkAddress(std::uint32_t size)const{
 std::uint32_t base=0x20000000;
 for(unsigned pass=0;pass<=objects.size();pass++){
  bool overlap=false;
  for(const auto&o:objects)if(o.type==Object::Chunk&&std::uint64_t(base)<std::uint64_t(o.base)+o.maximum&&std::uint64_t(o.base)<std::uint64_t(base)+size){base=o.base+o.maximum;overlap=true;break;}
  if(!overlap)return std::uint64_t(base)+size<=0x40000000?base:0;
 }return 0;
}
KernelResult Eka1Kernel::finish(std::uint32_t status,std::int32_t result){
 if(!memory.write(status,&result))return fault(status,Write);pendingRequests++;return returned(result);
}
bool Eka1Kernel::package(std::uint32_t at,std::uint32_t length,bool writable,std::uint32_t&data){
 std::uint32_t header=0,second=0,third=0;if(!memory.read(at,&header))return false;
 unsigned type=header>>28;if(std::uint64_t(at)+12>0x100000000ULL)return false;if(!writable&&(header&0xffffff)<length)return false;
 if(type==0&&!writable)data=at+4;
 else if(type==1&&!writable){if(!memory.read(at+4,&data))return false;}
 else if(type==2){if(!memory.read(at+4,&second)||!memory.read(at+8,&third)||second<length||(header&0xffffff)>second)return false;data=third;}
 else if(type==3){if(!memory.read(at+4,&second)||second<length||(header&0xffffff)>second)return false;data=at+8;}
 else return false;
 return memory.permits(data,length,writable?Read|Write:Read);
}
KernelResult Eka1Kernel::executor(std::uint32_t attr,std::uint32_t info,std::uint32_t status){
 std::array<std::uint32_t,4> args{};if(!memory.read(info,args.data(),sizeof(args)))return fault(info,Read);
 if(!memory.permits(status,4,Write))return fault(status,Write);
 unsigned operation=attr&255;
 if(attr&~0x400000ffu)return {};
 // Initial implementation covers anonymous, normal local chunks only.
 if(operation==0){
  if(args[1]!=0xffff0000)return finish(status,notSupported);
  std::uint32_t descriptionAddress=0;if(!package(args[2],12,false,descriptionAddress))return finish(status,argument);
  std::uint32_t description[3];memory.read(descriptionAddress,description,sizeof(description));
  if(description[2])return finish(status,notSupported);
  std::uint32_t initial,maximum;if(!pageRound(description[0],initial)||!pageRound(description[1],maximum)||!maximum||initial>maximum)return finish(status,argument);
  std::uint32_t output=0;if(!package(args[0],4,true,output))return finish(status,argument);
  auto*o=newObject();auto base=chunkAddress(maximum);
  if(!o||!base||!memory.map(base,maximum,Read|Write))return finish(status,noMemory);
  memory.commit(base,initial);o->type=Object::Chunk;o->base=base;o->maximum=maximum;o->size=initial;
  memory.write(output,&o->handle);return finish(status,0);
 }
 if(operation==3){auto*o=object(args[0],Object::Chunk);if(!o)return finish(status,badHandle);std::uint32_t size;if(!pageRound(args[1],size)||size>o->maximum)return finish(status,noMemory);memory.commit(o->base,size);o->size=size;return finish(status,0);}
 if(operation==0x17||operation==0x19){
  if(args[1]!=0xffff0000)return finish(status,notSupported);
  std::uint32_t output=0;if(!package(args[0],4,true,output))return finish(status,argument);
  if(operation==0x19&&args[3]>0x7fffffff)return finish(status,argument);
  auto*o=newObject();if(!o)return finish(status,noMemory);o->type=operation==0x19?Object::Semaphore:Object::Mutex;o->depth=operation==0x19?args[3]:0;
  memory.write(output,&o->handle);return finish(status,0);
 }
 if(operation==0x10){
  for(auto&o:objects)if(o.type&&o.handle==args[0]){if(o.type==Object::Chunk)memory.unmap(o.base);o={};return finish(status,0);}return finish(status,badHandle);
 }
 return {};
}
KernelResult Eka1Kernel::dispatch(std::uint32_t svc,const std::array<std::uint32_t,4>&a){
 switch(svc){
 case 0x6c:return returned(currentHeap);
 case 0x81:return returned(trapHandler);
 case 0x82:{if(a[0]&&!memory.permits(a[0],4,Read|Write))return returned(argument);auto previous=trapHandler;trapHandler=a[0];return returned(previous);}
 case 0xc0006d:{auto old=currentHeap;if(a[0]&&!memory.permits(a[0],4,Read|Write))return returned(argument);currentHeap=a[0];return returned(old);}
 case 0x800083:{
  if(a[0]!=8)return returned(notSupported);
  std::uint32_t page=4096;if(!memory.write(a[1],&page))return fault(a[1],Write);return returned(0);
 }
 case 0x01:case 0x02:case 0x03:{auto*o=object(a[0],Object::Chunk);if(!o)return returned(badHandle);return returned(svc==1?o->base:svc==2?o->size:o->maximum);}
 case 0x4d:if(pendingRequests){pendingRequests--;return returned(0);}return {KernelResult::Kind::Blocked,0,0,0};
 case 0x18:case 0x19:case 0x1a:{
  auto*o=object(a[0],Object::Mutex);if(!o)return returned(badHandle);
  if(svc==0x18)return returned(o->depth?0:1);
  if(svc==0x19){if(o->depth==0x7fffffff)return returned(argument);o->depth++;return returned(0);}
  if(!o->depth)return returned(argument);o->depth--;return returned(0);
 }
 case 0x29:case 0x2a:case 0xc0002b:case 0x80002c:{
  auto*o=object(a[0],Object::Semaphore);if(!o)return returned(badHandle);
  if(svc==0x29)return returned(o->depth);
  if(svc==0x2a){if(!o->depth)return {KernelResult::Kind::Blocked,0,0,0};o->depth--;return returned(0);}
  auto count=svc==0x80002c?a[1]:1u;if(count>std::uint32_t(0x7fffffff-o->depth))return returned(argument);o->depth+=count;return returned(0);
 }
 case 0x8d:case 0x8e:{std::uint32_t value;if(!memory.read(a[0],&value)||!memory.permits(a[0],4,Write))return fault(a[0],Read|Write);auto before=value;value+=svc==0x8d?1:-1;memory.write(a[0],&value);return returned(before);}
 case 0x8000a8:return returned(a[0]); // void HeapCreated notification; no guest state changes.
 case 0xc00076:return executor(a[0],a[1],a[2]);
 default:return {};
 }
}
}
