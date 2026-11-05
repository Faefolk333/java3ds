#include <3ds.h>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include "cpu_probe.h"
#include "touch_mapper.h"
#include "loader_probe.h"
#include "image_probe.h"
#include "version.h"
#include "../font8x8.h"
u32 __stacksize__=256*1024;
u32 __ctru_linear_heap_size=2*1024*1024;
static bool alive=true;
static bool imageCancelled=false;static const char*cancelSource="none";
static bool poll(){if(!alive)return false;if(!aptMainLoop()){cancelSource="APT";return alive=false;}hidScanInput();if(hidKeysDown()&KEY_START){cancelSource="START";return alive=false;}svcSleepThread(1000000);return true;}
static bool pollImage(){if(imageCancelled||!poll())return false;if(hidKeysDown()&KEY_B){cancelSource="B";imageCancelled=true;return false;}return true;}
static u64 diagnosticStarted=0,lastProgress=0;static const char*lastStage=nullptr;
static void showProgress(const char*stage,std::uint32_t done,std::uint32_t total){
 u64 now=osGetTime();if(lastStage&&strcmp(stage,lastStage)==0&&done!=total&&now-lastProgress<100)return;
 lastStage=stage;lastProgress=now;
 printf("\x1b[7;1H\x1b[2K%s\n\x1b[2K",stage);
 if(total){unsigned percent=(unsigned)((u64)done*100/total);printf("%u%%  ",percent);
  if(strstr(stage,"Reading")||strstr(stage,"memory"))printf("%lu / %lu KiB",(unsigned long)(done/1024),(unsigned long)((total+1023)/1024));
  else printf("%lu / %lu",(unsigned long)done,(unsigned long)total);
 }
 printf("\n\x1b[2KElapsed: %lu s\n",(unsigned long)((now-diagnosticStarted)/1000));gfxFlushBuffers();
}
static void pixel(u8*b,int x,int y,unsigned color){if(x<0||x>=320||y<0||y>=240)return;int i=(x*240+239-y)*3;b[i]=color;b[i+1]=color>>8;b[i+2]=color>>16;}
static void text(u8*b,int x,int y,const char*s,unsigned color){for(;*s;s++,x+=8)for(int row=0;row<8;row++)for(int col=0;col<8;col++)if(font8x8_basic[(unsigned char)*s][row]&(1<<col))pixel(b,x+col,y+row,color);}
static void circle(u8*b,int cx,int cy,int radius,unsigned color){for(int y=-radius;y<=radius;y++)for(int x=-radius;x<=radius;x++)if(x*x+y*y<=radius*radius&&(x*x+y*y>=(radius-2)*(radius-2)))pixel(b,cx+x,cy+y,color);}
static const char*status(bool okay){return okay?"PASS":"FAIL";}
int main(){
 gfxInitDefault();gfxSet3D(false);gfxSetDoubleBuffering(GFX_TOP,false);gfxSetDoubleBuffering(GFX_BOTTOM,true);osSetSpeedupEnable(true);consoleInit(GFX_TOP,nullptr);
 printf("JAVA3DS // SYMBIAN CPU PROBE\n\nEKA2L1 dyncom on ARM11\nNo phone OS or SIS games yet.\n\nRunning instruction checks...\nSTART: cancel\n");
 gfxFlushBuffers();
 auto r=symbianCpuProbe(poll);if(!alive){gfxExit();return 0;}
 printf("\nTesting EKA1 loader and guarded memory...\n");auto loader=symbianLoaderProbe();
 if(!poll()){gfxExit();return 0;}bool newModel=false;APT_CheckNew3DS(&newModel);consoleClear();
 printf("JAVA3DS // SYMBIAN PROBE " SYMBIAN_PROBE_VERSION "\n\n%s\nARM: %s  Thumb: %s\nExclusive: %s  VFP: %s\nContext: %s  Budget: %s\nFaults: %u  Cache: 8 MiB\n\n%llu instructions / %llu us\nCPU-only: %.2f M instr/s\n\nEKA1 loader: %s  Relocations: %s\nE32 call: %s  Import stop: %s\nSVC stop: %s  Memory guards: %s\nBudget: %s  Cancel: %s\n\nNo phone OS or SIS games yet.\nY: test supplied EKA1 image on SD\nPad / Circle: virtual joystick\nA / B: virtual action buttons\nTouch: direct pointer\nX: portrait / landscape\nSTART: exit\n",newModel?"New 3DS":"Old 3DS",status(r.arm),status(r.thumb),status(r.exclusive),status(r.vfp),status(r.context),status(r.budget),r.faults,(unsigned long long)r.benchmarkInstructions,(unsigned long long)r.benchmarkMicros,r.benchmarkMicros?double(r.benchmarkInstructions)/r.benchmarkMicros:0,status(loader.parsed),status(loader.relocated),status(loader.returned),status(loader.imports),status(loader.systemCall),status(loader.protectedMemory),status(loader.bounded),status(loader.cancelled));
 mkdir("/3ds",0777);mkdir("/3ds/java-3ds",0777);FILE*log=fopen("/3ds/java-3ds/symbian-probe.log","w");if(log){fprintf(log,"Symbian CPU probe " SYMBIAN_PROBE_VERSION "; no OS/game execution\nnew_model=%d arm=%d thumb=%d exclusive=%d vfp=%d context=%d bounded_slice=%d faults=%u cache_bytes=8388608 instructions=%llu cpu_us=%llu\n",newModel,r.arm,r.thumb,r.exclusive,r.vfp,r.context,r.budget,r.faults,(unsigned long long)r.benchmarkInstructions,(unsigned long long)r.benchmarkMicros);fprintf(log,"loader_parsed=%d relocated=%d returned=%d import_stop=%d svc_stop=%d memory_guards=%d bounded=%d cancelled=%d\n",loader.parsed,loader.relocated,loader.returned,loader.imports,loader.systemCall,loader.protectedMemory,loader.bounded,loader.cancelled);fclose(log);}
 symbian::TouchProfile profile;symbian::TouchMapper mapper(profile);int pointerX=0,pointerY=0;bool pointer=false;unsigned events=0;
 while(aptMainLoop()){
  hidScanInput();u32 held=hidKeysHeld(),down=hidKeysDown();if(down&KEY_START)break;
  if(down&KEY_Y){
   mapper.cancel();pointer=false;imageCancelled=false;cancelSource="none";diagnosticStarted=osGetTime();lastProgress=0;lastStage=nullptr;
   consoleClear();printf("SYMBIAN // EKA1 IMAGE DIAGNOSTIC\n\nLoading your test.app / SYM.ROM...\nB: cancel test    START: exit\nNo phone OS or gameplay yet.\n");
   gfxFlushBuffers();
   FILE*report=fopen("/3ds/java-3ds/symbian-loader.log","w");
   bool supplied=report&&symbianImageProbe("/3ds/java-3ds/symbian",report,pollImage,showProgress);
   if(report){if(imageCancelled||!alive)fprintf(report,"PLATFORM_CANCEL source=%s\n",cancelSource);fclose(report);}
   if(!alive)break;
   printf("\x1b[11;1H");
   if(imageCancelled)printf("\nDiagnostic cancelled. Y: retry\n");
   else printf(supplied?"\nDiagnostic finished.\nReport: /3ds/java-3ds/symbian-loader.log\n":"\nInput missing or log cannot be written.\nPut test.app in /3ds/java-3ds/symbian\n");
   printf("\nThis does not launch the actual game.\nY: run test again    START: exit\n");continue;
  }
  if(down&KEY_X){mapper.cancel();std::swap(profile.width,profile.height);mapper=symbian::TouchMapper(profile);pointer=false;}
  circlePosition pad;hidCircleRead(&pad);symbian::PadSample s;s.x=pad.dx;s.y=pad.dy;
  if(held&KEY_UP)s.buttons|=1;if(held&KEY_DOWN)s.buttons|=2;if(held&KEY_LEFT)s.buttons|=4;if(held&KEY_RIGHT)s.buttons|=8;if(held&KEY_A)s.buttons|=16;if(held&KEY_B)s.buttons|=32;
  if(held&KEY_TOUCH){touchPosition touch;hidTouchRead(&touch);s.touch=true;s.touchX=touch.px*1000/319;s.touchY=touch.py*1000/239;}
  auto batch=mapper.update(s);for(unsigned i=0;i<batch.count;i++){auto e=batch.events[i];pointer=e.phase!=symbian::TouchPhase::Up;pointerX=e.x;pointerY=e.y;events++;}
  auto b=gfxGetFramebuffer(GFX_BOTTOM,GFX_LEFT,nullptr,nullptr);memset(b,0,320*240*3);for(int x=0;x<320;x+=20)for(int y=24;y<240;y++)pixel(b,x,y,0x19253c);for(int y=24;y<240;y+=20)for(int x=0;x<320;x++)pixel(b,x,y,0x19253c);
  text(b,12,8,"SYMBIAN // TOUCH MAPPING",0x83eaff);circle(b,70,187,31,0x81d7f0);circle(b,281,187,17,0x93e7ff);circle(b,236,211,15,0xb3a9f0);text(b,278,184,"A",0xffffff);text(b,233,208,"B",0xffffff);
  char label[72];snprintf(label,sizeof(label),"PHONE %dx%d  EVENTS %u",profile.width,profile.height,events);text(b,8,35,label,0xbccfee);snprintf(label,sizeof(label),"%s  X:%d Y:%d",pointer?"DOWN":"UP  ",pointerX,pointerY);text(b,8,51,label,0xffffff);
  if(pointer){int x=pointerX*319/(profile.width-1),y=pointerY*239/(profile.height-1);for(int i=-6;i<=6;i++){pixel(b,x+i,y,0xffffff);pixel(b,x,y+i,0xffffff);}}
  gfxFlushBuffers();gfxSwapBuffers();gspWaitForVBlank();
 }
 gfxExit();return r.passed()&&loader.passed()?0:1;
}
