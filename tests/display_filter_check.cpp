#include "../native/display_filter.h"
#include <cassert>
#include <cstdio>
int main(){for(int x=0;x<12;x++)for(int y=0;y<12;y++){uint32_t c=0xab123456;assert(display_filter(c,x,y,0)==c);for(int m=1;m<3;m++){uint32_t d=display_filter(c,x,y,m);assert(d>>24==0xab);for(int s=0;s<24;s+=8)assert(((d>>s)&255)<=((c>>s)&255));}}assert(display_filter(0xffffffff,0,1,1)==0xffebebeb);assert(display_filter(0xffffffff,2,2,2)==0xffe6e6e6);puts("DISPLAY_FILTER_PASS off_identity subtle_scanlines_grid alpha_preserved");}
