// EKA1 ROM offsets verified against EKA2L1 loader/rom.h and romimage.cpp,
// pinned 78dbf9fcc. Only bounded, uncompressed EKA1 ROMs are supported here.
#include "rom_image.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <functional>
#include <set>
namespace symbian {
namespace {
std::uint32_t word(const unsigned char*b){return b[0]|std::uint32_t(b[1])<<8|std::uint32_t(b[2])<<16|std::uint32_t(b[3])<<24;}
bool error(std::string&e,const char*text){e=text;return false;}
}
bool RomImage::mount(AddressSpace&memory,const char*path,std::string&e,TaskControl*control){
 e.clear();if(mounted())return error(e,"ROM is already mounted");
 FILE*f=std::fopen(path,"rb");if(!f)return error(e,"Cannot open supplied SYM.ROM");
 unsigned char header[512];bool okay=std::fread(header,1,sizeof(header),f)==sizeof(header)&&std::fseek(f,0,SEEK_END)==0;
 long fileSize=okay?std::ftell(f):-1;
 std::uint32_t imageBase=okay?word(header+140):0,imageSize=okay?word(header+144):0;
 if(!okay||fileSize<512||std::uint64_t(fileSize)!=imageSize||imageBase!=0x50000000||imageSize>20*1024*1024||
   (word(header)&0xff000000)!=0xea000000||std::fseek(f,0,SEEK_SET)!=0){
  std::fclose(f);return error(e,"Unsupported/truncated ROM: need uncompressed EKA1 SYM.ROM within 20 MiB");
 }
 if(control){control->report("Allocating ROM memory",0,imageSize);if(!control->checkpoint()){std::fclose(f);return error(e,"ROM read cancelled");}}
 std::unique_ptr<unsigned char[]> buffer(new(std::nothrow) unsigned char[65536]);
 if(!buffer){std::fclose(f);return error(e,"Cannot allocate ROM I/O buffer");}
 if(!memory.map(imageBase,imageSize,Read|Execute)){std::fclose(f);return error(e,"ROM mapping exceeds guest memory budget or overlaps a region");}
 std::uint32_t copied=0;if(control)control->report("Reading ROM",0,imageSize);
 while(copied<imageSize){
  if(control&&!control->checkpoint()){std::fclose(f);memory.unmap(imageBase);return error(e,"ROM read cancelled");}
  std::size_t want=std::min<std::uint32_t>(65536,imageSize-copied);if(std::fread(buffer.get(),1,want,f)!=want||!memory.initialize(imageBase+copied,buffer.get(),want)){okay=false;break;}copied+=want;
  if(control)control->report("Reading ROM",copied,imageSize);
 }
 std::fclose(f);if(!okay){memory.unmap(imageBase);return error(e,"ROM read failed; mapping rolled back");}
 if(control&&!control->checkpoint()){memory.unmap(imageBase);return error(e,"ROM read cancelled");}
 base=imageBase;size=imageSize;return true;
}
bool RomImage::library(AddressSpace&memory,const char*path,RomLibrary&library,std::string&e)const{
 e.clear();if(!mounted())return error(e,"Mount the matching ROM before loading library headers");
 FILE*f=std::fopen(path,"rb");if(!f)return error(e,"ROM library descriptor not found");
 unsigned char h[100],actual[100];bool okay=std::fread(h,1,sizeof(h),f)==sizeof(h);std::fclose(f);
 if(!okay||word(h)!=0x10000079)return error(e,"Not an EKA1 ROM DLL descriptor");
 std::uint32_t codeBase=word(h+20),codeSize=word(h+28),exportsCount=word(h+60),exportsAddress=word(h+64);
 if(codeBase<base+100||codeSize==0||std::uint64_t(codeBase)+codeSize>std::uint64_t(base)+size||exportsCount>65536||
  !memory.read(codeBase-100,actual,sizeof(actual))||std::memcmp(actual,h,100))return error(e,"ROM DLL descriptor does not match the mounted device image");
 if(exportsCount&&(!exportsAddress||exportsAddress<codeBase||std::uint64_t(exportsAddress)+exportsCount*4ULL>std::uint64_t(codeBase)+codeSize))
  return error(e,"ROM DLL export table is outside its code range");
 RomLibrary result;result.uid3=word(h+8);result.codeBase=codeBase;result.codeSize=codeSize;
 result.entry=word(h+16);result.references=word(h+56);result.dataSize=word(h+36);result.bssSize=word(h+40);
 for(std::uint32_t n=0;n<exportsCount;n++){
  std::uint32_t target=0;if(!memory.read(exportsAddress+n*4,&target))return error(e,"Cannot read ROM DLL export");
  result.exports.push_back(target);
 }
 library=std::move(result);return true;
}
bool RomImage::initializationOrder(AddressSpace&memory,const std::vector<RomInitEntry>&roots,std::vector<RomInitEntry>&order,std::string&e,TaskControl*control)const{
 e.clear();std::vector<RomInitEntry> result;std::set<std::uint32_t> visited;std::size_t edges=0;
 auto inside=[&](std::uint32_t address,std::uint64_t bytes){return mounted()&&address>=base&&std::uint64_t(address)+bytes<=std::uint64_t(base)+size;};
 std::function<bool(RomInitEntry)> visit;
 visit=[&](RomInitEntry expected){
  if(control&&!control->checkpoint())return error(e,"ROM initialization planning cancelled");
  // This stage supports EKA1 entry-at-code-start headers only. Reject other
  // layouts; never scan backwards through arbitrary instructions for a match.
  std::uint32_t code=expected.entry&~1u;unsigned char h[100];
  if((code&3)||code<base+100||!inside(code-100,100)||!memory.read(code-100,h,sizeof(h))||word(h)!=0x10000079||word(h+16)!=expected.entry||word(h+20)!=code||!inside(code,word(h+28))||!word(h+28))return error(e,"Unsupported ROM DLL entry/header layout");
  RomInitEntry current{word(h+8),expected.entry,word(h+56)};
  if((expected.uid&&expected.uid!=current.uid)||expected.references!=current.references)return error(e,"ROM DLL dependency identity/table mismatch");
  if(word(h+36)||word(h+40))return error(e,"ROM DLL static writable data requires process mapping");
  if(visited.count(expected.entry))return true;
  if(visited.size()>=256)return error(e,"ROM DLL initialization limit (256) exceeded");
  visited.insert(current.entry);
  if(current.references){
   unsigned char head[4];
   if(!inside(current.references,4)||!memory.read(current.references,head,4))return error(e,"ROM DLL dependency table is outside the ROM");
   auto flags=unsigned(head[0])|(unsigned(head[1])<<8),count=unsigned(head[2])|(unsigned(head[3])<<8);
   if(flags!=1||count>128||count>8192-edges||!inside(current.references,4+count*8ULL))return error(e,"Unsupported ROM DLL dependency table flags/count");
   edges+=count;
   for(unsigned n=0;n<count;n++){
    std::uint32_t pair[2];if(!memory.read(current.references+4+n*8,pair,sizeof(pair)))return error(e,"Unreadable ROM DLL dependency");
    if(!visit({0,pair[0],pair[1]}))return false;
   }
  }
  result.push_back(current);if(control)control->report("Planning ROM initialization",result.size(),256);return true;
 };
 for(const auto&root:roots)if(!visit(root))return false;
 order=std::move(result);return true;
}
bool e32LibraryIdentity(const std::string&name,std::string&filename,std::uint32_t&uid){
 filename.clear();uid=0;auto bracket=name.find('[');
 // Legacy EKA1 DLLs can import by basename without an embedded UID.
 if(bracket==std::string::npos){
  if(name.size()<5||name.size()>255)return false;
  auto base=name.substr(0,name.size()-4),suffix=name.substr(name.size()-4);
  for(char&c:suffix)if(c>='a'&&c<='z')c-=32;
  if(suffix!=".DLL")return false;
  for(char c:base)if(!((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='_'))return false;
  filename=base+suffix;return true;
 }
 if(bracket==std::string::npos||bracket==0||bracket+14!=name.size()||name[bracket+9]!=']')return false;
 for(std::size_t i=0;i<bracket;i++){
  char c=name[i];if(!((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='_'))return false;
  filename+=c;
 }
 for(std::size_t i=bracket+1;i<bracket+9;i++){
  char c=name[i];unsigned value=c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:c>='A'&&c<='F'?c-'A'+10:16;
  if(value>15)return false;uid=(uid<<4)|value;
 }
 std::string suffix=name.substr(bracket+10);for(char&c:suffix)if(c>='a'&&c<='z')c-=32;
 if(suffix!=".DLL")return false;filename+=".DLL";return true;
}
}
