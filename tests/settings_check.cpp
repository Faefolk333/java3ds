#include "../native/catalog.h"
#include <cassert>
#include <cstdio>
int main(){
 const char*path="work/stereo-settings-test.cfg";GameSettings a,b;a.stereo=3;a.stereoStrength=6;a.bottomTimeout=60;a.filter=6;a.tone=2;a.orientation=1;a.bindings[4]=1002;assert(saveSettings(path,a));assert(loadSettings(path,b));assert(b.filter==6&&b.stereo==3&&b.stereoStrength==6&&b.bottomTimeout==60&&b.bindings[4]==1002&&b.tone==2&&b.orientation==1);
 FILE*f=fopen(path,"w");fprintf(f,"320 240 2 0 0\n0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n30\n");fclose(f);assert(loadSettings(path,b));assert(b.stereo==3&&b.stereoStrength==3&&b.bottomTimeout==30);
 assert(b.tone==1&&b.orientation==0);
 f=fopen(path,"a");fprintf(f,"99 -2\n");fclose(f);assert(loadSettings(path,b));assert(b.stereo==3&&b.stereoStrength==3);a.stereo=0;assert(saveSettings(path,a));assert(loadSettings(path,b)&&b.stereo==0);remove(path);
 GameInfo g;g.nativeWidth=240;g.nativeHeight=320;b=GameSettings();configureDisplay(g,b,0);assert(b.scale==0&&b.width==240&&b.height==320);b.orientation=2;configureDisplay(g,b,0);assert(b.scale==0&&b.width==320&&b.height==240);b.orientation=1;configureDisplay(g,b,2);assert(b.width==240&&b.height==320&&b.scale==2);
 puts("SETTINGS_PASS roundtrip migration orientation fit invalid_values");
}
