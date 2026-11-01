#include "../native/display_filter.h"
#include <cassert>
#include <cstdio>
int main(){for(int x=0;x<12;x++)for(int y=0;y<12;y++){uint32_t c=0xab123456;assert(display_filter(c,x,y,0)==c);for(int m=1;m<3;m++){uint32_t d=display_filter(c,x,y,m);assert(d>>24==0xab);for(int s=0;s<24;s+=8)assert(((d>>s)&255)<=((c>>s)&255));}}assert(display_filter(0xffffffff,0,1,1)==0xffebebeb);assert(display_filter(0xffffffff,2,2,2)==0xffe6e6e6);
 DisplayFilter cache={};for(int m=0;m<DISPLAY_FILTER_COUNT;m++){display_filter_configure(&cache,m);for(int y=0;y<12;y++)for(int x=0;x<12;x++)for(int v=0;v<256;v++){uint32_t c=0xa5000000|v<<16|((v*7)&255)<<8|((v*19)&255);assert(display_filter_apply(&cache,c,x,y)==display_filter(c,x,y,m));assert(display_filter_apply(&cache,c,x,y)>>24==0xa5);assert(display_filter_apply(&cache,0xff000000,x,y)==0xff000000);}}
 uint32_t bright=display_filter(0xff202020,0,0,3)&255,dim=display_filter(0xff202020,0,1,3)&255;assert(bright>dim+8);uint32_t mask=display_filter(0xff202020,0,0,4);assert(((mask>>16)&255)>(mask&255)+10);
 puts("DISPLAY_FILTER_PASS 7_modes lookup_exact dark_beam_color_contrast black_alpha_preserved");}
