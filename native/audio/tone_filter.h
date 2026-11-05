#pragma once
#include <cmath>
#include <stdint.h>
// Optional output tone shaping. Original bypasses it; Soft/Warm reduce treble.
struct AudioToneFilter {
 int mode=-1,rate=0;float b0=1,b1=0,b2=0,a1=0,a2=0,x1=0,x2=0,y1=0,y2=0;
 void reset(){x1=x2=y1=y2=0;}
 void configure(int value,int sampleRate){if(value<0||value>2)value=1;if(mode==value&&rate==sampleRate)return;mode=value;rate=sampleRate;reset();if(!mode)return;float frequency=mode==1?5200.f:3600.f;if(frequency>rate*.4f)frequency=rate*.4f;float w=6.28318530718f*frequency/rate,c=cosf(w),alpha=sinf(w)*.70710678118f,n=1+alpha;b0=(1-c)*.5f/n;b1=(1-c)/n;b2=b0;a1=-2*c/n;a2=(1-alpha)/n;}
 void apply(int16_t*pcm,int count){if(mode<=0)return;for(int i=0;i<count;i++){float x=pcm[i],y=b0*x+b1*x1+b2*x2-a1*y1-a2*y2;x2=x1;x1=x;y2=y1;y1=y;if((i&63)==63&&fabsf(y1)+fabsf(y2)+fabsf(x1)+fabsf(x2)<1e-12f)reset();int sample=(int)(y+(y<0?-.5f:.5f));pcm[i]=(int16_t)(sample<-32768?-32768:sample>32767?32767:sample);}if(fabsf(y1)+fabsf(y2)+fabsf(x1)+fabsf(x2)<1e-12f)reset();}
};
