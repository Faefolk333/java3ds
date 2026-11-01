#ifdef POP_AUDIO_DEVICE_TEST
#include "../../tests/audio_device_stub.hpp"
#define AUDIO_FONT "romfs/audio/gm.sf2"
#define AUDIO_THEME "romfs/audio/pop-theme.mid"
#else
#include <3ds.h>
#define AUDIO_FONT "romfs:/audio/gm.sf2"
#define AUDIO_THEME "romfs:/audio/pop-theme.mid"
#endif
#include <atomic>
#include <cstdio>
#include <cstring>
#include "midi_engine.h"
#include "device_3ds.h"
static constexpr int RATE=22050,FRAMES=1024,BUFFERS=3;
static MidiEngine* engine;
static MidiEngine* background;
static MidiEngine* effects;
static int theme=-1;
static bool themeEnabled,themeAudible;
static uint32_t midiAudible;
static LightLock audioLock;
static std::atomic<bool> running{false},paused{false};
static Thread audioThread;
static int16_t* output;
static ndspWaveBuf waves[BUFFERS];
static bool dsp;
static int loadTheme(MidiEngine*e){FILE*f=fopen(AUDIO_THEME,"rb");if(!f)return -1;unsigned char data[16384];size_t n=fread(data,1,sizeof(data),f);int extra=fgetc(f);bool ok=!ferror(f)&&extra==EOF;fclose(f);return ok?midi_engine_load(e,data,(int)n):-1;}
static void stream(void*){
 unsigned next=0;
 while(running.load()){
  ndspWaveBuf*w=&waves[next];
  if(w->status==NDSP_WBUF_FREE||w->status==NDSP_WBUF_DONE){
   int16_t*pcm=output+next*FRAMES;
   LightLock_Lock(&audioLock);
   int active=midi_engine_current(engine);bool melody=active>=0&&midi_engine_control(engine,active,MIDI_KIND,0)==1;
   if(paused.load()){memset(pcm,0,FRAMES*sizeof(*pcm));LightLock_Unlock(&audioLock);goto publish_buffer;}
   midi_engine_render(engine,pcm,FRAMES);
   int16_t fx[FRAMES];midi_engine_render(effects,fx,FRAMES);for(int i=0;i<FRAMES;i++){int mixed=(int)pcm[i]+fx[i];pcm[i]=(int16_t)(mixed < -32768?-32768:mixed>32767?32767:mixed);}
   if(melody&&!(midiAudible&(1u<<active))){int peak=0;for(int i=0;i<FRAMES;i++){int n=pcm[i]<0?-pcm[i]:pcm[i];if(n>peak)peak=n;}if(peak){midiAudible|=1u<<active;fprintf(stderr,"AUDIO midi PCM handle=%d peak=%d\n",active,peak);}}
   // Effects mix over the music. Original MIDI jingles temporarily take its place.
   if(themeEnabled&&!melody){int16_t music[FRAMES];midi_engine_render(background,music,FRAMES);int peak=0;
    for(int i=0;i<FRAMES;i++){int n=music[i]<0?-music[i]:music[i];if(n>peak)peak=n;int mixed=(int)pcm[i]+music[i];pcm[i]=(int16_t)(mixed < -32768?-32768:mixed>32767?32767:mixed);}
    if(peak&&!themeAudible){themeAudible=true;fprintf(stderr,"AUDIO background PCM peak=%d\n",peak);}
   }
   LightLock_Unlock(&audioLock);
   publish_buffer:
   memset(w,0,sizeof(*w));w->data_pcm16=pcm;w->nsamples=FRAMES;
   if(R_FAILED(DSP_FlushDataCache(pcm,FRAMES*sizeof(*pcm)))){fprintf(stderr,"AUDIO cache failure\n");running.store(false);break;}
   ndspChnWaveBufAdd(0,w);next=(next+1)%BUFFERS;
  }else svcSleepThread(2000000);
 }
}
extern "C" void crimson_audio_pause(int value){paused.store(value!=0);}
extern "C" int crimson_audio_init(){
 paused.store(false);
 LightLock_Init(&audioLock);
 Result status=ndspInit();if(R_FAILED(status)){fprintf(stderr,"AUDIO ndspInit failed %08lx (check /3ds/dspfirm.cdc)\n",(unsigned long)status);return -1;}dsp=true;
 engine=midi_engine_open(AUDIO_FONT,RATE);
 if(!engine){fprintf(stderr,"AUDIO soundfont unavailable\n");crimson_audio_shutdown();return -2;}
 effects=midi_engine_clone(engine);if(!effects){crimson_audio_shutdown();return -2;}
 background=midi_engine_clone(engine);theme=background?loadTheme(background):-1;
 if(theme<0){fprintf(stderr,"AUDIO background unavailable\n");crimson_audio_shutdown();return -2;}
 themeEnabled=themeAudible=false;midiAudible=0;
 midi_engine_control(background,theme,MIDI_VOLUME,80);
 output=(int16_t*)linearAlloc(FRAMES*BUFFERS*sizeof(*output));if(!output){crimson_audio_shutdown();return -3;}
 memset(waves,0,sizeof(waves));ndspSetOutputMode(NDSP_OUTPUT_STEREO);ndspChnReset(0);
 ndspChnSetInterp(0,NDSP_INTERP_LINEAR);ndspChnSetRate(0,RATE);ndspChnSetFormat(0,NDSP_FORMAT_MONO_PCM16);
 float mix[12]={};mix[0]=mix[1]=1.f;ndspChnSetMix(0,mix);
 running.store(true);bool newModel=false;int audioCore=-2;
 if(!R_FAILED(APT_CheckNew3DS(&newModel))&&newModel){audioThread=threadCreate(stream,nullptr,64*1024,0x2f,2,false);if(audioThread)audioCore=2;}
 if(!audioThread)audioThread=threadCreate(stream,nullptr,64*1024,0x2f,-2,false);
 if(!audioThread){running.store(false);crimson_audio_shutdown();return -4;}
 fprintf(stderr,"AUDIO ready: MIDI/WAV/AMR, %d Hz, core %d\n",RATE,audioCore);return 0;
}
extern "C" void crimson_audio_shutdown(){
 running.store(false);if(audioThread){threadJoin(audioThread,U64_MAX);threadFree(audioThread);audioThread=nullptr;}
 if(dsp){ndspChnWaveBufClear(0);ndspExit();dsp=false;}
 if(output){linearFree(output);output=nullptr;}midi_engine_close(effects);effects=nullptr;midi_engine_close(background);background=nullptr;theme=-1;themeEnabled=false;midi_engine_close(engine);engine=nullptr;
}
extern "C" int crimson_audio_load(const void*data,int size){
 if(!engine||!running.load())return -1;
 LightLock_Lock(&audioLock);bool wave=size>=4&&!memcmp(data,"RIFF",4);int id=midi_engine_load(wave?effects:engine,data,size);if(id>=0){if(wave)id+=32;else midiAudible&=~(1u<<id);}LightLock_Unlock(&audioLock);
 fprintf(stderr,"AUDIO load handle=%d bytes=%d type=%s\n",id,size,size>=4&&!memcmp(data,"MThd",4)?"MIDI":"WAV");return id;
}
extern "C" int crimson_audio_control(int id,int operation,int value){
 if(!engine||!running.load())return operation==MIDI_STATE?300:-1;
 LightLock_Lock(&audioLock);int result=midi_engine_control(id>=32?effects:engine,id>=32?id-32:id,operation,value);LightLock_Unlock(&audioLock);
 if(operation==MIDI_START||operation==MIDI_STOP||operation==MIDI_CLOSE)fprintf(stderr,"AUDIO command handle=%d op=%d value=%d result=%d\n",id,operation,value,result);
 return result;
}
extern "C" void crimson_audio_scene(int state,int level,int enabled){
 if(!engine||!running.load()||theme<0)return;
 bool play=enabled&&(state==3||(state==10&&level>=0&&level<14));
 LightLock_Lock(&audioLock);
 if(play!=themeEnabled){themeEnabled=play;if(play){midi_engine_control(background,theme,MIDI_START,-1);themeAudible=false;}
  // Pause by gating rendering: preserve the synthesizer's live voice phases.
  // STOP + resuming would replay up to a whole track under the VM lock.
  else if(!enabled){midi_engine_control(background,theme,MIDI_STOP,0);midi_engine_control(background,theme,MIDI_SEEK,0);}
  fprintf(stderr,"AUDIO background enabled=%d state=%d level=%d\n",play,state,level);}
 LightLock_Unlock(&audioLock);
}
