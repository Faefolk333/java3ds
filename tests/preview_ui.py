from pathlib import Path
r=Path(__file__).resolve().parents[1]
s=(r/'native/main.cpp').read_text();s=s[s.index('struct Surface{'):s.index('static bool folder(')]
a=s.index('static void flush()');b=s.index('static unsigned blend',a);s=s[:a]+'static void flush(){}\n'+s[b:]
prefix='#include <cstdio>\n#include <cstring>\n#include "../native/catalog.h"\n#include "../native/font8x8.h"\ntypedef unsigned char u8;typedef unsigned long long u64;\nstatic int updaterState(){return 2;}static int updaterPercent(){return 65;}static std::string updaterMessage(){return "Update v0.4.0 available - A: install";}\n'
tail='''
void save(const char*path,u8*data,int w){FILE*f=fopen(path,"wb");fprintf(f,"P6\\n%d 240\\n255\\n",w);for(int y=0;y<240;y++)for(int x=0;x<w;x++){int o=(x*240+239-y)*3;unsigned char p[]={data[o+2],data[o+1],data[o]};fwrite(p,1,3,f);}fclose(f);}
int main(){std::vector<BrowserEntry>e={{"Favorites","",true},{"Prince of Persia Classic.jar","",false},{"Blades and Magic [240x320].jar","",false},{"Launcher Check.jar","",false}};browser("/3ds/java-3ds/games",e,1,"");save("work/ui-library.ppm",topImage,400);save("work/ui-info.ppm",bottomImage,320);keypad((1<<4)|(1<<12));save("work/ui-keypad.ppm",bottomImage,320);GameInfo g;g.title="Prince of Persia Classic";g.pop=true;g.midlets.push_back({"Prince of Persia","GloftPOPC"});GameSettings o;o.width=400;o.keys=2;o.scale=1;details(g,o,1,"");save("work/ui-settings.ppm",topImage,400);controlScreen(o,2,4);save("work/ui-controls.ppm",topImage,400);loading(1200,false,false);save("work/ui-loading.ppm",topImage,400);updateScreen();save("work/ui-updates.ppm",topImage,400);gameMenu(o,2);save("work/ui-game-menu.ppm",bottomImage,320);}
'''
(r/'work/ui-preview.cpp').write_text(prefix+s+tail)
