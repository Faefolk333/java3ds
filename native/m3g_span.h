/* Java3DS native span port of FreeJ2ME-Plus Graphics3D/Pipelines.
 * GPL-3.0-or-later; see third-party/FreeJ2ME-M3G/LICENSE and NOTICE. */
#pragma once
#include <limits.h>
static float m3g_bits(uint32_t n){float f;memcpy(&f,&n,4);return f;}
static int m3g_int(float f){return isnan(f)?0:f>=2147483648.f?INT_MAX:f<=-2147483648.f?INT_MIN:(int)f;}
static float m3g_inv_step(float pw,float step,int n){float denom=pw*(pw+step*n);uint32_t bits;memcpy(&bits,&denom,4);float inv=m3g_bits(0x7EF127EA-bits);inv=inv*(2.f-denom*inv);return -step*inv;}
static uint32_t m3g_mod(uint32_t bg,uint32_t fg,int mode){uint32_t out=0;for(int i=0;i<4;i++){int b=(bg>>(i*8))&255,f=(fg>>(i*8))&255,v;if(mode==4)v=(b*f)>>7;else if(mode==2)v=i==3?(b*f+b)>>8:(b*f+128)>>8;else v=(b*f)>>8;if(v>255)v=255;out|=(uint32_t)v<<(i*8);}if(mode==1)out=(out&0xffffff)|(bg&0xff000000);return out;}
static uint32_t m3g_blend(uint32_t bg,uint32_t fg,int alpha,int mode){
 if(mode==0)return fg;if(mode==3||mode==4)return m3g_mod(bg,fg,mode);
 if(alpha<=0)return bg;if(mode==1&&alpha>=255)return fg;
 uint32_t out=0;for(int i=0;i<3;i++){int b=(bg>>(i*8))&255,f=(fg>>(i*8))&255,v=mode==1?(f*alpha+b*(255-alpha))>>8:b+((f*alpha)>>8);if(v>255)v=255;out|=(uint32_t)v<<(i*8);}int ba=bg>>24,a=mode==1?alpha+((ba*(255-alpha))>>8):ba+((alpha*(255-ba))>>8);return out|((uint32_t)a<<24);
}
static int m3g_wrap(int n,int length,int repeat){if(!repeat)return n<0?0:n>=length?length-1:n;if(!(length&(length-1)))return n&(length-1);int v=n%length;return v<0?v+length:v;}
static s32 m3gSpan(Runtime*r,JClass*c){
 (void)c;Instance*image=arr(r,0),*depth=arr(r,1),*values=arr(r,5),*state=arr(r,6),*texture=arr(r,7);int offset=arg(r,2),n=arg(r,3),x0=arg(r,4);
 if(!image||!depth||!values||!state||values->arr_length<12||state->arr_length<21||offset<0||n<0||offset>image->arr_length-n||offset>depth->arr_length-n)return 0;
 float*p=(float*)values->arr_body;int32_t*s=(int32_t*)state->arr_body;int flags=s[0],tw=s[6],th=s[7];if((flags&2)&&(!texture||tw<1||th<1||tw>16384||th>16384||(int64_t)tw*th>texture->arr_length))return 0;
 uint32_t*d=(uint32_t*)image->arr_body,*tex=texture?(uint32_t*)texture->arr_body:NULL;int16_t*zbuf=(int16_t*)depth->arr_body;
 float z=p[0],pw=p[2],inv=p[4],u=p[5],v=p[6],invStep=0;int next=(flags&4)?0:INT_MAX,fogFactor=65535,fogStep=0;uint32_t color[4]={(uint32_t)s[10],(uint32_t)s[11],(uint32_t)s[12],(uint32_t)s[13]};
 for(int i=0;i<n;i++,offset++,z+=p[1],pw+=p[3],inv+=invStep,u+=p[7],v+=p[8],fogFactor+=fogStep){
  if(i==next){int max=(s[19]+1)-((x0+i)&s[19]),len=n-i<max?n-i:max;if(len<1)len=1;invStep=m3g_inv_step(pw,p[3],len);if(flags&32){float a,b,end=inv+invStep*len;if(s[20]==81){a=(p[10]-inv*p[9])*256.f;b=(p[10]-end*p[9])*256.f;}else{a=m3g_bits((uint32_t)m3g_int(12102203.f*(-p[11]*inv)+1064866805.f))*256.f;b=m3g_bits((uint32_t)m3g_int(12102203.f*(-p[11]*end)+1064866805.f))*256.f;}a=a<0?0:a>255?255:a;b=b<0?0:b>255?255:b;fogFactor=m3g_int(a*256.f);fogStep=m3g_int((b-fogFactor*.00390625f)*(1.f/len)*256.f);}next+=len;}
  uint32_t pixel=(uint32_t)s[1];if(flags&1){pixel=((color[0]>>16)&255)<<24|((color[1]>>16)&255)<<16|((color[2]>>16)&255)<<8|((color[3]>>16)&255);for(int k=0;k<4;k++)color[k]+=(uint32_t)s[14+k];}
  int16_t zz=(int16_t)m3g_int(z);if(zbuf[offset]<zz)continue;
  if(flags&2){int xx=m3g_wrap(m3g_int(u*inv+32768.f)-32768,tw,s[8]),yy=m3g_wrap(m3g_int(v*inv+32768.f)-32768,th,s[9]);uint32_t t=tex[yy*tw+xx];pixel=s[5]==0?t:s[5]==3?(t&0xff000000)|(pixel&0xffffff):m3g_mod(pixel,t,s[5]);}
  int alpha=((pixel>>24)*s[3])>>8;if(alpha<s[2])continue;if(!alpha)pixel|=0xff000000;if(flags&8)zbuf[offset]=zz;if(!(flags&16))continue;
  if(flags&32){int amount=fogFactor>>8;uint32_t rb=s[18]&0xff00ff,gr=(s[18]>>8)&255;rb=(rb+(((pixel&0xff00ff)-rb)*amount>>8))&0xff00ff;gr=gr+((((pixel>>8)&255)-gr)*amount>>8);pixel=(pixel&0xff000000)|rb|(gr<<8);}
  d[offset]=m3g_blend(d[offset],pixel,alpha,s[4]);
 }
 return 0;
}
