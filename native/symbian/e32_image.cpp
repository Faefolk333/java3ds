// E32 field layout/relocation semantics checked against EKA2L1 78dbf9fcc,
// loader/e32img.{h,cpp}, kernel/codeseg.cpp and common/crypt.cpp. This bounded
// parser is project code; it does not copy the upstream decompressor/OS loader.
#include "e32_image.h"
#include <algorithm>
#include <cstdio>
#include <limits>

namespace symbian {
namespace {
std::uint16_t u16(const std::vector<std::uint8_t>&b,std::size_t p){return b[p]|std::uint16_t(b[p+1])<<8;}
std::uint32_t u32(const std::vector<std::uint8_t>&b,std::size_t p){return u16(b,p)|std::uint32_t(u16(b,p+2))<<16;}
bool range(std::size_t offset,std::uint64_t length,std::size_t size){return offset<=size&&length<=size-offset;}
std::uint16_t uidHalf(const std::vector<std::uint8_t>&b,unsigned start){
 std::uint16_t crc=0;for(unsigned n=start;n<12;n+=2){crc^=std::uint16_t(b[n])<<8;for(unsigned bit=0;bit<8;bit++)crc=(crc&0x8000)?(crc<<1)^0x1021:crc<<1;}return crc;
}
bool fail(std::string&error,const char*message){error=message;return false;}
}
bool E32Image::open(const char*path,std::string&error,bool(*keepRunning)()){
 ready=false;bytes.clear();imports.clear();relocations.clear();info={};
 FILE*f=std::fopen(path,"rb");if(!f)return fail(error,"Cannot open E32 file");
 if(std::fseek(f,0,SEEK_END)!=0){std::fclose(f);return fail(error,"Cannot seek E32 file");}
 long size=std::ftell(f);
 if(size<124||std::uint64_t(size)>maxFileBytes||std::fseek(f,0,SEEK_SET)!=0){std::fclose(f);return fail(error,"E32 file size is outside the loader limit");}
 std::vector<std::uint8_t> input(size);bool okay=true;std::size_t copied=0;
 while(copied<input.size()){
  if(keepRunning&&!keepRunning()){std::fclose(f);return fail(error,"E32 read cancelled");}
  auto chunk=std::min<std::size_t>(65536,input.size()-copied);
  if(std::fread(input.data()+copied,1,chunk,f)!=chunk){okay=false;break;}copied+=chunk;
 }
 std::fclose(f);
 if(!okay)return fail(error,"Short read of E32 file");return parse(std::move(input),error);
}
bool E32Image::parse(std::vector<std::uint8_t> input,std::string&error){
 ready=false;bytes.clear();imports.clear();relocations.clear();info={};error.clear();
 if(input.size()<124||input.size()>maxFileBytes)return fail(error,"E32 header/file size is invalid");
 bytes=std::move(input);const auto&b=bytes;
 if(u32(b,16)!=0x434f5045)return fail(error,"Not an E32 executable (SIS packages must be installed first)");
 if(u32(b,20)!=0x2000)return fail(error,"Only ARM EKA1 E32 images are supported by this loader stage");
 if(u32(b,12)!=(std::uint32_t(uidHalf(b,1))<<16|uidHalf(b,0)))return fail(error,"E32 checked UID checksum mismatch");
 info.uid1=u32(b,0);info.uid2=u32(b,4);info.uid3=u32(b,8);info.flags=u32(b,44);
 if(info.uid1!=0x10000079&&info.uid1!=0x1000007a)return fail(error,"Unsupported E32 executable UID");
 if((info.flags&0x0f000000)&&u32(b,28))return fail(error,"Compressed E32 image is not supported yet");
 if(info.flags&0xf0000000)return fail(error,"Only EKA1 PE import tables are supported yet");
 info.codeSize=u32(b,48);info.dataSize=u32(b,52);info.heapMin=u32(b,56);info.heapMax=u32(b,60);
 info.stack=u32(b,64);info.bssSize=u32(b,68);info.entry=u32(b,72);info.codeBase=u32(b,76);info.dataBase=u32(b,80);
 info.exportsOffset=u32(b,88);info.exportsCount=u32(b,92);info.textSize=u32(b,96);info.codeOffset=u32(b,100);info.dataOffset=u32(b,104);
 if(!info.codeSize||info.codeSize>16*1024*1024||info.textSize>info.codeSize||!info.textSize||
  info.codeOffset<124||!range(info.codeOffset,info.codeSize,b.size()))return fail(error,"E32 code/text range is invalid");
 if((info.entry&~1u)>=info.textSize||(!(info.entry&1)&&(info.entry&3)))return fail(error,"E32 entry point is outside aligned text");
 if(info.dataSize&&!range(info.dataOffset,info.dataSize,b.size()))return fail(error,"E32 data file range is invalid");
 if(std::uint64_t(info.dataSize)+info.bssSize>24*1024*1024||info.stack>1024*1024||info.heapMin>info.heapMax)
  return fail(error,"E32 data/stack/heap declarations exceed supported limits");
 if(std::uint64_t(info.codeBase)+info.codeSize>0x100000000ULL||std::uint64_t(info.dataBase)+info.dataSize+info.bssSize>0x100000000ULL)
  return fail(error,"E32 linked address range wraps");
 if(info.exportsCount>8192|| (info.exportsCount&&(!range(info.exportsOffset,std::uint64_t(info.exportsCount)*4,b.size())||
  info.exportsOffset<info.codeOffset||std::uint64_t(info.exportsOffset)+info.exportsCount*4ULL>std::uint64_t(info.codeOffset)+info.codeSize)))
  return fail(error,"E32 export directory is outside code");
 // EKA1 exports are code-relative offsets, unlike EKA2's linked addresses.
 // Reference: kernel/libmanager.cpp::import_e32img in the same upstream revision.
 for(std::uint32_t i=0;i<info.exportsCount;i++)if(u32(b,info.exportsOffset+i*4)>=info.codeSize)
  return fail(error,"EKA1 exported offset is outside code");
 const std::uint32_t libraryCount=u32(b,84),section=u32(b,108);
 if(libraryCount>256)return fail(error,"Too many E32 import libraries");
 if(libraryCount){
  if(!range(section,4,b.size()))return fail(error,"E32 import section is missing");
  std::uint32_t size=u32(b,section);if(size<4||!range(section,size,b.size()))return fail(error,"E32 import section length is invalid");
  std::size_t cursor=section+4,end=section+size;
  for(std::uint32_t i=0;i<libraryCount;i++){
   if(!range(cursor,8,end))return fail(error,"Truncated E32 import block");
   auto nameOffset=u32(b,cursor),count=u32(b,cursor+4);cursor+=8;
   if(count>8192-imports.size()||!range(cursor,std::uint64_t(count)*4,end)||!range(section,nameOffset,end)||nameOffset>=size)
    return fail(error,"E32 import count/name range is invalid");
   std::size_t nameAt=std::size_t(section)+nameOffset,nameEnd=nameAt;
   while(nameEnd<end&&nameEnd-nameAt<=255&&b[nameEnd]){if(b[nameEnd]<32||b[nameEnd]>126)return fail(error,"Non-ASCII E32 library name");nameEnd++;}
   if(nameEnd==end||nameEnd==nameAt||nameEnd-nameAt>255)return fail(error,"Unterminated/invalid E32 library name");
   std::string library(reinterpret_cast<const char*>(b.data()+nameAt),nameEnd-nameAt);
   for(std::uint32_t j=0;j<count;j++){
    std::uint32_t ordinal=u32(b,cursor);cursor+=4;
    std::uint64_t iat=std::uint64_t(info.textSize)+imports.size()*4;
    if(!ordinal||ordinal>65535||iat+4>info.codeSize||u32(b,info.codeOffset+iat)!=ordinal)return fail(error,"E32 IAT/ordinal mismatch");
    imports.push_back({library,ordinal,static_cast<std::uint32_t>(iat)});
   }
  }
 }
 auto readRelocations=[&](std::uint32_t offset,std::uint32_t segmentSize,bool data)->bool{
  if(!offset)return true;
  if(!range(offset,8,b.size()))return fail(error,"Truncated E32 relocation section");
  auto size=u32(b,offset),count=u32(b,offset+4);std::size_t cursor=std::size_t(offset)+8;
  if(!range(cursor,size,b.size())||count>segmentSize/4||count>131072-relocations.size())return fail(error,"E32 relocation size/count is invalid");
  std::size_t end=cursor+size;std::uint32_t observed=0;std::vector<std::uint32_t> seen;
  while(cursor<end){
   if(!range(cursor,8,end))return fail(error,"Truncated E32 relocation page");
   auto page=u32(b,cursor),blockSize=u32(b,cursor+4);
   if((page&4095)||blockSize<8||(blockSize&3)||!range(cursor,blockSize,end))return fail(error,"Invalid E32 relocation page size/address");
   for(std::size_t p=cursor+8;p<cursor+blockSize;p+=2){
    auto entry=u16(b,p);if(!entry)continue;unsigned type=entry>>12;
    std::uint64_t at=std::uint64_t(page)+(entry&4095);
    if(type<1||type>3||(at&3)||at+4>segmentSize||observed>=count)return fail(error,"Invalid/excess E32 relocation target");
    seen.push_back(static_cast<std::uint32_t>(at));
    relocations.push_back({static_cast<std::uint32_t>(at),type,data});observed++;
   }
   cursor+=blockSize;
  }
  std::sort(seen.begin(),seen.end());
  if(std::adjacent_find(seen.begin(),seen.end())!=seen.end())return fail(error,"Duplicate E32 relocation target");
  return observed==count?true:fail(error,"E32 relocation count mismatch");
 };
 if(!readRelocations(u32(b,112),info.codeSize,false)||!readRelocations(u32(b,116),info.dataSize,true))return false;
 ready=true;return true;
}
bool E32Image::load(AddressSpace&memory,std::uint32_t codeBase,std::uint32_t dataBase,LoadedE32&loaded,std::string&error)const{
 error.clear();if(!ready)return fail(error,"E32 image has not passed validation");
 if((codeBase&4095)||(dataBase&4095))return fail(error,"Runtime E32 bases must be page aligned");
 std::uint32_t dataBytes=info.dataSize+info.bssSize;
 if(!memory.map(codeBase,info.codeSize,Read|Execute))return fail(error,"Cannot map E32 code within guest memory budget");
 if(dataBytes&&!memory.map(dataBase,dataBytes,Read|Write)){memory.unmap(codeBase);return fail(error,"Cannot map E32 data within guest memory budget");}
 memory.initialize(codeBase,bytes.data()+info.codeOffset,info.codeSize);
 if(info.dataSize)memory.initialize(dataBase,bytes.data()+info.dataOffset,info.dataSize);
 for(const auto&r:relocations){
  std::uint32_t at=(r.data?dataBase:codeBase)+r.offset,value=0;memory.read(at,&value);
  std::uint32_t delta=r.type==1?codeBase-info.codeBase:r.type==2?dataBase-info.dataBase:
   (std::uint32_t(value-info.codeBase)<info.codeSize?codeBase-info.codeBase:dataBase-info.dataBase);
  value+=delta;memory.initialize(at,&value,sizeof(value));
 }
 LoadedE32 result;result.codeBase=codeBase;result.dataBase=dataBase;result.entry=codeBase+info.entry;result.imports=imports;
 for(std::uint32_t i=0;i<info.exportsCount;i++)result.exports.push_back(codeBase+u32(bytes,info.exportsOffset+i*4));
 loaded=std::move(result);return true;
}
}
