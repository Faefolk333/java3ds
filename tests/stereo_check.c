#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../native/raster.h"
#include "../native/stereo.h"
#include "../native/stereo_display.h"
int main(void){
 int8_t z[16];uint32_t src[16],a[16],b[16];for(int i=0;i<16;i++)src[i]=0xff000000+i;
 assert(crimson_stereo_mode()==0&&!crimson_stereo_target(crimson_pixels));assert(crimson_stereo_config(3));
 crimson_stereo_set_layer(1,2);crimson_fill(NULL,16,1,0,0,16,1,0xff000011);crimson_stereo_set_layer(0,2);crimson_fill(NULL,16,1,6,0,4,1,0xff22ff33);crimson_stereo_set_layer(-1,0);crimson_fill(NULL,16,1,0,0,2,1,0xffffffff);
 crimson_stereo_encode(z,16);assert(z[0]==-128&&z[3]<0&&z[7]>0);
 memset(z,0,sizeof(z));stereo_line(src,z,16,6*16,1,a);stereo_line(src,z,16,6*16,-1,b);assert(!memcmp(src,a,sizeof(src))&&!memcmp(src,b,sizeof(src)));
 for(int i=0;i<16;i++)z[i]=127;stereo_line(src,z,16,3*16,1,a);stereo_line(src,z,16,3*16,-1,b);assert(a[8]==src[5]&&b[8]==src[11]);for(int i=0;i<16;i++)assert(a[i]&&b[i]);
 z[8]=-128;stereo_line(src,z,16,3*16,1,a);assert(a[8]==src[8]);memset(z,0,sizeof(z));z[5]=-127;stereo_line(src,z,16,3*16,1,a);assert(a[2]==src[5]);
 float*d=crimson_stereo_target(crimson_pixels);for(int i=0;i<16;i++)d[i]=1.f+i*.1f;crimson_stereo_encode(z,16);assert(z[0]<0&&z[15]>0);
 assert(crimson_stereo_config(2));crimson_stereo_encode(z,16);for(int i=0;i<16;i++)assert(!z[i]);assert(crimson_stereo_config(0));assert(!crimson_stereo_target(crimson_pixels));
 crimson_stereo_release();assert(crimson_stereo_mode()==0&&!crimson_stereo_target(crimson_pixels));
 puts("STEREO_PASS flat_identity eye_direction gap_fill layer_depth model_depth HUD_priority mode_selection release");
}
