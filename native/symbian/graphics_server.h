#pragma once
#include "eka1_kernel.h"
#include "guest_descriptor.h"
namespace symbian {
// Initial real FBS session state. Font/bitmap requests remain explicit stops;
// accepting a connection does not claim that graphics have been implemented.
class GraphicsServer {
 AddressSpace&memory;
 struct Session {std::uint32_t handle=0;bool initialized=false;};
 std::array<Session,8> sessions{};
 std::array<Session,8> windows{};
public:
 explicit GraphicsServer(AddressSpace&m):memory(m){}
 std::uint32_t calls=0,initializations=0;std::int32_t lastOperation=-1;
 std::uint32_t windowCalls=0,windowInitializations=0,windowCommand=0,windowObject=0;std::int32_t windowOperation=-1;
 std::int32_t twipsX=0,twipsY=0;
 KernelResult request(const std::u16string&name,std::uint32_t handle,std::int32_t operation,const std::array<std::uint32_t,4>&a){
  auto done=[](int value){return KernelResult{KernelResult::Kind::Returned,std::uint32_t(value),0,0};};
  if(name==u"Windowserver"){
   if(operation==-1){for(auto&s:windows)if(!s.handle){s={handle,false};return done(0);}return done(-4);}
   Session*s=nullptr;for(auto&candidate:windows)if(handle&&candidate.handle==handle)s=&candidate;if(!s)return done(-8);
   if(operation==-2){*s={};return done(0);}windowCalls++;windowOperation=operation;
   if(operation==2){if(!s->initialized){s->initialized=true;windowInitializations++;}return done(handle);}
   if(!s->initialized)return done(-18);
   if(operation==0||operation==4){
    GuestDescriptor bytes(memory);if(!bytes.view(a[0],1,false)||bytes.length>65536)return done(-6);
    unsigned at=0,count=0;std::uint32_t object=0;bool unsupported=false,disconnect=false;
    // Validate the entire command batch before changing any session state.
    while(at<bytes.length){
     if(bytes.length-at<4||++count>1024)return done(-6);std::uint16_t header[2];memory.read(bytes.data+at,header,4);at+=4;
     auto op=header[0]&0x7fff;
     if(header[0]&0x8000){if(bytes.length-at<4)return done(-6);memory.read(bytes.data+at,&object);at+=4;}
     if(header[1]>bytes.length-at)return done(-6);
     if(!unsupported){windowCommand=op;windowObject=object;}
     if(op!=0||object!=handle||header[1])unsupported=true;else disconnect=true;
     at+=header[1];
    }
    if(unsupported)return {};if(disconnect)s->initialized=false;return done(0);
   }
   return {};
  }
  if(name!=u"Fontbitmapserver")return {};
  if(operation==-1){for(auto&s:sessions)if(!s.handle){s={handle,false};return done(0);}return done(-4);}
  Session*s=nullptr;for(auto&candidate:sessions)if(handle&&candidate.handle==handle)s=&candidate;if(!s)return done(-8);
  if(operation==-2){*s={};return done(0);}calls++;lastOperation=operation;
  if(operation==0){if(!s->initialized){s->initialized=true;initializations++;}return done(0);}
  if(!s->initialized)return done(-18);
  if(operation==11){if(!a[0]||!a[1]||a[0]>65536||a[1]>65536)return done(-6);twipsX=a[0];twipsY=a[1];return done(0);}
  return {};
 }
};
}
