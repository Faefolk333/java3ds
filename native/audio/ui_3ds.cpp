#include <3ds.h>
#include <cstdio>
#include <cstring>
#include "ui_3ds.h"
static bool ready;static s16* clips[3];static unsigned samples[3];static ndspWaveBuf wave;
static unsigned le32(const unsigned char*p){return p[0]|p[1]<<8|p[2]<<16|p[3]<<24;}
void uiAudioInit(){if(ready)return;if(R_FAILED(ndspInit()))return;ready=true;const char*names[]={"navigation","confirm","achievement"};
 for(int i=0;i<3;i++){char path[96];snprintf(path,sizeof(path),"romfs:/ui/%s.wav",names[i]);FILE*f=fopen(path,"rb");if(!f)continue;unsigned char h[44];if(fread(h,1,44,f)!=44||memcmp(h,"RIFF",4)||memcmp(h+8,"WAVEfmt ",8)||h[20]!=1||h[22]!=1||le32(h+24)!=22050||h[34]!=16||memcmp(h+36,"data",4)){fclose(f);continue;}unsigned n=le32(h+40);if(n&&n<=512*1024&&n%2==0){clips[i]=(s16*)linearAlloc(n);if(clips[i]){if(fread(clips[i],1,n,f)==n&&R_SUCCEEDED(DSP_FlushDataCache(clips[i],n)))samples[i]=n/2;else{linearFree(clips[i]);clips[i]=nullptr;}}}fclose(f);}
 ndspSetOutputMode(NDSP_OUTPUT_STEREO);ndspChnReset(1);ndspChnSetInterp(1,NDSP_INTERP_LINEAR);ndspChnSetRate(1,22050);ndspChnSetFormat(1,NDSP_FORMAT_MONO_PCM16);float mix[12]={};mix[0]=mix[1]=.55f;ndspChnSetMix(1,mix);
}
void uiAudioPlay(UiSound sound){int i=(int)sound;if(!ready||i<0||i>2||!samples[i])return;ndspChnWaveBufClear(1);memset(&wave,0,sizeof(wave));wave.data_pcm16=clips[i];wave.nsamples=samples[i];ndspChnWaveBufAdd(1,&wave);}
void uiAudioClose(){if(!ready)return;ndspChnWaveBufClear(1);for(int i=0;i<3;i++){if(clips[i])linearFree(clips[i]);clips[i]=nullptr;samples[i]=0;}ndspExit();ready=false;}
