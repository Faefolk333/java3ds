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
#include "asset_install.hpp"
#include "audio/device_3ds.h"
#include "font8x8.h"
u32 __stacksize__=1024*1024;
static const std::string ROOT="/3ds/java-3ds";
static LightLock frameLock;static uint32_t front[512*512];static std::atomic<unsigned>held{0},generation{0};
static std::atomic<bool>done{false},exitRequested{false},failed{false};static int result;static Thread vm;
static std::string launchConfig,activeJar;static bool popProfile;
extern "C" const char*crimson_launch_config(){return launchConfig.c_str();}
extern "C" int crimson_buttons(){return held.load();}
extern "C" void crimson_music(int,int,int){}
extern "C" void crimson_gc_wait_report(const char*s){fprintf(stderr,"GC %s\n",s);}
extern "C" void crimson_publish(const uint32_t*p){LightLock_Lock(&frameLock);memcpy(front,p,crimson_width*crimson_height*sizeof(*p));generation.fetch_add(1);LightLock_Unlock(&frameLock);}
extern "C" void crimson_stage(int s,int d){if(s==80)exitRequested.store(true);if(s==90)failed.store(true);if(s==82&&popProfile)crimson_audio_scene((d>>8)&255,d&255,(d>>16)&1);if(s>=40&&s!=60&&s!=61&&s!=63&&s!=64&&s!=82)fprintf(stderr,"VM stage %d %d\n",s,d);}
static GameInfo loadingGame;static GameSettings loadingOptions;static std::string loadingError;
static void worker(void*){if(!prepareGame(loadingGame,loadingOptions,"romfs:/profiles/pop-classic.zip",activeJar,loadingError)||!writeLaunchConfig(loadingGame,loadingOptions,ROOT+"/saves",launchConfig,loadingError)){result=-1;done.store(true);return;}if(held.load()&0x40000000){result=0;done.store(true);return;}crimson_audio_init();std::string cp=ROOT+"/runtime/midp.jar:"+activeJar;result=crimson_run_game((ROOT+"/runtime/minijvm_rt.jar").c_str(),cp.c_str(),"crimson.GameMain");done.store(true);}
struct Surface{
 u8*data;int width;
 void pixel(int x,int y,unsigned c){if(x<0||y<0||x>=width||y>=240)return;int o=(x*240+239-y)*3;data[o]=c;data[o+1]=c>>8;data[o+2]=c>>16;}
 void rect(int x,int y,int w,int h,unsigned c){for(int yy=y;yy<y+h;yy++)for(int xx=x;xx<x+w;xx++)pixel(xx,yy,c);}
 void text(int x,int y,const std::string&s,unsigned color=0xe8eef5,int max=48){int n=0;for(unsigned char c:s){if(n++>=max)break;if(c<32||c>126)c='?';for(int row=0;row<8;row++)for(int col=0;col<8;col++)if(font8x8_basic[c][row]&(1<<col))pixel(x+col,y+row,color);x+=8;}}
};
// All UI painting is off-screen. Keep both screens across buffer swaps.
static u8 topImage[400*240*3],bottomImage[320*240*3];
static Surface top(){return {topImage,400};}static Surface bottom(){return {bottomImage,320};}
static const char*keys[]={"Phone / D-pad","Numeric keypad","PoP: B jump, A action"};
static void flush(){memcpy(gfxGetFramebuffer(GFX_TOP,GFX_LEFT,nullptr,nullptr),topImage,sizeof(topImage));memcpy(gfxGetFramebuffer(GFX_BOTTOM,GFX_LEFT,nullptr,nullptr),bottomImage,sizeof(bottomImage));gfxFlushBuffers();gfxSwapBuffers();gspWaitForVBlank();}
static unsigned blend(unsigned a,unsigned b,int t,int n){unsigned c=0;for(int shift=0;shift<24;shift+=8)c|=(( ((a>>shift)&255)*(n-t)+((b>>shift)&255)*t)/n)<<shift;return c;}
static void chrome(Surface s,int x,int y,int w,int h,bool selected=false){for(int j=0;j<h;j++)s.rect(x,y+j,w,1,blend(selected?0x78edff:0xe8e9ff,selected?0x3651ab:0x626386,j,h));s.rect(x,y,w,1,0xf6ffff);s.rect(x,y,1,h,0xe4f8ff);s.rect(x+w-1,y,1,h,0x25213f);s.rect(x,y+h-1,w,1,0x25213f);}
static int parallaxX=0,parallaxY=0;
static void shell(Surface s,const std::string&title){for(int y=0;y<240;y++)s.rect(0,y,s.width,1,blend(0x21134b,0x071e38,y,240));for(int x=parallaxX-20;x<s.width;x+=20)s.rect(x,33,1,207,0x253558);for(int y=40+parallaxY;y<240;y+=20)s.rect(0,y,s.width,1,0x253558);chrome(s,4,4,s.width-8,26);s.text(12,13,title,0x171b42,(s.width-24)/8);s.rect(8,34,s.width-16,2,0x66e7ff);}
static unsigned keyLights(unsigned raw,const GameSettings&o){
 unsigned lit=raw>>16;const int touch[]={49,50,51,52,53,54,55,56,57,42,48,35};
 const int phone[]={50,56,52,54,53,48,-6,42,35,51,49,55,57,-7};
 for(int i=0;i<14;i++)if(raw&(1u<<i)){int code=o.bindings[i];if(code==0)code=phone[i];if(code==-1)code=50;if(code==-2)code=56;if(code==-3)code=52;if(code==-4)code=54;if(code==-5||code==1002)code=53;
  if((o.keys==2&&o.bindings[i]==0&&i==5)||code==1001)code=(raw&4)?49:(raw&8)?51:50;
  for(int j=0;j<12;j++)if(code==touch[j])lit|=1u<<j;
 }return lit;
}
static void keypad(unsigned lit=0,bool hidden=false){auto b=bottom();if(hidden){b.rect(0,0,320,240,0);return;}shell(b,"JAVA3DS // TOUCH KEYPAD");b.text(8,39,"START + SELECT : LIBRARY",0x9ef4ff,38);const char*labels[]={"1","2 ABC","3 DEF","4 GHI","5 JKL","6 MNO","7 PQRS","8 TUV","9 WXYZ","*","0","#"};for(int i=0;i<12;i++){int x=13+i%3*100,y=54+i/3*40;bool down=lit&(1u<<i);chrome(b,x,y,94,34,down);if(down){b.rect(x,y,94,2,0x18223c);b.rect(x,y,2,34,0x18223c);b.rect(x+2,y+32,92,2,0xc2faff);}else b.rect(x+3,y+3,88,11,0xc5d4f6);b.text(x+47-strlen(labels[i])*4+(down?1:0),y+14+(down?2:0),labels[i],0x151c45,8);}b.text(8,222,"KEY BINDINGS: GAME > CONTROLS",0xa1ebff,38);}
static void infoBottom(const std::string&a,const std::string&b,const std::string&c){auto s=bottom();shell(s,"JAVA3DS // 0.3.0");s.text(10,50,a,0xffffff,37);s.text(10,72,b,0xa1ebff,37);s.text(10,98,c,0xe3c2ff,37);chrome(s,8,134,304,72);s.text(16,146,"A: OPEN / CONFIRM",0x151c45,36);s.text(16,166,"B: BACK    X: REFRESH",0x151c45,36);s.text(16,186,"JAR + JAD // SD LIBRARY",0x151c45,36);s.text(8,222,"JAD NEEDS ITS LOCAL JAR FILE",0xa1ebff,38);}
static void browser(const std::string&folder,const std::vector<BrowserEntry>&entries,int selected,const std::string&error){auto s=top();shell(s,"JAVA3DS // SD LIBRARY");s.text(12,40,folder,0xb7dcff,47);int start=(selected/9)*9;for(int i=start;i<(int)entries.size()&&i<start+9;i++){int y=60+(i-start)*17;if(i==selected)chrome(s,8,y-4,384,17,true);s.text(14,y,(entries[i].folder?"[+] ":" >  ")+entries[i].name,i==selected?0x101c41:0xe0e8ff,46);}if(entries.empty())s.text(12,80,"Add JAR / JAD files to this folder.",0xffffff);s.text(12,222,error.empty()?"Y: WI-FI UPDATES    START: EXIT":error,0xffc7e9,47);infoBottom(entries.empty()?"LIBRARY EMPTY":entries[selected].name,"Select a game or open a folder.","UP/DOWN: SELECT  LEFT/RIGHT: PAGE");flush();}
static void details(const GameInfo&g,const GameSettings&o,int row,const std::string&error){auto s=top();shell(s,g.title);bool wide=o.scale==1;std::string display=wide?(g.pop?"Display: 3DS fullscreen":"Display: 3DS screen (fit)"):"Display: Native "+std::to_string(o.width)+"x"+std::to_string(o.height)+(g.resolutionKnown?"":" (assumed)");std::string lines[]={"Launch game",display,"Controls / key bindings...","MIDlet: "+g.midlets[o.midlet].title,"Back"};for(int i=0;i<5;i++){int y=59+i*28;if(i==row)chrome(s,8,y-5,384,23,true);s.text(14,y,lines[i],i==row?0x101c41:0xe0e8ff,46);}s.text(10,212,error.empty()?(g.unsupported.empty()?(g.pop?"PoP: wider-camera profile available":"Old / New 3DS: same 400 x 240 output"):g.unsupported):error,0xffc7e9,47);infoBottom(g.midlets[o.midlet].main,"LEFT/RIGHT: CHANGE OPTION",g.unsupported.empty()?"Built-in MIDI + PCM WAV audio":"3D backend not implemented yet.");flush();}
static const char*physical[]={"D-pad / stick UP","D-pad / stick DOWN","D-pad / stick LEFT","D-pad / stick RIGHT","A","B","SELECT","X","Y","R","L","ZL","ZR","START"};
static const int bindValues[]={0,-1,-2,-3,-4,-5,-6,-7,48,49,50,51,52,53,54,55,56,57,42,35,1001,1002};
static const char*bindNames[]={"Profile default","Phone UP","Phone DOWN","Phone LEFT","Phone RIGHT","FIRE / 5","Left softkey","Right softkey","0","1","2","3","4","5","6","7","8","9","*","#","PoP jump (B)","PoP action (A)"};
static int bindingIndex(int value){for(int i=0;i<22;i++)if(bindValues[i]==value)return i;return 0;}
static void controlScreen(const GameSettings&o,int page,int row){auto s=top();shell(s,page==1?"CONTROLS":"CUSTOM BUTTON BINDINGS");if(page==1){std::string lines[]={"Preset: "+std::string(keys[o.keys]),std::string("Bottom screen: ")+(o.hideBottom?"Hidden":"Shown"),"Remap buttons...","Reset button bindings","Back"};for(int i=0;i<5;i++){int y=59+i*28;if(i==row)chrome(s,8,y-5,384,23,true);s.text(14,y,lines[i],i==row?0x101c41:0xe0e8ff,46);}}else{int start=row/9*9;for(int i=start;i<15&&i<start+9;i++){int y=60+(i-start)*17;if(i==row)chrome(s,8,y-4,384,17,true);std::string line=i==14?"Back":std::string(physical[i])+": "+bindNames[bindingIndex(o.bindings[i])];s.text(14,y,line,i==row?0x101c41:0xe0e8ff,46);}}s.text(10,222,"START + SELECT always returns to library",0xa1ebff,47);infoBottom("PER-GAME CONTROL SETTINGS","LEFT / RIGHT / A: CHANGE","B: BACK     SAVED AUTOMATICALLY");flush();}
static void loading(u64 now,bool stopping,bool hide){auto s=top();shell(s,stopping?"JAVA3DS // RETURNING":"JAVA3DS // LOADING");s.text(200-14*4,81,stopping?"CLOSING GAME...":"LOADING JAVA...",0xc5f9ff);chrome(s,44,111,312,29);s.rect(48,115,304,21,0x14152b);int phase=(now/70)%36;for(int i=0;i<30;i++){int distance=(i-phase+36)%36;if(distance<8)s.rect(51+i*10,118,7,15,blend(0x9c82ef,0x72f5ff,distance,8));}s.text(200-24*4,162,"VIRTUAL MACHINE // READY",0xc6b2f7);if(hide)bottom().rect(0,0,320,240,0);else{shell(bottom(),"JAVA3DS // SYSTEM");bottom().text(32,90,stopping?"RELEASING GAME MEMORY":"INITIALIZING RUNTIME",0x9ef4ff);bottom().text(32,116,"START + SELECT: CANCEL",0xe3c2ff);}flush();}
static void updateScreen(){auto s=top();shell(s,"JAVA3DS // WI-FI UPDATES");s.text(16,51,"Installed version: 0.3.0",0xb7dcff);s.text(16,76,"Private: Faefolk333/java3ds",0xe3c2ff);std::string m=updaterMessage();if(m.empty())m="A: check for updates";for(size_t i=0;i<m.size();i+=45)s.text(16,106+(i/45)*16,m.substr(i,45),0xffffff,45);int st=updaterState();if(st==3){chrome(s,16,160,368,20);s.rect(19,163,362*updaterPercent()/100,14,0x65edff);}s.text(16,211,st==2?"A: INSTALL    B: LATER":st==3?"PLEASE WAIT...":st==4?"START: EXIT / REOPEN APP":"A: CHECK    B: BACK",0x9ef4ff,46);infoBottom("PRIVATE UPDATES OVER WI-FI","Token file on SD:","/3ds/java-3ds/github-token.txt");flush();}
static void setDisplay(GameInfo&g,GameSettings&o,bool wide){o.scale=wide?1:2;o.width=wide&&g.pop?400:g.nativeWidth;o.height=wide&&g.pop?240:g.nativeHeight;}
static bool folder(const std::string&p){struct stat st;if(!stat(p.c_str(),&st))return S_ISDIR(st.st_mode);return mkdir(p.c_str(),0777)==0;}
static bool prepare(){if(R_FAILED(romfsInit())||chdir("sdmc:/"))return false;for(const char*p:{"/3ds","/3ds/java-3ds","/3ds/java-3ds/runtime","/3ds/java-3ds/cache","/3ds/java-3ds/games","/3ds/java-3ds/settings","/3ds/java-3ds/saves"})if(!folder(p))return false;
 freopen("/3ds/java-3ds/runtime.log","w",stderr);freopen("/3ds/java-3ds/output.log","w",stdout);setvbuf(stderr,nullptr,_IONBF,0);setvbuf(stdout,nullptr,_IONBF,0);
 for(const char*n:{"minijvm_rt.jar","midp.jar"}){char error[160];if(pop_install_file((std::string("romfs:/java/")+n).c_str(),(ROOT+"/runtime/"+n).c_str(),error,sizeof(error))<0){fprintf(stderr,"Runtime install: %s\n",error);return false;}}
 // Bundle a small original test application; never overwrite user game files.
 std::string demo=ROOT+"/games/LauncherCheck.jar";struct stat st;if(stat(demo.c_str(),&st)){char error[160];pop_install_file("romfs:/demo/LauncherCheck.jar",demo.c_str(),error,sizeof(error));}
 return true;
}
static unsigned buttons(u32 k,bool hide){unsigned b=0;if(k&(KEY_UP|KEY_CPAD_UP))b|=1;if(k&(KEY_DOWN|KEY_CPAD_DOWN))b|=2;if(k&(KEY_LEFT|KEY_CPAD_LEFT))b|=4;if(k&(KEY_RIGHT|KEY_CPAD_RIGHT))b|=8;if(k&KEY_A)b|=16;if(k&KEY_B)b|=32;if(k&KEY_SELECT)b|=64;if(k&KEY_X)b|=128;if(k&KEY_Y)b|=256;if(k&KEY_R)b|=512;if(k&KEY_L)b|=1024;if(k&KEY_ZL)b|=2048;if(k&KEY_ZR)b|=4096;if(k&KEY_START)b|=8192;if(!hide&&(k&KEY_TOUCH)){touchPosition p;hidTouchRead(&p);int x=(int)p.px-13,y=(int)p.py-54;if(x>=0&&x<294&&y>=0&&y<160&&x%100<94&&y%40<34){int key=y/40*3+x/100;if(key<12)b|=1u<<(16+key);}}return b;}
static void gameFrame(int mode){auto s=top();s.rect(0,0,400,240,0);int dw=400,dh=240;if(mode!=1||!popProfile){double ratio=std::min(400.0/crimson_width,240.0/crimson_height);if(mode==2&&ratio>1)ratio=1;dw=(int)(crimson_width*ratio);dh=(int)(crimson_height*ratio);}int ox=(400-dw)/2,oy=(240-dh)/2;LightLock_Lock(&frameLock);for(int y=0;y<dh;y++)for(int x=0;x<dw;x++)s.pixel(ox+x,oy+y,front[(y*crimson_height/dh)*crimson_width+x*crimson_width/dw]);LightLock_Unlock(&frameLock);flush();}
int main(int argc,char**argv){
 gfxInitDefault();gfxSet3D(false);gfxSetDoubleBuffering(GFX_TOP,true);gfxSetDoubleBuffering(GFX_BOTTOM,true);osSetSpeedupEnable(true);LightLock_Init(&frameLock);
 if(!prepare()){auto s=top();s.rect(0,0,400,240,0x111e30);s.text(8,20,"Cannot prepare SD card / runtime.");s.text(8,42,"START: EXIT");flush();while(aptMainLoop()){hidScanInput();if(hidKeysDown()&KEY_START)break;gspWaitForVBlank();}romfsExit();gfxExit();return 1;}
 std::string exe=argc>0&&argv&&argv[0]?argv[0]:ROOT+"/java_pocket.3dsx";
 updaterInit(ROOT,exe);struct stat tokenStat;if(!stat((ROOT+"/github-token.txt").c_str(),&tokenStat))updaterCheck();
 std::string path=ROOT+"/games",error;std::vector<BrowserEntry>entries;int selected=0,row=0,controlPage=0,controlRow=0;bool detail=false,run=false,redraw=true,updates=false;GameInfo game;GameSettings options;listGames(path,entries,error);unsigned shown=0,lastLights=~0u;u64 quitAt=0,repeatAt=0,uiAt=0;u32 lastDirection=0;int updateSeen=0;
 while(aptMainLoop()){
  hidScanInput();u32 k=hidKeysHeld(),press=hidKeysDown();u64 now=osGetTime();u32 dir=k&(KEY_UP|KEY_DOWN|KEY_LEFT|KEY_RIGHT|KEY_CPAD_UP|KEY_CPAD_DOWN|KEY_CPAD_LEFT|KEY_CPAD_RIGHT);if(dir){if(dir!=lastDirection)repeatAt=now+350;else if(now>=repeatAt){press|=dir;repeatAt=now+85;}}lastDirection=dir;
  if(run){
   if((k&(KEY_START|KEY_SELECT))==(KEY_START|KEY_SELECT)&&!quitAt){quitAt=now;held.store(0x40000000);}else if(!quitAt)held.store(buttons(k,options.hideBottom));
   if(exitRequested.load()&&!quitAt){quitAt=now;held.store(0x40000000);}
   // Abort stuck Java loops cooperatively; never free a VM while its threads still run.
   if(quitAt&&now-quitAt>750)crimson_request_stop();
   unsigned current=generation.load(),lights=keyLights(held.load(),options);
   if(!current||quitAt){if(now-uiAt>=80){loading(now,quitAt!=0,options.hideBottom);uiAt=now;}}
   else if(current!=shown||lights!=lastLights){keypad(lights,options.hideBottom);gameFrame(options.scale);shown=current;lastLights=lights;}
   if(done.load()){threadJoin(vm,U64_MAX);threadFree(vm);vm=nullptr;crimson_audio_shutdown();run=false;detail=true;redraw=true;error=loadingError.empty()?(result||failed.load()?"Game failed. See output.log / runtime.log":""):loadingError;if(quitAt)error.clear();quitAt=0;held.store(0);}
   else if(quitAt&&now-quitAt>12000){auto s=top();s.text(16,196,"Native call is still stopping.",0xffc7e9);s.text(16,214,"START: CLOSE APPLICATION",0xffc7e9);flush();if(press&KEY_START)svcExitProcess();}
   gspWaitForVBlank();continue;
  }
  if(now-uiAt>=100){circlePosition c;hidCircleRead(&c);parallaxX=(now/300)%20+c.dx/24;parallaxY=c.dy/24;uiAt=now;redraw=true;}
  int us=updaterState();if(us!=updateSeen){if(us==2)updates=true;updateSeen=us;redraw=true;}
  if(updates){if((press&KEY_START)&&us!=3)break;if((press&KEY_B)&&us!=3){updates=false;redraw=true;}else{if(press&KEY_A){if(us==2)updaterInstall();else if(us!=1&&us!=3&&us!=4)updaterCheck();redraw=true;}if(redraw){updateScreen();redraw=false;}gspWaitForVBlank();continue;}}
  bool up=press&(KEY_UP|KEY_CPAD_UP),down=press&(KEY_DOWN|KEY_CPAD_DOWN),left=press&(KEY_LEFT|KEY_CPAD_LEFT),right=press&(KEY_RIGHT|KEY_CPAD_RIGHT);
  if(controlPage){int count=controlPage==1?5:15;if(up)controlRow=(controlRow+count-1)%count;if(down)controlRow=(controlRow+1)%count;int delta=left?-1:right?1:0;bool activate=press&KEY_A;
   if((press&KEY_B)||(activate&&controlRow==count-1)){if(controlPage==2){controlPage=1;controlRow=2;}else{controlPage=0;row=2;}}
   else if(controlPage==1){if((delta||activate)&&controlRow==0)options.keys=(options.keys+(delta?delta:1)+(game.pop?3:2))%(game.pop?3:2);if((delta||activate)&&controlRow==1)options.hideBottom=!options.hideBottom;if(activate&&controlRow==2){controlPage=2;controlRow=0;}else if(activate&&controlRow==3)memset(options.bindings,0,sizeof(options.bindings));}
   else if((delta||activate)&&controlRow<14){int n=game.pop?22:20;options.bindings[controlRow]=bindValues[(bindingIndex(options.bindings[controlRow])+(delta?delta:1)+n)%n];}
   if(press){saveSettings(ROOT+"/settings/"+game.id+".cfg",options);redraw=true;}if(redraw){if(controlPage)controlScreen(options,controlPage,controlRow);else details(game,options,row,error);redraw=false;}gspWaitForVBlank();continue;
  }
  if(!detail){
   if(press&KEY_START)break;if(press&KEY_Y){updates=true;redraw=true;continue;}
   if(up&&!entries.empty()){selected=(selected+(int)entries.size()-1)%entries.size();redraw=true;}if(down&&!entries.empty()){selected=(selected+1)%entries.size();redraw=true;}if((left||right)&&!entries.empty()){selected=std::max(0,std::min((int)entries.size()-1,selected+(right?9:-9)));redraw=true;}
   if(press&KEY_B){path=parentPath(path);selected=0;listGames(path,entries,error);redraw=true;}if(press&KEY_X){error.clear();listGames(path,entries,error);selected=std::min(selected,std::max(0,(int)entries.size()-1));redraw=true;}
   if((press&KEY_A)&&!entries.empty()){if(entries[selected].folder){path=entries[selected].path;selected=0;listGames(path,entries,error);}else{error.clear();if(inspectGame(entries[selected].path,game,error)){options=GameSettings();setDisplay(game,options,game.pop);if(game.pop)options.keys=2;loadSettings(ROOT+"/settings/"+game.id+".cfg",options);bool wide=options.scale==1||(options.scale==0&&options.width==400&&options.height==240);setDisplay(game,options,wide);if(options.midlet>=(int)game.midlets.size())options.midlet=0;if(!game.pop&&options.keys==2)options.keys=0;row=0;detail=true;}}redraw=true;}
  }else{
   if(up){row=(row+4)%5;redraw=true;}if(down){row=(row+1)%5;redraw=true;}if(press&KEY_B){detail=false;error.clear();redraw=true;}
   int delta=left?-1:right?1:0;if((press&KEY_A)&&(row==1||row==3))delta=1;
   if(delta){if(row==1)setDisplay(game,options,options.scale!=1);if(row==3)options.midlet=(options.midlet+delta+(int)game.midlets.size())%game.midlets.size();saveSettings(ROOT+"/settings/"+game.id+".cfg",options);redraw=true;}
   if((press&KEY_A)&&row==2){controlPage=1;controlRow=0;redraw=true;controlScreen(options,controlPage,controlRow);continue;}
   if((press&KEY_A)&&row==4){detail=false;error.clear();redraw=true;}
   if((press&KEY_A)&&row==0&&!game.unsupported.empty()){error=game.unsupported;redraw=true;}
   else if((press&KEY_A)&&row==0){
    error.clear();activeJar=ROOT+"/cache/active.jar";launchConfig=ROOT+"/cache/launch.cfg";saveSettings(ROOT+"/settings/"+game.id+".cfg",options);loadingGame=game;loadingOptions=options;loadingError.clear();crimson_width=options.width;crimson_height=options.height;memset(front,0,sizeof(front));held.store(0);generation.store(0);shown=0;lastLights=~0u;done.store(false);exitRequested.store(false);failed.store(false);popProfile=game.pop&&options.width==400&&options.height==240;
    loading(now,false,options.hideBottom);vm=threadCreate(worker,nullptr,1024*1024,0x30,-2,false);if(vm)run=true;else error="Cannot create game thread";redraw=true;
   }
  }
  if(redraw&&!run){if(detail)details(game,options,row,error);else browser(path,entries,selected,error);redraw=false;}gspWaitForVBlank();
 }
 if(vm){held.store(0x40000000);crimson_request_stop();if(R_SUCCEEDED(threadJoin(vm,3000000000ULL)))threadFree(vm);else svcExitProcess();crimson_audio_shutdown();}updaterClose();romfsExit();gfxExit();return 0;
}
