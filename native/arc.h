#pragma once
#include <math.h>
static s32 arc(Runtime*r,JClass*c){
 (void)c;int dw=arg(r,1),dh=arg(r,2);uint32_t*d=target(r,dw,dh);int x=arg(r,3),y=arg(r,4),w=arg(r,5),h=arg(r,6),fill=arg(r,15);int64_t extent=arg(r,8);if(!d||w<=0||h<=0||!extent)return 0;
 int start=arg(r,7)%360,full=extent>=360||extent<=-360;if(extent<0){start=(start+(int)(extent%360))%360;extent=-extent;}if(full)extent=360;
 float a=start*(3.14159265358979323846/180),b=(start+extent)*(3.14159265358979323846/180);float sx=cosf(a),sy=sinf(a),ex=cosf(b),ey=sinf(b),rx=w*.5f,ry=h*.5f,mx=x+rx,my=y+ry,ix=1/rx,iy=1/ry;
 int cx=arg(r,10),cy=arg(r,11),cw=arg(r,12),ch=arg(r,13);int64_t l=x,t=y,right=(int64_t)x+w+(fill?0:1),bottom=(int64_t)y+h+(fill?0:1);if(l<0)l=0;if(t<0)t=0;if(l<cx)l=cx;if(t<cy)t=cy;if(right>dw)right=dw;if(bottom>dh)bottom=dh;if(right>(int64_t)cx+cw)right=(int64_t)cx+cw;if(bottom>(int64_t)cy+ch)bottom=(int64_t)cy+ch;
 Instance*shadow=arr(r,14);float*z=crimson_stereo_target(d);if(crimson_stereo_mode()&&shadow&&shadow->arr_length>=dw*dh)z=(float*)shadow->arr_body;float layer=crimson_stereo_tag_2d(dw,dh,x,y,w,h);uint32_t color=(uint32_t)arg(r,9)|0xff000000;
 for(int yy=(int)t;yy<bottom;yy++)for(int xx=(int)l;xx<right;xx++){
  float dx=(xx+(fill?.5f:0)-mx)*ix,dy=(my-yy-(fill?.5f:0))*iy;if(dx*dx+dy*dy>1.000001f)continue;
  if(!fill){float nx=fabsf(dx)+ix,ny=fabsf(dy)+iy;if(nx*nx+dy*dy<=1&&dx*dx+ny*ny<=1)continue;}
  if(!full){float first=sx*dy-sy*dx,last=dx*ey-dy*ex;int inside=extent<=180?(first>=-1e-6f&&last>=-1e-6f):(first>=-1e-6f||last>=-1e-6f);if(!inside)continue;}
  int index=yy*dw+xx;d[index]=color;if(z)z[index]=layer;
 }return 0;
}
