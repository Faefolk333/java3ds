#pragma once
#include "eka1_kernel.h"
#include "window_server.h"
namespace symbian {
// Initial real FBS session state. Font/bitmap requests remain explicit stops;
// accepting a connection does not claim that graphics have been implemented.
class GraphicsServer {
 AddressSpace&memory;
 struct Session {std::uint32_t handle=0;bool initialized=false;};
 std::array<Session,8> sessions{};
public:
 WindowServer window;
 explicit GraphicsServer(AddressSpace&m):memory(m),window(m){}
 std::uint32_t calls=0,initializations=0;std::int32_t lastOperation=-1;
 std::int32_t twipsX=0,twipsY=0;
 KernelResult request(const std::u16string&name,std::uint32_t handle,std::int32_t operation,const std::array<std::uint32_t,4>&a){
  auto done=[](int value){return KernelResult{KernelResult::Kind::Returned,std::uint32_t(value),0,0};};
  if(name==u"Windowserver")return window.request(handle,operation,a);
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
