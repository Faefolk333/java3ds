#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace symbian {
struct TouchProfile {int width=360,height=640,stickX=220,stickY=780,radius=130,actionX=880,actionY=780,secondaryX=740,secondaryY=880;};
struct PadSample {unsigned buttons=0;int x=0,y=0;bool touch=false;int touchX=0,touchY=0;};
enum class TouchPhase {Down,Move,Up};
struct TouchEvent {TouchPhase phase;int x,y;};
struct TouchEvents {TouchEvent events[2];unsigned count=0;};
// One emulated pointer. Buttons temporarily take priority over a virtual stick;
// this deliberately does not claim multi-touch support on a single-touch OS.
class TouchMapper {
 TouchProfile profile;int kind=0,lastX=0,lastY=0;
 static int clamp(int x){return std::max(0,std::min(1000,x));}
 int px(int x)const{return clamp(x)*(profile.width-1)/1000;}
 int py(int y)const{return clamp(y)*(profile.height-1)/1000;}
public:
 explicit TouchMapper(TouchProfile p={}):profile(p){}
 TouchEvents cancel(){TouchEvents r;if(kind)r.events[r.count++]={TouchPhase::Up,lastX,lastY};kind=0;return r;}
 const TouchProfile&settings()const{return profile;}
 TouchEvents update(const PadSample&s){
  TouchEvents r;if(profile.width<1||profile.height<1)return cancel();int target=0,x=0,y=0;
  if(s.touch){target=4;x=px(s.touchX);y=py(s.touchY);}
  else if(s.buttons&16){target=2;x=px(profile.actionX);y=py(profile.actionY);}
  else if(s.buttons&32){target=3;x=px(profile.secondaryX);y=py(profile.secondaryY);}
  else{
   float dx=s.x/156.f,dy=-s.y/156.f;
   if(s.buttons&4)dx=-1;if(s.buttons&8)dx=1;if(s.buttons&1)dy=-1;if(s.buttons&2)dy=1;
   float length=std::sqrt(dx*dx+dy*dy);
   if(length>.13f){target=1;if(length>1){dx/=length;dy/=length;}int radius=std::min(profile.width,profile.height)*profile.radius/1000;x=std::max(0,std::min(profile.width-1,px(profile.stickX)+(int)(dx*radius)));y=std::max(0,std::min(profile.height-1,py(profile.stickY)+(int)(dy*radius)));}
  }
  if(kind&&kind!=target)r.events[r.count++]={TouchPhase::Up,lastX,lastY};
  if(target){if(kind!=target)r.events[r.count++]={TouchPhase::Down,x,y};else if(x!=lastX||y!=lastY)r.events[r.count++]={TouchPhase::Move,x,y};lastX=x;lastY=y;}
  kind=target;return r;
 }
};
}
