#pragma once
#include <stdint.h>
enum { DISPLAY_FILTER_COUNT=7 };
// Screen-space masks use the same phase for both eyes. The UI stays unfiltered.
static inline uint32_t display_filter(uint32_t color,int x,int y,int mode){
 if(mode>=3&&mode<DISPLAY_FILTER_COUNT){
  uint32_t result=color&0xff000000;int phase=mode==5?(x+(y&1))%3:mode==6?(x/2)%3:x%3;
  for(int c=0;c<3;c++){
   int shift=16-c*8,v=(color>>shift)&255;
   // Lift dim phosphors before applying the mask, preserving true black.
   int lift=mode==3?60:mode==6?128:100;v+=((v*(255-v)*lift+32768)>>16);
   int mask=255,beam=255;
   if(mode==3)beam=(y&1)?144:255;
   else {mask=phase==c?255:mode==5?105:mode==6?184:124;beam=mode==5?(y%3==2?155:255):(y&1)?(mode==6?162:205):255;}
   if(mode==6){v=v*(c==0?270:c==2?242:255)/255;if(v>255)v=255;}
   v=(v*mask*beam+32512)/65025;result|=(uint32_t)v<<shift;
  }return result;
 }
 int factor=255;if(mode==1&&(y&1))factor=235;
 if(mode==2){int vertical=x%3==2,horizontal=y%3==2;factor=vertical&&horizontal?230:vertical||horizontal?242:255;}
 if(factor==255)return color;
 uint32_t result=color&0xff000000;for(int shift=0;shift<24;shift+=8)result|=((((color>>shift)&255)*factor+127)/255)<<shift;return result;
}
// Prepare once on mode changes; presentation then needs only three byte lookups.
// Six pixels cover both the two-row beam and three-column phosphor periods.
typedef struct {uint8_t lut[6][6][3][256];int mode,ready;} DisplayFilter;
static inline void display_filter_configure(DisplayFilter*s,int mode){
 if(mode<0||mode>=DISPLAY_FILTER_COUNT)mode=0;if(s->ready&&s->mode==mode)return;
 s->mode=mode;s->ready=1;if(!mode)return;
 for(int y=0;y<6;y++)for(int x=0;x<6;x++)for(int c=0;c<3;c++)for(int v=0;v<256;v++)s->lut[y][x][c][v]=(display_filter(0xff000000|((uint32_t)v<<(16-8*c)),x,y,mode)>>(16-8*c))&255;
}
static inline uint32_t display_filter_apply(const DisplayFilter*s,uint32_t color,int x,int y){
 if(!s->mode)return color;const uint8_t (*p)[256]=s->lut[y%6][x%6];return (color&0xff000000)|((uint32_t)p[0][(color>>16)&255]<<16)|((uint32_t)p[1][(color>>8)&255]<<8)|p[2][color&255];
}
