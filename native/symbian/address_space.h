#pragma once
#include <array>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
#include <new>

namespace symbian {
enum Access : unsigned { Read=1, Write=2, Execute=4 };

// Sparse guest addresses never become host pointers. This initial single-process
// implementation deliberately uses checked callbacks, not direct CPU TLB pages.
class AddressSpace {
 struct Region {
  std::uint32_t base=0,size=0,committed=0;
  unsigned access=0;
  std::unique_ptr<std::uint8_t[]> bytes;
 };
 std::array<Region,128> regions;
 std::size_t usedBytes=0,limitBytes;
 Region* find(std::uint32_t address,std::size_t size,unsigned access) {
  if(!size||size>0x100000000ULL-address)return nullptr;
  for(auto&r:regions)if(r.bytes&&address>=r.base&&
    std::uint64_t(address)-r.base+size<=(access?r.committed:r.size)&&(r.access&access)==access)return &r;
  return nullptr;
 }
public:
 explicit AddressSpace(std::size_t limit=24*1024*1024):limitBytes(limit){}
 std::size_t used()const{return usedBytes;}
 bool map(std::uint32_t base,std::uint32_t size,unsigned access) {
  if(!base||!size||std::uint64_t(base)+size>0x100000000ULL||
    size>limitBytes-usedBytes||!(access&Read)||(access&~7u))return false;
  Region*slot=nullptr;
  for(auto&r:regions){
   if(!r.bytes){if(!slot)slot=&r;continue;}
   if(std::uint64_t(base)<std::uint64_t(r.base)+r.size&&std::uint64_t(r.base)<std::uint64_t(base)+size)return false;
  }
  if(!slot)return false;
  std::unique_ptr<std::uint8_t[]> bytes(new(std::nothrow) std::uint8_t[size]());
  if(!bytes)return false;
  slot->base=base;slot->size=size;slot->committed=size;slot->access=access;slot->bytes=std::move(bytes);usedBytes+=size;return true;
 }
 bool commit(std::uint32_t base,std::uint32_t size){
  for(auto&r:regions)if(r.bytes&&r.base==base){
   if(size>r.size)return false;
   auto begin=std::min(size,r.committed),end=std::max(size,r.committed);
   if(end>begin)std::memset(r.bytes.get()+begin,0,end-begin);
   r.committed=size;return true;
  }return false;
 }
 bool unmap(std::uint32_t base){for(auto&r:regions)if(r.bytes&&r.base==base){usedBytes-=r.size;r=Region{};return true;}return false;}
 bool permits(std::uint32_t address,std::size_t size,unsigned access){return find(address,size,access)!=nullptr;}
 bool read(std::uint32_t address,void*out,std::size_t size,unsigned access=Read){
  auto*r=find(address,size,access);if(!r)return false;
  std::memcpy(out,r->bytes.get()+(address-r->base),size);return true;
 }
 bool write(std::uint32_t address,const void*source,std::size_t size){
  auto*r=find(address,size,Write);if(!r)return false;
  std::memcpy(r->bytes.get()+(address-r->base),source,size);return true;
 }
 // Only the loader uses this operation to initialize an RX region before run.
 bool initialize(std::uint32_t address,const void*source,std::size_t size){
  auto*r=find(address,size,0);if(!r)return false;
  std::memcpy(r->bytes.get()+(address-r->base),source,size);return true;
 }
 template<class T>bool read(std::uint32_t a,T*out){return read(a,out,sizeof(T));}
 template<class T>bool write(std::uint32_t a,const T*in){return write(a,in,sizeof(T));}
};
}
