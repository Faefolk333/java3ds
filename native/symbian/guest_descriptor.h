#pragma once
#include "address_space.h"
#include <string>
namespace symbian {
// Checked EKA1 descriptor view. Capacity is in elements, never bytes. Mutable
// HBufC-backed TPtr updates both lengths; validation precedes all data writes.
class GuestDescriptor {
 AddressSpace&memory;std::uint32_t at=0,info=0,backing=0,backingInfo=0;bool writableView=false;
public:
 std::uint32_t data=0,length=0,capacity=0;unsigned width=0;
 explicit GuestDescriptor(AddressSpace&m):memory(m){}
 bool view(std::uint32_t address,unsigned elementBytes,bool writable){
  at=address;width=elementBytes;backing=0;writableView=false;
  if((width!=1&&width!=2)||std::uint64_t(at)+12>0x100000000ULL||!memory.read(at,&info))return false;
  if(info&0x0f000000)return false;unsigned type=info>>28;length=info&0xffffff;capacity=length;
  if(type==0)data=at+4;
  else if(type==1){if(!memory.read(at+4,&data))return false;}
  else if(type>=2&&type<=4){
   if(!memory.read(at+4,&capacity)||length>capacity||capacity>0xffffff)return false;
   if(type==3)data=at+8;else if(!memory.read(at+8,&data))return false;
   if(type==4){backing=data;if(std::uint64_t(backing)+4>0xffffffffULL||!memory.read(backing,&backingInfo)||(backingInfo>>28)!=0||(backingInfo&0xffffff)<length)return false;data+=4;}
  }else return false;
  if(writable&&(type<2||!memory.permits(at,4,Write)||(backing&&!memory.permits(backing,4,Write))))return false;
  if(length&&!memory.permits(data,std::uint64_t(length)*width,Read))return false;writableView=writable;return true;
 }
 bool canWrite(std::uint32_t count)const{return writableView&&count<=capacity&&(!count||memory.permits(data,std::uint64_t(count)*width,Write));}
 bool write(const void*bytes,std::uint32_t count){
  if(!canWrite(count))return false;
  if(count&&!memory.write(data,bytes,std::size_t(count)*width))return false;
  auto header=(info&0xff000000)|count;memory.write(at,&header);
  if(backing){header=(backingInfo&0xff000000)|count;memory.write(backing,&header);}length=count;return true;
 }
 bool text(std::uint32_t address,std::u16string&out){
  if(!view(address,2,false)||length>256)return false;
  std::u16string value(length,0);if(length&&!memory.read(data,value.data(),length*2))return false;out=std::move(value);return true;
 }
};
}
