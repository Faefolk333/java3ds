#include "../native/symbian/touch_mapper.h"
#include <cassert>
#include <cstdio>
using namespace symbian;
int main(){
 TouchMapper m;PadSample s;assert(!m.update(s).count);s.x=15;s.y=0;assert(!m.update(s).count);
 s.x=156;auto e=m.update(s);assert(e.count==1&&e.events[0].phase==TouchPhase::Down);int stickX=e.events[0].x;assert(stickX>220*359/1000);assert(!m.update(s).count);
 s.y=156;e=m.update(s);assert(e.count==1&&e.events[0].phase==TouchPhase::Move&&e.events[0].y<780*639/1000);
 s.buttons=16;e=m.update(s);assert(e.count==2&&e.events[0].phase==TouchPhase::Up&&e.events[1].phase==TouchPhase::Down&&e.events[1].x==880*359/1000);assert(!m.update(s).count);
 s.buttons=0;e=m.update(s);assert(e.count==2&&e.events[1].x<stickX);e=m.cancel();assert(e.count==1&&e.events[0].phase==TouchPhase::Up);assert(!m.cancel().count);
 s={};s.buttons=1|4;e=m.update(s);assert(e.count==1);assert(e.events[0].x<220*359/1000&&e.events[0].y<780*639/1000);
 s.touch=true;s.touchX=1200;s.touchY=-50;e=m.update(s);assert(e.count==2&&e.events[1].x==359&&e.events[1].y==0);m.cancel();
 TouchProfile wide;wide.width=640;wide.height=360;TouchMapper landscape(wide);s={};s.buttons=32;e=landscape.update(s);assert(e.count==1&&e.events[0].x==740*639/1000&&e.events[0].y==880*359/1000);s={};e=landscape.update(s);assert(e.count==1&&e.events[0].phase==TouchPhase::Up);
 puts("SYMBIAN_TOUCH_PASS deadzone diagonal movement action_priority release_resume cancel orientation coordinate_bounds single_pointer");
}
