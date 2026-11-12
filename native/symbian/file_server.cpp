// ABI: pinned EKA2L1 services/fs and kernel/svc EKA1 four-word IPC paths.
#include "file_server.h"
#include "guest_descriptor.h"
#include <algorithm>
#include <cstring>
namespace symbian {
namespace {KernelResult done(int value){return {KernelResult::Kind::Returned,std::uint32_t(value),0,0};}}
FileServer::~FileServer(){for(auto&f:files)close(f);}
void FileServer::close(File&f){if(f.stream)std::fclose(f.stream);f={};}
bool FileServer::mountApplication(const std::string&path){
 if(!application.empty()||path.empty())return false;auto*f=std::fopen(path.c_str(),"rb");if(!f)return false;std::fclose(f);application=path;return true;
}
bool FileServer::normalize(const std::u16string&base,const std::u16string&name,std::u16string&out)const{
 if(name.empty()||name.size()>256)return false;std::u16string value=name;
 for(auto&c:value){if(c=='/')c='\\';if(c<32||c>126||c=='*'||c=='?'||c=='"'||c=='<'||c=='>'||c=='|')return false;if(c>='a'&&c<='z')c-=32;}
 if(value.size()>=2&&value[1]==':'){if(value[0]<'A'||value[0]>'Z'||value.size()<3||value[2]!='\\')return false;}
 else if(value[0]=='\\')value=base.substr(0,2)+value;else value=base+value;
 if(value.size()>256)return false;
 std::size_t begin=3;while(begin<value.size()){auto end=value.find('\\',begin);if(end==value.npos)end=value.size();auto part=value.substr(begin,end-begin);if(part.empty()||part==u"."||part==u".."||part.find(':')!=part.npos)return false;begin=end+1;}
 out=std::move(value);return true;
}
int FileServer::open(const std::u16string&path,File&f){
 if(path.size()<4||path.back()=='\\')return -28;
 if(path[0]=='Z'){
  std::string narrow(path.begin(),path.end()),error;RomFile rf;if(!rom.findFile(memory,narrow,rf,error))return -1;f.address=rf.address;f.size=rf.size;
 }else if(path==u"E:\\SYSTEM\\APPS\\JAVA3DS\\TEST.APP"&&!application.empty()){
  f.stream=std::fopen(application.c_str(),"rb");if(!f.stream)return -1;
  if(std::fseek(f.stream,0,SEEK_END)){close(f);return -20;}auto size=std::ftell(f.stream);
  if(size<0||std::uint64_t(size)>0x7fffffff){close(f);return -20;}f.size=size;std::rewind(f.stream);
 }else return path[0]=='C'||path[0]=='E'?-1:-18;
 f.path=path;return 0;
}
KernelResult FileServer::request(std::uint32_t handle,std::int32_t operation,const std::array<std::uint32_t,4>&a){
 if(operation==-1){for(auto&s:sessions)if(!s.handle){s={};s.handle=handle;return done(0);}return done(-4);}
 Session*s=nullptr;for(auto&candidate:sessions)if(handle&&candidate.handle==handle)s=&candidate;if(!s)return done(-8);
 if(operation==-2){for(auto&f:files)if(f.owner==handle)close(f);*s={};return done(0);}
 trace.calls++;trace.operation=operation;trace.path.clear();trace.unsupported=false;
 auto finish=[&](int value){trace.result=value;return done(value);};
 auto unknown=[&](){trace.unsupported=true;return KernelResult{};};
 // EPOC6 has a single subsession-close opcode, preceding FileOpen.
 if(operation<0||operation>255)return unknown();unsigned op=operation;if(op>26)op+=3;if(op>69)op--;
 GuestDescriptor descriptor(memory);std::u16string name,path;
 auto inputPath=[&](unsigned slot){if(!descriptor.text(a[slot],name)||!normalize(s->path,name,path))return false;trace.path=path;return true;};
 auto output=[&](unsigned slot,const void*data,unsigned bytes){GuestDescriptor out(memory);return out.view(a[slot],1,true)&&out.write(data,bytes);};
 auto outputText=[&](unsigned slot,const std::u16string&text){GuestDescriptor out(memory);return out.view(a[slot],2,true)&&out.write(text.data(),text.size());};
 if(op==12||op==14)return finish(outputText(0,op==12?u"C:\\":s->path)?0:-6);
 if(op==15){if(!inputPath(0)||path.back()!='\\')return finish(-28);s->path=path;return finish(0);}
 if(op==65)return finish(s->notify?1:0);
 if(op==66){s->notify=bool(a[0]);return finish(0);}
 if(op==5){std::uint8_t drives[26]{};drives[2]=0x11;drives[4]=0x20;drives[25]=0x12;return finish(output(0,drives,sizeof(drives))?0:-6);}
 if(op==30){
  if(!inputPath(0))return finish(-28);
  if(a[1]&~0x603u)return finish(-5);if(a[1]&0x200)return finish(-21);
  GuestDescriptor out(memory);if(!out.view(a[3],1,true)||!out.canWrite(4))return finish(-6);
  File*slot=nullptr;for(auto&f:files)if(!f.handle){slot=&f;break;}if(!slot||nextFile==0x7fffffff)return finish(-4);
  int result=open(path,*slot);if(result)return finish(result);
  slot->owner=handle;slot->handle=nextFile++;out.write(&slot->handle,4);trace.opens++;return finish(0);
 }
 if(op==22||op==67||op==70){
  if(!inputPath(0))return finish(-28);File file;int result=open(path,file);
  if(op==67){auto address=result?0:file.address;close(file);return finish(output(1,&address,4)?0:-6);}
  if(result)return finish(result);
  if(op==70){
   auto length=a[3],position=a[1];GuestDescriptor out(memory);
   if(length>1024*1024||!out.view(a[2],1,true)){close(file);return finish(-6);}
   auto count=position<file.size?std::min(length,file.size-position):0;
   if(!out.canWrite(count)){close(file);return finish(-22);}
   std::vector<std::uint8_t> bytes(count);bool ok=true;
   if(count){if(file.stream)ok=!std::fseek(file.stream,position,SEEK_SET)&&std::fread(bytes.data(),1,count,file.stream)==count;else ok=memory.read(file.address+position,bytes.data(),count);}
   close(file);if(!ok)return finish(-20);out.write(bytes.data(),count);trace.reads++;trace.bytes+=count;return finish(0);
  }
  // EKA1 TEntry: attributes, size, time, UID triple, TBufC<256> name.
  std::array<std::uint8_t,544> bytes{};auto word=[&](unsigned offset,std::uint32_t v){std::memcpy(bytes.data()+offset,&v,4);};
  word(0,1);word(4,file.size);std::uint32_t uids[3]{};if(file.size>=12){if(file.stream)std::fread(uids,1,12,file.stream);else memory.read(file.address,uids,12);}for(unsigned n=0;n<3;n++)word(16+n*4,uids[n]);
  auto leaf=path.substr(path.find_last_of('\\')+1);word(28,leaf.size());std::memcpy(bytes.data()+32,leaf.data(),leaf.size()*2);close(file);return finish(output(1,bytes.data(),bytes.size())?0:-6);
 }
 File*f=nullptr;for(auto&candidate:files)if(candidate.handle&&candidate.owner==handle&&candidate.handle==a[3])f=&candidate;
 if(op==26||op==34||op==38||op==39||op==40||op==42||op==44){
  if(!f)return finish(-8);
  if(op==26){close(*f);return finish(0);}
  if(op==39)return finish(0); // read-only streams have nothing to flush.
  if(op==40)return finish(output(0,&f->size,4)?0:-6);
  if(op==42){std::uint32_t att=1;return finish(output(0,&att,4)?0:-6);}
  if(op==44){std::uint64_t time=0;return finish(output(0,&time,8)?0:-6);}
  if(op==38){
   auto mode=a[1];if(mode>3)return finish(-6);auto base=mode==2?f->position:mode==3?f->size:0;
   auto position=std::int64_t(base)+std::int32_t(a[0]);if(position<0||position>0x7fffffff)return finish(-6);
   auto value=std::uint32_t(position);if(!output(2,&value,4))return finish(-6);f->position=value;return finish(0);
  }
  auto length=a[1],position=a[2];if(position==0x80000000||position==0xffffffff)position=f->position;
  if(length>1024*1024||position>0x7fffffff)return finish(-6);
  GuestDescriptor out(memory);if(!out.view(a[0],1,true))return finish(-6);auto count=position<f->size?std::min(length,f->size-position):0;
  if(!out.canWrite(count))return finish(-22);std::vector<std::uint8_t> bytes(count);bool ok=true;
  if(count){if(f->stream)ok=!std::fseek(f->stream,position,SEEK_SET)&&std::fread(bytes.data(),1,count,f->stream)==count;else ok=memory.read(f->address+position,bytes.data(),count);}
  if(!ok)return finish(-20);out.write(bytes.data(),count);f->position=position+count;trace.reads++;trace.bytes+=count;return finish(0);
 }
 return unknown();
}
}
