#include "../native/bottom_screen.h"
#include <cassert>
#include <cstdio>
int main(){BottomScreenState s;s.start(0,15,false);assert(!s.update(14999,false,false)&&!s.asleep);s.update(15000,false,false);assert(s.asleep);assert(!s.update(16000,true,false)&&!s.asleep);assert(!s.update(16001,true,false));s.update(16002,false,false);assert(s.update(16003,true,false));s.update(60000,false,true);assert(!s.asleep);s.start(0,0,false);s.update(999999,false,false);assert(!s.asleep);s.start(0,30,true);assert(!s.update(1,true,false)&&!s.asleep);puts("BOTTOM_SCREEN_PASS timeout wake_consumes_touch held_touch menu never start_asleep");}
