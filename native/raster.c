#include "raster.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stereo.h"
int crimson_width=320,crimson_height=240;
uint32_t crimson_pixels[512*512];
static float*stereo_depth;static int stereo_mode,stereo_has_depth;static float stereo_layer;
int crimson_stereo_mode(void){return __atomic_load_n(&stereo_mode,__ATOMIC_ACQUIRE);}
void crimson_stereo_release(void){__atomic_store_n(&stereo_mode,0,__ATOMIC_RELEASE);free(stereo_depth);stereo_depth=NULL;stereo_has_depth=0;stereo_layer=0;}
int crimson_stereo_config(int mode){if(mode<0||mode>3)mode=0;if(mode&&!stereo_depth){float*p=calloc(512*512,sizeof(float));if(!p)return 0;stereo_depth=p;}__atomic_store_n(&stereo_mode,mode,__ATOMIC_RELEASE);return 1;}
float*crimson_stereo_target(const uint32_t*d){return d==crimson_pixels&&crimson_stereo_mode()?stereo_depth:NULL;}
float crimson_stereo_layer(void){return stereo_layer?stereo_layer:stereo_has_depth?-4.f:0;}
void crimson_stereo_set_layer(int index,int count){stereo_layer=index<0||count<1?0:-2.f-(count==1?0:.75f-1.5f*index/(count-1));if(index>=0&&(crimson_stereo_mode()&2))stereo_has_depth=1;}
void crimson_stereo_content_drawn(void){if(crimson_stereo_mode())stereo_has_depth=1;}
void crimson_stereo_models_drawn(void){if(crimson_stereo_mode()&1)stereo_has_depth=1;}
void crimson_stereo_finish_frame(void){stereo_has_depth=0;stereo_layer=0;}
void crimson_stereo_encode(int8_t*out,int count){int mode=crimson_stereo_mode();float lo=INFINITY,hi=0;if((mode&1)&&stereo_depth)for(int i=0;i<count;i++){float z=stereo_depth[i];if(z>0&&isfinite(z)){if(z<lo)lo=z;if(z>hi)hi=z;}}for(int i=0;i<count;i++){float z=mode&&stereo_depth?stereo_depth[i]:0,v=0;if(z==-4){out[i]=-128;continue;}if(z>0&&(mode&1))v=hi>lo?(z-lo)/(hi-lo)-.5f:-.25f;else if(z<=-1&&(mode&2))v=-z-2;int n=(int)(v*127);out[i]=n<-127?-127:n>127?127:n;}}
static int max(int a,int b){return a>b?a:b;}static int min(int a,int b){return a<b?a:b;}
void crimson_fill(uint32_t* dst,int dw,int dh,int x,int y,int w,int h,uint32_t c){
 if(!dst)dst=crimson_pixels;
 int x0=max(0,x),y0=max(0,y),x1=min(dw,x+w),y1=min(dh,y+h);
 float*z=crimson_stereo_target(dst);float layer=crimson_stereo_layer();if(x0==0&&y0==0&&x1==dw&&y1==dh&&layer==-4){layer=0;crimson_stereo_finish_frame();}if(z)for(int yy=y0;yy<y1;yy++)for(int xx=x0;xx<x1;xx++)z[yy*dw+xx]=layer;
 for(int yy=y0;yy<y1;yy++)for(int xx=x0;xx<x1;xx++)dst[yy*dw+xx]=c;
}
void crimson_blit(uint32_t* dst,int dw,int dh,const uint32_t* src,int sw,int sh,int x,int y,int cx,int cy,int cw,int ch){
 if(!dst)dst=crimson_pixels;
 int x0=max(max(0,x),cx),y0=max(max(0,y),cy),x1=min(min(dw,x+sw),cx+cw),y1=min(min(dh,y+sh),cy+ch);
 float*z=crimson_stereo_target(dst);float layer=crimson_stereo_layer();
 for(int yy=y0;yy<y1;yy++)for(int xx=x0;xx<x1;xx++){
  uint32_t s=src[(yy-y)*sw+xx-x],a=s>>24;if(!a)continue;
  if(z)z[yy*dw+xx]=layer;
  if(a==255){dst[yy*dw+xx]=s;continue;}uint32_t d=dst[yy*dw+xx],o=0xff000000;
  for(int n=0;n<24;n+=8)o|=((((s>>n)&255)*a+((d>>n)&255)*(255-a)+127)/255)<<n;
  dst[yy*dw+xx]=o;
 }
}
