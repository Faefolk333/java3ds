#pragma once
#include <stdint.h>
struct BottomScreenState {
 uint64_t lastTouch=0;int timeout=30;bool asleep=false,consume=false;
 void start(uint64_t now,int seconds,bool hidden){lastTouch=now;timeout=seconds;asleep=hidden;consume=false;}
 bool update(uint64_t now,bool touch,bool menu){
  if(!touch)consume=false;
  if(touch){lastTouch=now;if(asleep){asleep=false;consume=true;}}
  if(menu){lastTouch=now;asleep=false;}else if(timeout>0&&now-lastTouch>=(uint64_t)timeout*1000)asleep=true;
  return touch&&!consume&&!asleep;
 }
};
