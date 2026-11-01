#include "../native/catalog.h"
#include <cassert>
#include <cstdio>
int main(){
 const char*path="work/stereo-settings-test.cfg";GameSettings a,b;a.stereo=3;a.stereoStrength=6;a.bottomTimeout=60;a.filter=2;a.bindings[4]=1002;assert(saveSettings(path,a));assert(loadSettings(path,b));assert(b.filter==2&&b.stereo==3&&b.stereoStrength==6&&b.bottomTimeout==60&&b.bindings[4]==1002);
 FILE*f=fopen(path,"w");fprintf(f,"320 240 2 0 0\n0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n30\n");fclose(f);assert(loadSettings(path,b));assert(b.stereo==3&&b.stereoStrength==3&&b.bottomTimeout==30);
 f=fopen(path,"a");fprintf(f,"99 -2\n");fclose(f);assert(loadSettings(path,b));assert(b.stereo==3&&b.stereoStrength==3);a.stereo=0;assert(saveSettings(path,a));assert(loadSettings(path,b)&&b.stereo==0);remove(path);puts("SETTINGS_PASS stereo_roundtrip older_settings invalid_values");
}
