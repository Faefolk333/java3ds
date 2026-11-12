#include <3ds.h>
#include <atomic>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include "catalog.h"
#include "updater.h"
#include "raster.h"
#include "stereo.h"
#include "stereo_display.h"
#include "display_filter.h"
#include "asset_install.hpp"
#include "audio/device_3ds.h"
#include "font8x8.h"
#include "motion.h"
#include "bottom_screen.h"
#include "lifecycle_gate.h"
#include "text_input.h"
#include "../third-party/minijvm/minijvm/c/utils/crimson_thread_policy.h"
#include "audio/ui_3ds.h"
u32 __stacksize__=1024*1024;
#ifdef JAVA3DS_CIA
// Software framebuffers/audio need < 2 MiB; preserve the Old 3DS VM heap.
u32 __ctru_linear_heap_size=4*1024*1024;
#endif
static const std::string ROOT="/3ds/java-3ds";
static LightLock frameLock;static uint32_t front[512*512];static int8_t frontDepth[512*512];static std::atomic<unsigned>held{0},generation{0};
static std::atomic<bool>done{false},exitRequested{false},failed{false};static int result;static Thread vm;
static std::string launchConfig,activeJar;static bool popProfile;
extern "C" const char*crimson_launch_config(){return launchConfig.c_str();}
static std::atomic<unsigned>motionRequests{0},motionReady{0};static std::atomic<int>motionValues[6];
extern "C" int crimson_motion(int kind,int sample[3]){if(kind<0||kind>1)return 0;motionRequests.fetch_or(1u<<kind);if(!(motionReady.load()&(1u<<kind)))return 0;for(int i=0;i<3;i++)sample[i]=motionValues[kind*3+i].load();return 1;}
static void sensors(){static unsigned attempted=0;static float gyroCoefficient=0;unsigned requested=motionRequests.load();if((requested&1)&&!(attempted&1)){attempted|=1;if(R_SUCCEEDED(HIDUSER_EnableAccelerometer()))motionReady.fetch_or(1);}if((requested&2)&&!(attempted&2)){attempted|=2;if(R_SUCCEEDED(HIDUSER_EnableGyroscope())&&R_SUCCEEDED(HIDUSER_GetGyroscopeRawToDpsCoefficient(&gyroCoefficient))&&gyroCoefficient>0)motionReady.fetch_or(2);}
 if(motionReady.load()&1){accelVector a;hidAccelRead(&a);motionValues[0]=motionAcceleration(a.x);motionValues[1]=motionAcceleration(a.y);motionValues[2]=motionAcceleration(a.z);}if(motionReady.load()&2){angularRate g;hidGyroRead(&g);motionValues[3]=motionRotation(g.x,gyroCoefficient);motionValues[4]=motionRotation(g.y,gyroCoefficient);motionValues[5]=motionRotation(g.z,gyroCoefficient);}}
