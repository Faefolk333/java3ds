#include "raster.h"
#include <stddef.h>
int crimson_width=320,crimson_height=240;
uint32_t crimson_pixels[512*512];
static int max(int a,int b){return a>b?a:b;}static int min(int a,int b){return a<b?a:b;}
void crimson_fill(uint32_t* dst,int dw,int dh,int x,int y,int w,int h,uint32_t c){
 if(!dst)dst=crimson_pixels;
 int x0=max(0,x),y0=max(0,y),x1=min(dw,x+w),y1=min(dh,y+h);
 for(int yy=y0;yy<y1;yy++)for(int xx=x0;xx<x1;xx++)dst[yy*dw+xx]=c;
}
void crimson_blit(uint32_t* dst,int dw,int dh,const uint32_t* src,int sw,int sh,int x,int y,int cx,int cy,int cw,int ch){
 if(!dst)dst=crimson_pixels;
 int x0=max(max(0,x),cx),y0=max(max(0,y),cy),x1=min(min(dw,x+sw),cx+cw),y1=min(min(dh,y+sh),cy+ch);
 for(int yy=y0;yy<y1;yy++)for(int xx=x0;xx<x1;xx++){
  uint32_t s=src[(yy-y)*sw+xx-x],a=s>>24;if(!a)continue;
  if(a==255){dst[yy*dw+xx]=s;continue;}uint32_t d=dst[yy*dw+xx],o=0xff000000;
  for(int n=0;n<24;n+=8)o|=((((s>>n)&255)*a+((d>>n)&255)*(255-a)+127)/255)<<n;
  dst[yy*dw+xx]=o;
 }
}
