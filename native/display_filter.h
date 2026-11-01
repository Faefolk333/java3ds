#pragma once
#include <stdint.h>
// Screen-space masks use the same phase for both eyes. The UI stays unfiltered.
static inline uint32_t display_filter(uint32_t color,int x,int y,int mode){
 int factor=255;if(mode==1&&(y&1))factor=235;
 if(mode==2){int vertical=x%3==2,horizontal=y%3==2;factor=vertical&&horizontal?230:vertical||horizontal?242:255;}
 if(factor==255)return color;
 uint32_t result=color&0xff000000;for(int shift=0;shift<24;shift+=8)result|=((((color>>shift)&255)*factor+127)/255)<<shift;return result;
}
