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
 if(!h)return nullptr;for(auto&o:objects)if(o.type==type&&o.handle==h)return &o;
 for(const auto&a:aliases)if(a.handle==h&&objects[a.index].type==type)return &objects[a.index];return nullptr;
}
Eka1Kernel::Object* Eka1Kernel::newObject(){
 if(nextHandle>=0x7fffffff)return nullptr;
 // Callbacks may allocate nested kernel objects before a caller assigns type.
 // A reserved slot already owns a reference and must not be reused then.
 for(auto&o:objects)if(!o.type&&!o.references){o={};o.handle=nextHandle++;o.references=1;return &o;}return nullptr;
}
std::uint32_t Eka1Kernel::newAlias(Object&o){
 if(nextHandle>=0x7fffffff)return 0;
 for(auto&a:aliases)if(!a.handle){a={nextHandle++,static_cast<unsigned>(&o-objects.data())};o.references++;return a.handle;}return 0;
}
std::uint32_t Eka1Kernel::chunkAddress(std::uint32_t size)const{
 std::uint32_t base=0x20000000;
 for(unsigned pass=0;pass<=objects.size();pass++){
  bool overlap=false;
  for(const auto&o:objects)if(o.type==Object::Chunk&&std::uint64_t(base)<std::uint64_t(o.base)+o.maximum&&std::uint64_t(o.base)<std::uint64_t(base)+size){base=o.base+o.maximum;overlap=true;break;}
  if(!overlap)return std::uint64_t(base)+size<=0x40000000?base:0;
 }return 0;
}
bool Eka1Kernel::initializeGraphicsResources(){
 const std::u16string names[]={u"FbsSharedChunk",u"FbsLargeChunk",u"FbsLargeBitmapAccess"};
 for(const auto&o:objects)if(o.type==Object::Chunk&&o.global&&o.name==names[0])return true;
 std::array<Object*,3> created{};std::array<bool,2> mapped{};
 for(unsigned n=0;n<3;n++){
  auto*o=newObject();if(!o)break;created[n]=o;o->name=names[n];o->global=true;
  if(n<2){o->type=Object::Chunk;o->maximum=1024*1024;o->size=n?0:65536;o->base=chunkAddress(o->maximum);
   if(!o->base||!memory.map(o->base,o->maximum,Read|Write))break;mapped[n]=true;memory.commit(o->base,o->size);
  }else{o->type=Object::Mutex;return true;}
 }
 for(unsigned n=0;n<3;n++)if(auto*o=created[n]){if(n<2&&mapped[n])memory.unmap(o->base);*o={};}return false;
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
bool Eka1Kernel::configureProcess(const std::u16string&filename,const std::u16string&command){
 if(filename.empty()||filename.size()>256||command.size()>256||filename.find(char16_t(0))!=filename.npos||command.find(char16_t(0))!=command.npos)return false;
 processFilename=filename;commandLine=command;return true;
}
KernelResult Eka1Kernel::text16(std::uint32_t at,const std::u16string&text){
 std::uint32_t info=0,capacity=0,pointer=0,backing=0,backingInfo=0;
 if(std::uint64_t(at)+12>0x100000000ULL||!memory.read(at,&info)||!memory.permits(at,4,Write)||!memory.read(at+4,&capacity)||(info&0xffffff)>capacity)return returned(argument);
 unsigned type=info>>28;if(type!=2&&type!=3&&type!=4)return returned(argument);
 if(text.size()>capacity)return returned(-22);
 if(type==3)pointer=at+8;
 else if(!memory.read(at+8,&pointer))return returned(argument);
 if(type==4){backing=pointer;if(std::uint64_t(backing)+4>0xffffffffULL||!memory.read(backing,&backingInfo)||(backingInfo>>28)!=0||!memory.permits(backing,4,Write))return returned(argument);pointer=backing+4;}
 if(!text.empty()&&!memory.permits(pointer,text.size()*2,Write))return returned(argument);
 auto length=static_cast<std::uint32_t>(text.size());
 if(!text.empty())memory.write(pointer,text.data(),text.size()*2);
 info=(info&0xff000000)|length;memory.write(at,&info);
 if(type==4){backingInfo=(backingInfo&0xff000000)|length;memory.write(backing,&backingInfo);}
 return returned(0);
}
bool Eka1Kernel::readText16(std::uint32_t at,std::u16string&text){
 std::uint32_t info=0,pointer=0,capacity=0;
 if(std::uint64_t(at)+12>0x100000000ULL||!memory.read(at,&info)||(info&0xffffff)>256)return false;
 auto length=info&0xffffff;unsigned type=info>>28;
 if(type==0)pointer=at+4;
 else if(type==1){if(!memory.read(at+4,&pointer))return false;}
 else if(type==2||type==3||type==4){
  if(!memory.read(at+4,&capacity)||capacity<length)return false;
  if(type==3)pointer=at+8;else if(!memory.read(at+8,&pointer))return false;
  if(type==4){std::uint32_t backing=0;if(std::uint64_t(pointer)+4>0xffffffffULL||!memory.read(pointer,&backing)||(backing>>28)!=0||(backing&0xffffff)<length)return false;pointer+=4;}
 }else return false;
 std::u16string result(length,0);if(length&&!memory.read(pointer,result.data(),length*2))return false;text=std::move(result);return true;
}
KernelResult Eka1Kernel::executor(std::uint32_t attr,std::uint32_t info,std::uint32_t status){
 std::array<std::uint32_t,4> args{};if(!memory.read(info,args.data(),sizeof(args)))return fault(info,Read);
 if(!memory.permits(status,4,Write))return fault(status,Write);
 unsigned operation=attr&255;
 if(attr&~0x400000ffu)return {};
 if(operation==2||operation==0x18){
  std::u16string name;if(!readText16(args[1],name))return finish(status,argument);
  std::uint32_t output=0;if(!package(args[0],4,true,output))return finish(status,argument);
  unsigned type=operation==2?Object::Chunk:Object::Mutex;
  for(auto&o:objects)if(o.type==type&&o.global&&o.name==name){auto h=newAlias(o);if(!h)return finish(status,noMemory);memory.write(output,&h);return finish(status,0);}return finish(status,-1);
 }
 if(operation==0x16){
  std::u16string name;if(!readText16(args[1],name))return finish(status,argument);
  if(name==u"FileServer"?!fileService:!systemService)return {};
  std::uint32_t output=0;if(!package(args[0],4,true,output)||args[2]>64)return finish(status,argument);
  auto*o=newObject();if(!o)return finish(status,noMemory);
  auto result=name==u"FileServer"?fileService(o->handle,-1,{}):systemService(name,o->handle,-1,{});if(result.kind!=KernelResult::Kind::Returned||result.value){*o={};return result.kind==KernelResult::Kind::Returned?finish(status,result.value):result;}
  o->type=Object::Session;o->base=o->handle;o->name=std::move(name);memory.write(output,&o->handle);return finish(status,0);
 }
 if(operation==0x2d||operation==0x21){
  if((args[0]&~0x8000u)!=(operation==0x2d?0xffff0001u:0xffff0000u))return finish(status,badHandle);
  finish(status,0);threadExited=true;threadExitCode=args[1];return {KernelResult::Kind::Exited,args[1],0,0};
 }
 if(operation==0x2c||operation==0x20){
  if((args[0]&~0x8000u)!=(operation==0x2c?0xffff0001u:0xffff0000u))return finish(status,badHandle);
  std::u16string name;if(!readText16(args[1],name))return finish(status,argument);
  (operation==0x2c?threadName:processName)=std::move(name);return finish(status,0);
 }
 // Initial implementation covers anonymous, normal local chunks only.
 if(operation==0){
  if(args[1]!=0xffff0000)return finish(status,notSupported);
  std::uint32_t descriptionAddress=0;if(!package(args[2],12,false,descriptionAddress))return finish(status,argument);
  std::uint32_t description[3];memory.read(descriptionAddress,description,sizeof(description));
  if(description[2])return finish(status,notSupported);
  std::uint32_t initial,maximum;if(!pageRound(description[0],initial)||!pageRound(description[1],maximum)||!maximum||initial>maximum)return finish(status,argument);
  std::uint32_t output=0;if(!package(args[0],4,true,output))return finish(status,argument);
  auto*o=newObject();auto base=chunkAddress(maximum);
  if(!o)return finish(status,noMemory);if(!base||!memory.map(base,maximum,Read|Write)){*o={};return finish(status,noMemory);}
  memory.commit(base,initial);o->type=Object::Chunk;o->base=base;o->maximum=maximum;o->size=initial;
  memory.write(output,&o->handle);return finish(status,0);
 }
 if(operation==3){auto*o=object(args[0],Object::Chunk);if(!o)return finish(status,badHandle);std::uint32_t size;if(!pageRound(args[1],size)||size>o->maximum)return finish(status,noMemory);memory.commit(o->base,size);o->size=size;return finish(status,0);}
 if(operation==0x17||operation==0x19){
  bool global=args[1]!=0xffff0000;std::u16string name;
  if(global&&args[2]&&!readText16(args[2],name))return finish(status,argument);
  std::uint32_t output=0;if(!package(args[0],4,true,output))return finish(status,argument);
  if(operation==0x19&&args[3]>0x7fffffff)return finish(status,argument);
  unsigned type=operation==0x19?Object::Semaphore:Object::Mutex;
  if(global)for(const auto&o:objects)if(o.type==type&&o.global&&o.name==name)return finish(status,-11);
  auto*o=newObject();if(!o)return finish(status,noMemory);o->type=type;o->depth=operation==0x19?args[3]:0;o->global=global;o->name=std::move(name);
  memory.write(output,&o->handle);return finish(status,0);
 }
 if(operation==0x1a){
  std::u16string name;if(!readText16(args[1],name))return finish(status,argument);
  std::uint32_t output=0;if(!package(args[0],4,true,output))return finish(status,argument);
  for(auto&o:objects)if(o.type==Object::Semaphore&&o.global&&o.name==name){auto h=newAlias(o);if(!h)return finish(status,noMemory);memory.write(output,&h);return finish(status,0);}
  return finish(status,-1);
 }
 if(operation==0x10){
  if(!args[0])return finish(status,badHandle);Object*target=nullptr;
  for(auto&o:objects)if(o.type&&o.handle==args[0]){o.handle=0;target=&o;break;}
  if(!target)for(auto&a:aliases)if(a.handle==args[0]){target=&objects[a.index];a={};break;}
  if(!target)return finish(status,badHandle);
  if(!--target->references){if(target->type==Object::Chunk)memory.unmap(target->base);if(target->type==Object::Session){if(target->name==u"FileServer"&&fileService)fileService(target->base,-2,{});else if(systemService)systemService(target->name,target->base,-2,{});}*target={};}return finish(status,0);
 }
 if(operation==0x34){
  for(auto&slot:tls)if(slot.used&&slot.key==args[0]){slot.value=args[1];return finish(status,0);}
  for(auto&slot:tls)if(!slot.used){slot={true,args[0],args[1]};return finish(status,0);}
  return finish(status,noMemory);
 }
 if(operation==0x35){for(auto&slot:tls)if(slot.used&&slot.key==args[0]){slot={};break;}return finish(status,0);}
 return {};
}
KernelResult Eka1Kernel::dispatch(std::uint32_t svc,const std::array<std::uint32_t,4>&a){
 if(threadExited)return {KernelResult::Kind::Exited,threadExitCode,0,0};
 switch(svc){
 case 0xc000bf:case 0xc00031:{
  auto*o=object(a[0],Object::Session);if(!o)return returned(badHandle);
  if(a[3]&&!memory.permits(a[3],4,Write))return fault(a[3],Write);
  std::array<std::uint32_t,4> args{};if(a[2]&&!memory.read(a[2],args.data(),sizeof(args)))return fault(a[2],Read);
  auto result=o->name==u"FileServer"&&fileService?fileService(o->base,a[1],args):systemService?systemService(o->name,o->base,a[1],args):KernelResult{};if(result.kind!=KernelResult::Kind::Returned)return result;
  if(a[3]){finish(a[3],result.value);return returned(0);}return result;
 }
 case 0x6c:return returned(currentHeap);
 case 0x4f:case 0x51:case 0x52:{
  auto c=a[0];if(c>127)return {};if(svc==0x52){if(c>='a'&&c<='z')c-=32;}else if(c>='A'&&c<='Z')c+=32;return returned(c);
 }
 case 0x53:{
  auto c=a[0];if(c>127)return {};if(c<32||c==127)return returned(0x80);if(c==' ')return returned(0x70);
  if(c>='A'&&c<='Z')return returned(0);if(c>='a'&&c<='z')return returned(1);if(c>='0'&&c<='9')return returned(0x40);
  if(c=='_')return returned(0x50);if(c=='-')return returned(0x51);if(c=='('||c=='['||c=='{')return returned(0x52);if(c==')'||c==']'||c=='}')return returned(0x53);
  if(c=='+'||c=='<'||c=='='||c=='>'||c=='|'||c=='~')return returned(0x60);if(c=='$')return returned(0x61);if(c=='^'||c=='`')return returned(0x62);return returned(0x56);
 }
 case 0x8000c0:if((a[0]&~0x8000u)!=0xffff0000)return returned(badHandle);if(processFilename.empty())return {};return returned(commandLine.size());
 case 0x80001e:case 0x80001f:
  if((a[1]&~0x8000u)!=0xffff0000)return returned(badHandle);if(processFilename.empty())return {};
  return text16(a[0],svc==0x80001e?processFilename:commandLine);
 case 0x3c:return returned(pendingRequests);
 case 0x74:return returned(activeScheduler);
 case 0x75:if(a[0]&&!memory.permits(a[0],4,Read|Write))return returned(argument);activeScheduler=a[0];return returned(a[0]);
 case 0x80:for(const auto&slot:tls)if(slot.used&&slot.key==a[0])return returned(slot.value);return returned(0);
 case 0x72:{
  auto frame=a[0];std::uint32_t result=0;
  if((frame&3)||!memory.permits(frame,76,Read|Write)||!memory.read(frame+68,&result)||(result&3)||!memory.permits(result,4,Read|Write))return fault(frame,Read|Write);
  if(trapCount==trapFrames.size())return fault(frame,Write);
  for(unsigned n=0;n<trapCount;n++)if(trapFrames[n]==frame)return fault(frame,Write);
  std::uint32_t previous=trapCount?trapFrames[trapCount-1]:0;
  memory.write(frame+64,&previous);memory.write(frame+72,&trapHandler);
  trapFrames[trapCount++]=frame;return returned(trapHandler);
 }
 case 0x73:{
  if(!trapCount)return returned(0);
  auto frame=trapFrames[trapCount-1];std::uint32_t previous=0;
  if(!memory.permits(frame,76,Read|Write)||!memory.read(frame+64,&previous)||previous!=(trapCount>1?trapFrames[trapCount-2]:0))return fault(frame,Read);
  --trapCount;return returned(frame);
 }
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
