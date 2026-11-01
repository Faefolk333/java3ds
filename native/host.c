#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "raster.h"
#include "audio/midi_engine.h"
static volatile LONG frames;static unsigned peak;static int pauseFailed;static ULONGLONG start;static const char*config;static int autoplay,laststate=-999;
const char*crimson_launch_config(void){return config;}
void crimson_gc_wait_report(const char*s){fprintf(stderr,"GC_WAIT %s\n",s);}
void crimson_stage(int s,int d){
#ifdef CRIMSON_CP_ALLOCATION_TEST
 if(s==85){extern void crimson_arm_cp_failure(void);crimson_arm_cp_failure();}
#endif
if(s==60){if(laststate!=d){printf("GAME_STATE %d\n",d);laststate=d;}}else if(s>=40&&s!=61&&s!=63)printf("STAGE %d %d\n",s,d);}
void crimson_music(int a,int b,int c){}
static MidiEngine*audio;static CRITICAL_SECTION audioLock;static volatile LONG audioRunning;
static DWORD WINAPI audio_stream(void*p){int16_t buffer[220];ULONGLONG runStart=0;int phase=0;LONG freeze=0;while(InterlockedCompareExchange(&audioRunning,0,0)){
 if(getenv("JAVA_PAUSE_TEST")&&start){ULONGLONG t=GetTickCount64()-start;if(runStart!=start){runStart=start;phase=0;}
  if(phase==0&&t>500){crimson_pause_game(1);phase=1;printf("PAUSE_REQUEST\n");}
  if(phase==1&&t>750){freeze=InterlockedCompareExchange(&frames,0,0);phase=2;}
  if(phase==2&&t>1250){if(InterlockedCompareExchange(&frames,0,0)!=freeze)pauseFailed=1;crimson_pause_game(0);phase=3;printf("PAUSE_FROZEN frames=%ld\n",freeze);}
  if(phase==3&&t>1750){if(InterlockedCompareExchange(&frames,0,0)<=freeze)pauseFailed=1;printf("PAUSE_RESUMED frames=%ld\n",InterlockedCompareExchange(&frames,0,0));phase=4;if(atoi(getenv("JAVA_PAUSE_TEST"))==2)crimson_pause_game(1);}
 }
 if(start&&GetTickCount64()-start>(getenv("JAVA_ABORT_MS")?atoi(getenv("JAVA_ABORT_MS")):(getenv("JAVA_RUN_MS")?atoi(getenv("JAVA_RUN_MS"))+2000:32000)))crimson_request_stop();EnterCriticalSection(&audioLock);midi_engine_render(audio,buffer,220);LeaveCriticalSection(&audioLock);for(int i=0;i<220;i++){unsigned a=abs(buffer[i]);if(a>peak)peak=a;}Sleep(10);}return 0;}
int crimson_audio_load(const void*p,int n){EnterCriticalSection(&audioLock);int h=midi_engine_load(audio,p,n);LeaveCriticalSection(&audioLock);printf("VM_AUDIO_LOAD handle=%d bytes=%d\n",h,n);return h;}
int crimson_audio_control(int h,int op,int v){EnterCriticalSection(&audioLock);int r=midi_engine_control(audio,h,op,v);LeaveCriticalSection(&audioLock);if(op==MIDI_START)printf("VM_AUDIO_START %d result %d\n",h,r);return r;}
void crimson_publish(const uint32_t*p){InterlockedIncrement(&frames);if(frames%(getenv("JAVA_CAPTURE_EVERY")?atoi(getenv("JAVA_CAPTURE_EVERY")):100)==0){FILE*f=fopen("work/host-frame.ppm","wb");if(f){fprintf(f,"P6\n%d %d\n255\n",crimson_width,crimson_height);for(int i=0;i<crimson_width*crimson_height;i++){unsigned char rgb[3]={p[i]>>16,p[i]>>8,p[i]};fwrite(rgb,1,3,f);}fclose(f);}}}
int crimson_motion(int kind,int sample[3]){sample[0]=kind?1250:0;sample[1]=kind?-2500:0;sample[2]=kind?3750:9807;return kind==0||kind==1;}
int crimson_buttons(void){ULONGLONG t=GetTickCount64()-start;if(t>(getenv("JAVA_RUN_MS")?atoi(getenv("JAVA_RUN_MS")):30000))return 0x40000000;const char*script=getenv("JAVA_INPUT");if(script&&t>2000&&t%1600<200){if(!strcmp(script,"softkeys"))return t<16000?64:16;if(!strcmp(script,"blades"))return t<6500?8192:t<15000?64:16;if(!strcmp(script,"soul-title"))return t>=9500&&t<10000?64:0;if(!strcmp(script,"soul"))return t<14000?64:16;if(!strcmp(script,"mm2"))return t<24000?32:t<32000?64:16;}return autoplay&&t>2000&&t%1800<200?16:0;}
int main(int argc,char**argv){
#ifdef CRIMSON_CP_ALLOCATION_TEST
 extern int crimson_constant_pool_test(void);if(!getenv("JAVA_TEST_CP_OOM_VM"))return crimson_constant_pool_test();
#endif
if(argc<5)return 1;config=argv[4];autoplay=argc>7;if(argc>5)crimson_width=atoi(argv[5]);if(argc>6)crimson_height=atoi(argv[6]);InitializeCriticalSection(&audioLock);audio=midi_engine_open("romfs/audio/gm.sf2",22050);if(!audio)return 2;InterlockedExchange(&audioRunning,1);HANDLE stream=CreateThread(NULL,0,audio_stream,NULL,0,NULL);int r=0;for(int run=0;run<(getenv("JAVA_TEST_RUNS")?atoi(getenv("JAVA_TEST_RUNS")):2);run++){start=GetTickCount64();frames=0;r=crimson_run_game(argv[1],argv[2],argv[3]);printf("VM_EXIT run=%d result=%d frames=%u peak=%u\n",run,r,frames,peak);fflush(stdout);if(r)break;}InterlockedExchange(&audioRunning,0);WaitForSingleObject(stream,INFINITE);CloseHandle(stream);midi_engine_close(audio);DeleteCriticalSection(&audioLock);if(getenv("JAVA_PAUSE_TEST")){printf("PAUSE_TEST %s\n",pauseFailed?"FAIL":"PASS");if(pauseFailed)return 3;}return r;}