extern "C" int crimson_buttons(){return held.load();}
extern "C" void crimson_music(int,int,int){}
extern "C" void crimson_gc_wait_report(const char*s){fprintf(stderr,"GC %s\n",s);}
extern "C" void crimson_publish(const uint32_t*p){LightLock_Lock(&frameLock);memcpy(front,p,crimson_width*crimson_height*sizeof(*p));if(crimson_stereo_mode())crimson_stereo_encode(frontDepth,crimson_width*crimson_height);generation.fetch_add(1);LightLock_Unlock(&frameLock);}
extern "C" void crimson_stage(int s,int d){if(s==80)exitRequested.store(true);if(s==90)failed.store(true);if(s==82&&popProfile)crimson_audio_scene((d>>8)&255,d&255,(d>>16)&1);if(s>=40&&s!=60&&s!=61&&s!=63&&s!=64&&s!=82)fprintf(stderr,"VM stage %d %d\n",s,d);}
static GameInfo loadingGame;static GameSettings loadingOptions;static std::string loadingError;
static void worker(void*){if(!prepareGame(loadingGame,loadingOptions,"romfs:/profiles/pop-classic.zip",activeJar,loadingError,[](){return (held.load()&0x40000000)!=0;})||!writeLaunchConfig(loadingGame,loadingOptions,ROOT+"/saves",launchConfig,loadingError)){result=-1;done.store(true);return;}if(held.load()&0x40000000){result=0;done.store(true);return;}crimson_audio_tone(loadingOptions.tone);crimson_audio_init();std::string cp=ROOT+"/runtime/midp.jar:"+activeJar;result=crimson_run_game((ROOT+"/runtime/minijvm_rt.jar").c_str(),cp.c_str(),"crimson.GameMain");done.store(true);}
struct TextRequest {std::atomic<int>state{0};char label[128]={},initial[1024]={},result[1024]={};int maximum=255,constraints=0,confirmed=0;};
static TextRequest textRequest;
extern "C" int crimson_text_input(const char*label,const char*initial,int maximum,int constraints,char*result,size_t capacity){
 if(maximum<1||capacity<1)return 0;
 int expected=0;if(!textRequest.state.compare_exchange_strong(expected,-1))return 0;
 snprintf(textRequest.label,sizeof(textRequest.label),"%s",label);snprintf(textRequest.initial,sizeof(textRequest.initial),"%s",initial);textRequest.maximum=std::min(maximum,255);textRequest.constraints=constraints;textRequest.state.store(1);
 for(;;){int state=textRequest.state.load();if(state==3){int accepted=textRequest.confirmed;if(accepted)snprintf(result,capacity,"%s",textRequest.result);textRequest.state.store(0);return accepted;}
 if(crimson_cancelled()){textRequest.state.store(4);return -1;}svcSleepThread(10000000);}
}
static void pollTextInput(){
 if(textRequest.state.load()==4){textRequest.state.store(0);return;}
 int expected=1;if(!textRequest.state.compare_exchange_strong(expected,2))return;
 SwkbdState keyboard;swkbdInit(&keyboard,SWKBD_TYPE_NORMAL,2,textRequest.maximum);swkbdSetHintText(&keyboard,textRequest.label);swkbdSetInitialText(&keyboard,textRequest.initial);if(textRequest.constraints&65536)swkbdSetPasswordMode(&keyboard,SWKBD_PASSWORD_HIDE);
 char answer[1024]={};bool confirmed=swkbdInputText(&keyboard,answer,sizeof(answer))==SWKBD_BUTTON_CONFIRM;
 if(textRequest.state.load()==4){textRequest.state.store(0);return;}
 textRequest.confirmed=confirmed;if(confirmed)snprintf(textRequest.result,sizeof(textRequest.result),"%s",answer);textRequest.state.store(3);
}
struct Surface{
 u8*data;int width;
 void pixel(int x,int y,unsigned c){if(x<0||y<0||x>=width||y>=240)return;int o=(x*240+239-y)*3;data[o]=c;data[o+1]=c>>8;data[o+2]=c>>16;}
 void rect(int x,int y,int w,int h,unsigned c){for(int yy=y;yy<y+h;yy++)for(int xx=x;xx<x+w;xx++)pixel(xx,yy,c);}
 void text(int x,int y,const std::string&s,unsigned color=0xe8eef5,int max=48){int n=0;for(unsigned char c:s){if(n++>=max)break;if(c<32||c>126)c='?';for(int row=0;row<8;row++)for(int col=0;col<8;col++)if(font8x8_basic[c][row]&(1<<col))pixel(x+col,y+row,color);x+=8;}}
};
// All UI painting is off-screen. Keep both screens across buffer swaps.
static u8 topImage[400*240*3],rightImage[400*240*3],bottomImage[320*240*3];static bool topStereo;
static Surface top(){return {topImage,400};}static Surface bottom(){return {bottomImage,320};}
static const char*keys[]={"Phone / D-pad","Numeric keypad","PoP: B jump, A action"};
static void flush(){memcpy(gfxGetFramebuffer(GFX_TOP,GFX_LEFT,nullptr,nullptr),topImage,sizeof(topImage));if(topStereo)memcpy(gfxGetFramebuffer(GFX_TOP,GFX_RIGHT,nullptr,nullptr),rightImage,sizeof(rightImage));memcpy(gfxGetFramebuffer(GFX_BOTTOM,GFX_LEFT,nullptr,nullptr),bottomImage,sizeof(bottomImage));gfxFlushBuffers();gfxSwapBuffers();}
static unsigned blend(unsigned a,unsigned b,int t,int n){unsigned c=0;for(int shift=0;shift<24;shift+=8)c|=(( ((a>>shift)&255)*(n-t)+((b>>shift)&255)*t)/n)<<shift;return c;}
static void chrome(Surface s,int x,int y,int w,int h,bool selected=false){for(int j=0;j<h;j++)s.rect(x,y+j,w,1,blend(selected?0x78edff:0xe8e9ff,selected?0x3651ab:0x626386,j,h));s.rect(x,y,w,1,0xf6ffff);s.rect(x,y,1,h,0xe4f8ff);s.rect(x+w-1,y,1,h,0x25213f);s.rect(x,y+h-1,w,1,0x25213f);}
static int parallaxX=0,parallaxY=0;
static void shell(Surface s,const std::string&title){for(int y=0;y<240;y++)s.rect(0,y,s.width,1,blend(0x21134b,0x071e38,y,240));for(int x=parallaxX-20;x<s.width;x+=20)s.rect(x,33,1,207,0x253558);for(int y=40+parallaxY;y<240;y+=20)s.rect(0,y,s.width,1,0x253558);chrome(s,4,4,s.width-8,26);s.text(12,13,title,0x171b42,(s.width-24)/8);s.rect(8,34,s.width-16,2,0x66e7ff);}
static unsigned keyLights(unsigned raw,const GameSettings&){return (raw>>16)&0x3fff;}
static void keypad(unsigned lit=0,bool hidden=false){auto b=bottom();if(hidden){b.rect(0,0,320,240,0);return;}shell(b,"JAVA3DS // TOUCH KEYPAD");const char*soft[]={"LEFT","MENU","RIGHT"};for(int i=0;i<3;i++){int x=13+i*100;bool down=i!=1&&(lit&(1u<<(i==0?12:13)));chrome(b,x,37,94,25,down);b.text(x+47-strlen(soft[i])*4,46,soft[i],0x151c45);}const char*labels[]={"1","2 ABC","3 DEF","4 GHI","5 JKL","6 MNO","7 PQRS","8 TUV","9 WXYZ","*","0","#"};for(int i=0;i<12;i++){int x=13+i%3*100,y=72+i/3*38;bool down=lit&(1u<<i);chrome(b,x,y,94,34,down);if(down){b.rect(x,y,94,2,0x18223c);b.rect(x,y,2,34,0x18223c);b.rect(x+2,y+32,92,2,0xc2faff);}else b.rect(x+3,y+3,88,11,0xc5d4f6);b.text(x+47-strlen(labels[i])*4+(down?1:0),y+14+(down?2:0),labels[i],0x151c45,8);}b.text(44,230,"TAP MENU FOR GAME SETTINGS",0xa1ebff,32);}
static void infoBottom(const std::string&a,const std::string&b,const std::string&c){auto s=bottom();shell(s,"JAVA3DS // 0.4.0");s.text(10,50,a,0xffffff,37);s.text(10,72,b,0xa1ebff,37);s.text(10,98,c,0xe3c2ff,37);chrome(s,8,134,304,72);s.text(16,146,"A: OPEN / CONFIRM",0x151c45,36);s.text(16,166,"B: BACK    X: REFRESH",0x151c45,36);s.text(16,186,"JAR + JAD // SD LIBRARY",0x151c45,36);s.text(8,222,"JAR: GAME   JAD: DESCRIPTOR",0xa1ebff,38);}
static void browser(const std::string&folder,const std::vector<BrowserEntry>&entries,int selected,const std::string&error){auto s=top();shell(s,"JAVA3DS // SD LIBRARY");s.text(12,40,folder,0xb7dcff,47);int start=(selected/9)*9;for(int i=start;i<(int)entries.size()&&i<start+9;i++){int y=60+(i-start)*17;if(i==selected)chrome(s,8,y-4,384,17,true);s.text(14,y,std::string(entries[i].folder?"[+] ":" >  ")+(entries[i].folder?"":(entries[i].name.size()>4&&entries[i].name.substr(entries[i].name.size()-4)==".jad"?"[JAD] ":"[JAR] "))+entries[i].name,i==selected?0x101c41:0xe0e8ff,46);}if(entries.empty())s.text(12,80,"Add JAR / JAD files to this folder.",0xffffff);s.text(12,222,error.empty()?"Y: WI-FI UPDATES    START: EXIT":error,0xffc7e9,47);infoBottom(entries.empty()?"LIBRARY EMPTY":entries[selected].name,"Select a game or open a folder.","UP/DOWN: SELECT  LEFT/RIGHT: PAGE");flush();}
static const char*displayLabels[]={"Scale to fit","Fill screen","Native pixels"};
static const char*orientationLabels[]={"Auto","Portrait","Landscape"};
static const char*toneLabels[]={"Original","Soft","Warm"};
static void details(const GameInfo&g,const GameSettings&o,int row,const std::string&error){auto s=top();shell(s,g.title);std::string lines[]={"Launch game","Display: "+std::string(displayLabels[o.scale]),"Controls / key bindings...","MIDlet: "+g.midlets[o.midlet].title,"Phone screen: "+std::string(orientationLabels[o.orientation]),"Back"};for(int i=0;i<6;i++){int y=49+i*26;if(i==row)chrome(s,8,y-5,384,23,true);s.text(14,y,lines[i],i==row?0x101c41:0xe0e8ff,46);}s.text(10,211,error.empty()?"Phone canvas: "+std::to_string(o.width)+" x "+std::to_string(o.height):error,0xffc7e9,47);infoBottom(g.midlets[o.midlet].main,"LEFT/RIGHT: CHANGE OPTION","MIDI / WAV / AMR audio");flush();}
static const char*physical[]={"D-pad / stick UP","D-pad / stick DOWN","D-pad / stick LEFT","D-pad / stick RIGHT","A","B","SELECT","X","Y","R","L","ZL","ZR","START"};
static const int bindValues[]={0,-1,-2,-3,-4,-5,-6,-7,48,49,50,51,52,53,54,55,56,57,42,35,1001,1002};
static const char*bindNames[]={"Profile default","Phone UP","Phone DOWN","Phone LEFT","Phone RIGHT","FIRE / 5","Left softkey","Right softkey","0","1","2","3","4","5","6","7","8","9","*","#","PoP jump (B)","PoP action (A)"};
static int bindingIndex(int value){for(int i=0;i<22;i++)if(bindValues[i]==value)return i;return 0;}
static const int timeouts[]={0,15,30,60,120};
static std::string timeoutLabel(int seconds){return seconds?std::to_string(seconds)+" seconds":"Never";}
static void changeTimeout(GameSettings&o,int delta){int n=0;for(int i=0;i<5;i++)if(timeouts[i]==o.bottomTimeout)n=i;o.bottomTimeout=timeouts[(n+delta+5)%5];}
static void controlScreen(const GameSettings&o,int page,int row){auto s=top();shell(s,page==1?"CONTROLS":"CUSTOM BUTTON BINDINGS");if(page==1){std::string lines[]={"Preset: "+std::string(keys[o.keys]),std::string("Start bottom screen: ")+(o.hideBottom?"Asleep":"Awake"),"Sleep after: "+timeoutLabel(o.bottomTimeout),"Remap buttons...","Reset button bindings","Back"};for(int i=0;i<6;i++){int y=53+i*25;if(i==row)chrome(s,8,y-5,384,23,true);s.text(14,y,lines[i],i==row?0x101c41:0xe0e8ff,46);}}else{int start=row/9*9;for(int i=start;i<15&&i<start+9;i++){int y=60+(i-start)*17;if(i==row)chrome(s,8,y-4,384,17,true);std::string line=i==14?"Back":std::string(physical[i])+": "+bindNames[bindingIndex(o.bindings[i])];s.text(14,y,line,i==row?0x101c41:0xe0e8ff,46);}}s.text(10,222,"Touch wakes screen; L + R opens game menu",0xa1ebff,47);infoBottom("PER-GAME CONTROL SETTINGS","LEFT / RIGHT / A: CHANGE","B: BACK     SAVED AUTOMATICALLY");flush();}
static const char*filterLabels[]={"Off","Soft scanlines","Soft grid","Scanlines","RGB grille","Shadow mask","Color TV"};
static const char*stereoLabels[]={"Off","Models","Layers","Auto"};
static void gameMenu(const GameSettings&o,int row){auto b=bottom();shell(b,"JAVA3DS // GAME MENU");std::string items[]={"Resume game","Display: "+std::string(displayLabels[o.scale]),"Sleep: "+timeoutLabel(o.bottomTimeout),"Stereo 3D: "+std::string(stereoLabels[o.stereo]),"3D strength: "+std::to_string(o.stereoStrength),"CRT: "+std::string(filterLabels[o.filter]),"Sound tone: "+std::string(toneLabels[o.tone]),"End game / Library","Close Java3DS"};for(int i=0;i<9;i++){int y=45+i*20;if(i==row)chrome(b,10,y-4,300,19,true);b.text(17,y,items[i],i==row?0x101c41:0xe0e8ff,36);}b.text(16,224,"3D SLIDER + MODELS / LAYERS",0xa1ebff,36);flush();}
static void loading(u64 now,bool stopping,bool hide){topStereo=false;gfxSet3D(false);auto s=top();shell(s,stopping?"JAVA3DS // RETURNING":"JAVA3DS // LOADING");s.text(200-14*4,81,stopping?"CLOSING GAME...":"LOADING JAVA...",0xc5f9ff);chrome(s,44,111,312,29);s.rect(48,115,304,21,0x14152b);int phase=(now/70)%36;for(int i=0;i<30;i++){int distance=(i-phase+36)%36;if(distance<8)s.rect(51+i*10,118,7,15,blend(0x9c82ef,0x72f5ff,distance,8));}s.text(200-24*4,162,"VIRTUAL MACHINE // READY",0xc6b2f7);if(hide)bottom().rect(0,0,320,240,0);else{shell(bottom(),"JAVA3DS // SYSTEM");bottom().text(32,90,stopping?"RELEASING GAME MEMORY":"INITIALIZING RUNTIME",0x9ef4ff);bottom().text(32,116,"START + SELECT: CANCEL",0xe3c2ff);}flush();}
static void updateScreen(){auto s=top();shell(s,"JAVA3DS // WI-FI UPDATES");s.text(16,51,"Installed version: 0.4.0",0xb7dcff);s.text(16,76,"Channel: Faefolk333/java3ds",0xe3c2ff);std::string m=updaterMessage();if(m.empty())m="A: check for updates";for(size_t i=0;i<m.size();i+=45)s.text(16,106+(i/45)*16,m.substr(i,45),0xffffff,45);int st=updaterState();if(st==3){chrome(s,16,160,368,20);s.rect(19,163,362*updaterPercent()/100,14,0x65edff);}s.text(16,211,st==2?"A: INSTALL    B: LATER":st==3?"PLEASE WAIT...":st==4?"START: EXIT / REOPEN APP":"A: CHECK    B: BACK",0x9ef4ff,46);infoBottom("UPDATES OVER WI-FI","Public test window: no sign-in needed","A: CHECK / INSTALL    B: BACK");flush();}
static void setDisplay(GameInfo&g,GameSettings&o,int mode){configureDisplay(g,o,mode);}
static bool folder(const std::string&p){struct stat st;if(!stat(p.c_str(),&st))return S_ISDIR(st.st_mode);return mkdir(p.c_str(),0777)==0;}
static void startupScreen(const std::string&phase){auto s=top();shell(s,"JAVA3DS // STARTING");s.text(112,80,"LOADING JAVA3DS",0xc5f9ff);chrome(s,44,111,312,29);for(int i=0;i<30;i++)s.rect(51+i*10,118,7,15,blend(0x9c82ef,0x72f5ff,i,30));s.text(16,165,phase,0xe3c2ff,46);shell(bottom(),"JAVA3DS // PLEASE WAIT");bottom().text(24,92,"PREPARING YOUR GAME LIBRARY",0xc5f9ff,34);flush();gspWaitForVBlank();}
static bool prepare(){if(R_FAILED(romfsInit())||chdir("sdmc:/"))return false;for(const char*p:{"/3ds","/3ds/java-3ds","/3ds/java-3ds/runtime","/3ds/java-3ds/cache","/3ds/java-3ds/games","/3ds/java-3ds/settings","/3ds/java-3ds/saves"})if(!folder(p))return false;
 for(const char*name:{"runtime","output"}){std::string current=ROOT+"/"+name+".log",previous=ROOT+"/"+name+".previous.log";struct stat logStat;if(!stat(current.c_str(),&logStat)){remove(previous.c_str());rename(current.c_str(),previous.c_str());}}
 freopen("/3ds/java-3ds/runtime.log","w",stderr);freopen("/3ds/java-3ds/output.log","w",stdout);setvbuf(stderr,nullptr,_IONBF,0);setvbuf(stdout,nullptr,_IONBF,0);fprintf(stderr,"Java3DS 0.4.0 / VM priority 0x34 / Unix epoch waits / HOME gate\n");
 for(const char*n:{"minijvm_rt.jar","midp.jar"}){startupScreen(std::string("Checking runtime: ")+n);char error[160];if(pop_install_file((std::string("romfs:/java/")+n).c_str(),(ROOT+"/runtime/"+n).c_str(),error,sizeof(error))<0){fprintf(stderr,"Runtime install: %s\n",error);return false;}}
 // Keep the independently built CPU test available after a CIA Wi-Fi update.
 // Failure to copy this optional tool does not stop the Java game launcher.
 mkdir("/3ds/java3ds-symbian-probe",0777);
 for(const char*ext:{"3dsx","smdh"}){startupScreen("Preparing Symbian CPU test...");std::string name=std::string("java3ds-symbian-probe.")+ext;char error[160];PopAssetKind kind=!strcmp(ext,"3dsx")?PopAssetKind::ThreeDsx:PopAssetKind::Smdh;if(pop_install_file(("romfs:/probe/"+name).c_str(),("/3ds/java3ds-symbian-probe/"+name).c_str(),error,sizeof(error),kind)<0)fprintf(stderr,"Optional Symbian probe: %s\n",error);}
 // Bundle a small original test application; never overwrite user game files.
 std::string demo=ROOT+"/games/LauncherCheck.jar";struct stat st;if(stat(demo.c_str(),&st)){char error[160];pop_install_file("romfs:/demo/LauncherCheck.jar",demo.c_str(),error,sizeof(error));}
 return true;
}
static int touchControl(int px,int py){if(py>=37&&py<62&&px>=13&&px<307&&(px-13)%100<94){int col=(px-13)/100;return col==0?12:col==2?13:14;}int x=px-13,y=py-72;if(x>=0&&x<294&&y>=0&&y<152&&x%100<94&&y%38<34){int key=y/38*3+x/100;if(key<12)return key;}return -1;}
static unsigned buttons(u32 k,bool allowTouch){unsigned b=0;if(k&(KEY_UP|KEY_CPAD_UP))b|=1;if(k&(KEY_DOWN|KEY_CPAD_DOWN))b|=2;if(k&(KEY_LEFT|KEY_CPAD_LEFT))b|=4;if(k&(KEY_RIGHT|KEY_CPAD_RIGHT))b|=8;if(k&KEY_A)b|=16;if(k&KEY_B)b|=32;if(k&KEY_SELECT)b|=64;if(k&KEY_X)b|=128;if(k&KEY_Y)b|=256;if(k&KEY_R)b|=512;if(k&KEY_L)b|=1024;if(k&KEY_ZL)b|=2048;if(k&KEY_ZR)b|=4096;if(k&KEY_START)b|=8192;if(allowTouch&&(k&KEY_TOUCH)){touchPosition p;hidTouchRead(&p);int key=touchControl(p.px,p.py);if(key>=0&&key<14)b|=1u<<(16+key);}return b;}
static bool lcdReady;static std::atomic<bool>backlightOn{true};static aptHookCookie lcdHook;static LifecycleGate lifecycle;
static void bottomBacklight(bool on){if(backlightOn.exchange(on)==on)return;if(lcdReady){Result r=on?GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_BOTTOM):GSPLCD_PowerOffBacklight(GSPLCD_SCREEN_BOTTOM);if(R_FAILED(r))fprintf(stderr,"Bottom backlight change failed: %08lx\n",(unsigned long)r);}}
static void lcdLifecycle(APT_HookType type,void*){
 bool sleeping;
 switch(type){
 case APTHOOK_ONSUSPEND:sleeping=lifecycle.set(LifecycleGate::APPLET,true);break;
 case APTHOOK_ONRESTORE:sleeping=lifecycle.set(LifecycleGate::APPLET,false);break;
 case APTHOOK_ONSLEEP:sleeping=lifecycle.set(LifecycleGate::SLEEP,true);break;
 case APTHOOK_ONWAKEUP:sleeping=lifecycle.set(LifecycleGate::SLEEP,false);break;
 case APTHOOK_ONEXIT:sleeping=lifecycle.set(LifecycleGate::EXIT,true);break;
 default:return;
 }
 held.store(0);crimson_set_suspended(sleeping);crimson_audio_suspend(sleeping);
}
static void gameFrame(int mode,int stereoMode,int strength,int filter){
 static DisplayFilter phosphors={};display_filter_configure(&phosphors,filter);
 topStereo=stereoMode&&osGet3DSliderState()>.01f;gfxSet3D(topStereo);
 auto s=top();s.rect(0,0,400,240,0);if(topStereo)memset(rightImage,0,sizeof(rightImage));int dw=400,dh=240;
 if(mode!=1){double ratio=std::min(400.0/crimson_width,240.0/crimson_height);if(mode==2&&ratio>1)ratio=1;dw=(int)(crimson_width*ratio);dh=(int)(crimson_height*ratio);}
 int ox=(400-dw)/2,oy=(240-dh)/2;int xs[400],ys[240];for(int x=0;x<dw;x++)xs[x]=x*crimson_width/dw;for(int y=0;y<dh;y++)ys[y]=(y*crimson_height/dh)*crimson_width;
 LightLock_Lock(&frameLock);
 if(topStereo){uint32_t row[400],left[400],right[400];int8_t z[400];int amount=(int)(strength*osGet3DSliderState()*16);for(int y=0;y<dh;y++){for(int x=0;x<dw;x++){int i=ys[y]+xs[x];row[x]=front[i];z[x]=frontDepth[i];}stereo_line(row,z,dw,amount,1,left);stereo_line(row,z,dw,amount,-1,right);for(int x=0;x<dw;x++){int offset=((x+ox)*240+239-y-oy)*3;unsigned a=display_filter_apply(&phosphors,left[x],x+ox,y+oy),b=display_filter_apply(&phosphors,right[x],x+ox,y+oy);topImage[offset]=a;topImage[offset+1]=a>>8;topImage[offset+2]=a>>16;rightImage[offset]=b;rightImage[offset+1]=b>>8;rightImage[offset+2]=b>>16;}}}
 else for(int by=0;by<dh;by+=8)for(int bx=0;bx<dw;bx+=8)for(int x=bx;x<std::min(bx+8,dw);x++)for(int y=by;y<std::min(by+8,dh);y++){
  unsigned c=display_filter_apply(&phosphors,front[ys[y]+xs[x]],x+ox,y+oy);int offset=((x+ox)*240+239-y-oy)*3;topImage[offset]=c;topImage[offset+1]=c>>8;topImage[offset+2]=c>>16;
 }
 LightLock_Unlock(&frameLock);flush();
}
// A stopped VM is never freed while a worker is alive. If a native call cannot
// unwind, end the process through APT; CIA relaunches with fresh VM/DSP state.
[[noreturn]] static void recoverGame(bool closeApplication){
 held.store(0x40000000);crimson_set_suspended(1);crimson_audio_suspend(1);bottomBacklight(true);
 auto s=top();shell(s,"JAVA3DS // RECOVERY");s.text(24,90,closeApplication?"CLOSING APPLICATION...":"RESTARTING JAVA3DS...",0xc5f9ff);flush();gspWaitForVBlank();
 #ifdef JAVA3DS_CIA
 if(!closeApplication)aptSetChainloaderToSelf();else aptClearChainloader();
 #else
 aptClearChainloader();closeApplication=true;
 #endif
 aptExit();if(closeApplication)APT_CloseApplication(nullptr,0,0);svcExitProcess();
 for(;;)svcSleepThread(1000000000);
}
int main(int argc,char**argv){
 gfxInitDefault();gfxSet3D(false);gfxSetDoubleBuffering(GFX_TOP,true);gfxSetDoubleBuffering(GFX_BOTTOM,true);osSetSpeedupEnable(true);LightLock_Init(&frameLock);lcdReady=R_SUCCEEDED(gspLcdInit());
 startupScreen("Mounting SD card...");
 if(!prepare()){auto s=top();s.rect(0,0,400,240,0x111e30);s.text(8,20,"Cannot prepare SD card / runtime.");s.text(8,42,"START: EXIT");flush();while(aptMainLoop()){hidScanInput();if(hidKeysDown()&KEY_START)break;gspWaitForVBlank();}if(lcdReady)gspLcdExit();romfsExit();gfxExit();return 1;}
 aptHook(&lcdHook,lcdLifecycle,nullptr);uiAudioInit();std::string exe=argc>0&&argv&&argv[0]?argv[0]:ROOT+"/java_pocket.3dsx";updaterInit(ROOT,exe);updaterCheck();
 std::string path=ROOT+"/games",error;std::vector<BrowserEntry>entries;int selected=0,row=0,controlPage=0,controlRow=0,menuRow=0;bool detail=false,run=false,redraw=true,updates=false,menu=false,closeAfterGame=false,releaseGameInput=false;GameInfo game;GameSettings options;listGames(path,entries,error);unsigned shown=0,lastLights=~0u;u64 quitAt=0,repeatAt=0,uiAt=0;u32 lastDirection=0;int updateSeen=0;BottomScreenState screen;
 for(;;){
  // Restore the lower backlight before APT takes the display, outside its hook.
  if(aptShouldJumpToHome())bottomBacklight(true);
  if(!aptMainLoop())break;
  if(lifecycle.restored.exchange(false)){held.store(0);shown=~0u;lastLights=~0u;redraw=true;screen.lastTouch=osGetTime();screen.asleep=false;backlightOn.store(false);bottomBacklight(true);}
  aptSetHomeAllowed(updaterState()!=3);
  hidScanInput();sensors();u32 k=hidKeysHeld(),press=hidKeysDown();u64 now=osGetTime();u32 dir=k&(KEY_UP|KEY_DOWN|KEY_LEFT|KEY_RIGHT|KEY_CPAD_UP|KEY_CPAD_DOWN|KEY_CPAD_LEFT|KEY_CPAD_RIGHT);if(dir){if(dir!=lastDirection)repeatAt=now+350;else if(now>=repeatAt){press|=dir;repeatAt=now+85;}}lastDirection=dir;
  if(run){
   int effectiveStereo=osGet3DSliderState()>.01f?options.stereo:0;if(crimson_stereo_mode()!=effectiveStereo&&!crimson_stereo_config(effectiveStereo))options.stereo=0;
   bool touch=(k&KEY_TOUCH)!=0;screen.timeout=options.bottomTimeout;bool wasAsleep=screen.asleep;bool allowTouch=screen.update(now,touch,menu||quitAt);if(wasAsleep!=screen.asleep)lastLights=~0u;bottomBacklight(!screen.asleep);touchPosition tp={};if(touch)hidTouchRead(&tp);bool tap=(press&KEY_TOUCH)&&allowTouch;
   bool menuChord=(k&(KEY_L|KEY_R))==(KEY_L|KEY_R)&&(press&(KEY_L|KEY_R));bool openMenu=!quitAt&&!menu&&((tap&&touchControl(tp.px,tp.py)==14)||menuChord);
   if(openMenu){menu=true;menuRow=0;held.store(0);crimson_pause_game(1);crimson_audio_pause(1);screen.update(now,false,true);bottomBacklight(true);uiAudioPlay(UI_CONFIRM);gameMenu(options,menuRow);}
   else if(menu&&!quitAt){
    bool up=press&KEY_UP,down=press&KEY_DOWN;int delta=(press&KEY_LEFT)?-1:(press&KEY_RIGHT)?1:0;bool activate=press&KEY_A;
    if(up)menuRow=(menuRow+8)%9;if(down)menuRow=(menuRow+1)%9;if(tap&&tp.px>=10&&tp.px<310&&tp.py>=41&&tp.py<221){int hit=(tp.py-41)/20;if(hit<9&&(tp.py-41)%20<19){menuRow=hit;activate=true;}}
    if(up||down||delta)uiAudioPlay(UI_NAVIGATION);if(activate)uiAudioPlay(UI_CONFIRM);
    if((press&KEY_B)||menuChord||(activate&&menuRow==0)){menu=false;releaseGameInput=true;crimson_pause_game(0);crimson_audio_pause(0);lastLights=~0u;screen.lastTouch=now;}
    else if((activate||delta)&&menuRow==1){options.scale=(options.scale+(delta?delta:1)+3)%3;saveSettings(ROOT+"/settings/"+game.id+".cfg",options);gameFrame(options.scale,options.stereo,options.stereoStrength,options.filter);}
    else if((activate||delta)&&menuRow==2){changeTimeout(options,delta?delta:1);saveSettings(ROOT+"/settings/"+game.id+".cfg",options);}
    else if((activate||delta)&&menuRow==3){int next=(options.stereo+(delta?delta:1)+4)%4;if(crimson_stereo_config(osGet3DSliderState()>.01f?next:0))options.stereo=next;saveSettings(ROOT+"/settings/"+game.id+".cfg",options);}
    else if((activate||delta)&&menuRow==4){options.stereoStrength=(options.stereoStrength-1+(delta?delta:1)+6)%6+1;saveSettings(ROOT+"/settings/"+game.id+".cfg",options);}
    else if((activate||delta)&&menuRow==5){options.filter=(options.filter+(delta?delta:1)+DISPLAY_FILTER_COUNT)%DISPLAY_FILTER_COUNT;saveSettings(ROOT+"/settings/"+game.id+".cfg",options);}
    else if((activate||delta)&&menuRow==6){options.tone=(options.tone+(delta?delta:1)+3)%3;crimson_audio_tone(options.tone);saveSettings(ROOT+"/settings/"+game.id+".cfg",options);}
    else if(activate&&menuRow>=7){closeAfterGame=menuRow==8;menu=false;quitAt=now;crimson_pause_game(0);crimson_audio_pause(0);held.store(0x40000000);}
    if(menu)gameMenu(options,menuRow);
   }
   if(menu){crimson_pause_game(1);crimson_audio_pause(1);}
   if((k&(KEY_START|KEY_SELECT))==(KEY_START|KEY_SELECT)&&!quitAt){menu=false;crimson_pause_game(0);crimson_audio_pause(0);quitAt=now;held.store(0x40000000);}else if(!quitAt&&!menu){if(!(k&(KEY_A|KEY_B|KEY_L|KEY_R|KEY_TOUCH|KEY_UP|KEY_DOWN|KEY_LEFT|KEY_RIGHT)))releaseGameInput=false;held.store(openMenu||releaseGameInput?0:buttons(k,allowTouch));}
   if(!quitAt&&!menu&&textRequest.state.load()!=0)pollTextInput();
   if(exitRequested.load()&&!quitAt){menu=false;crimson_pause_game(0);crimson_audio_pause(0);quitAt=now;held.store(0x40000000);}
   if(quitAt)crimson_request_stop();
   static int sliderWas=-1;int sliderNow=(int)(osGet3DSliderState()*100);if(sliderNow!=sliderWas){shown=~0u;sliderWas=sliderNow;}unsigned current=generation.load(),lights=keyLights(held.load(),options);
   if(!menu){if(!current||quitAt){if(now-uiAt>=80){loading(now,quitAt!=0,screen.asleep);uiAt=now;}}
    else if(current!=shown||lights!=lastLights){if(lights!=lastLights)keypad(lights,screen.asleep);gameFrame(options.scale,options.stereo,options.stereoStrength,options.filter);shown=current;lastLights=lights;}}
   if(done.load()){threadJoin(vm,U64_MAX);threadFree(vm);vm=nullptr;textRequest.state.store(0);crimson_stereo_release();crimson_audio_shutdown();run=false;detail=false;menu=false;redraw=true;bottomBacklight(true);error=loadingError.empty()?(result||failed.load()?"Game failed. See output.log / runtime.log":""):loadingError;if(quitAt)error.clear();quitAt=0;held.store(0);if(closeAfterGame)break;}
   else if(quitAt&&now-quitAt>5000)recoverGame(closeAfterGame);
   gspWaitForVBlank();continue;
  }
  topStereo=false;gfxSet3D(false);bottomBacklight(true);if(press&KEY_A)uiAudioPlay(UI_CONFIRM);if(press&(KEY_UP|KEY_DOWN|KEY_LEFT|KEY_RIGHT))uiAudioPlay(UI_NAVIGATION);
  if(now-uiAt>=16){circlePosition c;hidCircleRead(&c);static float px=0,py=0;px+=(c.dx/20.f-px)*.18f;py+=(c.dy/20.f-py)*.18f;parallaxX=(int)(now/80)%20+(int)px;parallaxY=(int)py;uiAt=now;redraw=true;}
  int us=updaterState();if(us!=updateSeen){if(us==2)updates=true;if(us==4)uiAudioPlay(UI_ACHIEVEMENT);updateSeen=us;redraw=true;}
  if(updates){if((press&KEY_START)&&us!=3)break;if((press&KEY_B)&&us!=3&&us!=4){updates=false;redraw=true;}else{if(press&KEY_A){if(us==2){aptSetHomeAllowed(false);updaterInstall();}else if(us!=1&&us!=3&&us!=4)updaterCheck();redraw=true;}if(redraw){updateScreen();redraw=false;}gspWaitForVBlank();continue;}}
  bool up=press&KEY_UP,down=press&KEY_DOWN,left=press&KEY_LEFT,right=press&KEY_RIGHT;
  if(controlPage){int count=controlPage==1?6:15;if(up)controlRow=(controlRow+count-1)%count;if(down)controlRow=(controlRow+1)%count;int delta=left?-1:right?1:0;bool activate=press&KEY_A;
   if((press&KEY_B)||(activate&&controlRow==count-1)){if(controlPage==2){controlPage=1;controlRow=3;}else{controlPage=0;row=2;}}
   else if(controlPage==1){if((delta||activate)&&controlRow==0)options.keys=(options.keys+(delta?delta:1)+(game.pop?3:2))%(game.pop?3:2);if((delta||activate)&&controlRow==1)options.hideBottom=!options.hideBottom;if((delta||activate)&&controlRow==2)changeTimeout(options,delta?delta:1);if(activate&&controlRow==3){controlPage=2;controlRow=0;}else if(activate&&controlRow==4)memset(options.bindings,0,sizeof(options.bindings));}
   else if((delta||activate)&&controlRow<14){int n=game.pop?22:20;options.bindings[controlRow]=bindValues[(bindingIndex(options.bindings[controlRow])+(delta?delta:1)+n)%n];}
   if(press){saveSettings(ROOT+"/settings/"+game.id+".cfg",options);redraw=true;}if(redraw){if(controlPage)controlScreen(options,controlPage,controlRow);else details(game,options,row,error);redraw=false;}gspWaitForVBlank();continue;
  }
  if(!detail){
   if(press&KEY_START)break;if(press&KEY_Y){updates=true;redraw=true;continue;}
   if(up&&!entries.empty()){selected=(selected+(int)entries.size()-1)%entries.size();redraw=true;}if(down&&!entries.empty()){selected=(selected+1)%entries.size();redraw=true;}if((left||right)&&!entries.empty()){selected=std::max(0,std::min((int)entries.size()-1,selected+(right?9:-9)));redraw=true;}
   if(press&KEY_B){path=parentPath(path);selected=0;listGames(path,entries,error);redraw=true;}if(press&KEY_X){error.clear();listGames(path,entries,error);selected=std::min(selected,std::max(0,(int)entries.size()-1));redraw=true;}
   if((press&KEY_A)&&!entries.empty()){if(entries[selected].folder){path=entries[selected].path;selected=0;listGames(path,entries,error);}else{error.clear();if(inspectGame(entries[selected].path,game,error)){options=GameSettings();setDisplay(game,options,game.pop?1:0);if(game.pop)options.keys=2;loadSettings(ROOT+"/settings/"+game.id+".cfg",options);setDisplay(game,options,options.scale);if(options.midlet>=(int)game.midlets.size())options.midlet=0;if(!game.pop&&options.keys==2)options.keys=0;row=0;detail=true;}}redraw=true;}
  }else{
   if(up){row=(row+5)%6;redraw=true;}if(down){row=(row+1)%6;redraw=true;}if(press&KEY_B){detail=false;error.clear();redraw=true;}
   int delta=left?-1:right?1:0;if((press&KEY_A)&&(row==1||row==3||row==4))delta=1;
   if(delta){if(row==4){options.orientation=(options.orientation+delta+3)%3;setDisplay(game,options,options.scale);}if(row==1)setDisplay(game,options,(options.scale+delta+3)%3);if(row==3)options.midlet=(options.midlet+delta+(int)game.midlets.size())%game.midlets.size();saveSettings(ROOT+"/settings/"+game.id+".cfg",options);redraw=true;}
   if((press&KEY_A)&&row==2){controlPage=1;controlRow=0;redraw=true;controlScreen(options,controlPage,controlRow);continue;}
   if((press&KEY_A)&&row==5){detail=false;error.clear();redraw=true;}
   if((press&KEY_A)&&row==0&&!game.unsupported.empty()){error=game.unsupported;redraw=true;}
   else if((press&KEY_A)&&row==0&&updaterState()==3){error="Wait for update installation to finish";redraw=true;}
   else if((press&KEY_A)&&row==0){
    error.clear();activeJar=ROOT+"/cache/active.jar";launchConfig=ROOT+"/cache/launch.cfg";saveSettings(ROOT+"/settings/"+game.id+".cfg",options);loadingGame=game;loadingOptions=options;loadingError.clear();crimson_width=options.width;crimson_height=options.height;if(!crimson_stereo_config(osGet3DSliderState()>.01f?options.stereo:0))options.stereo=0;memset(front,0,sizeof(front));held.store(0);generation.store(0);shown=0;lastLights=~0u;done.store(false);exitRequested.store(false);failed.store(false);popProfile=game.pop&&options.width==400&&options.height==240;menu=false;closeAfterGame=false;screen.start(now,options.bottomTimeout,options.hideBottom);
    textRequest.state.store(0);crimson_reset_stop();loading(now,false,options.hideBottom);vm=threadCreate(worker,nullptr,1024*1024,CRIMSON_VM_PRIORITY,-2,false);if(vm)run=true;else error="Cannot create game thread";redraw=true;
   }
  }
  if(redraw&&!run){if(detail)details(game,options,row,error);else browser(path,entries,selected,error);redraw=false;}gspWaitForVBlank();
 }
 if(vm){held.store(0x40000000);crimson_pause_game(0);crimson_request_stop();if(R_SUCCEEDED(threadJoin(vm,3000000000ULL)))threadFree(vm);else recoverGame(true);crimson_audio_shutdown();}if(!updaterClose())recoverGame(true);uiAudioClose();aptUnhook(&lcdHook);bottomBacklight(true);if(lcdReady)gspLcdExit();if(motionReady.load()&1)HIDUSER_DisableAccelerometer();if(motionReady.load()&2)HIDUSER_DisableGyroscope();romfsExit();gfxExit();return 0;
}
