#include "../native/audio/tone_filter.h"
#include <cassert>
#include <cstdio>
#include <vector>
static double response(int mode,float frequency){
 std::vector<int16_t> pcm(22050);double before=0,after=0;for(int i=0;i<(int)pcm.size();i++){pcm[i]=(int16_t)(12000*sin(6.28318530718*frequency*i/22050));if(i>1024)before+=(double)pcm[i]*pcm[i];}
 AudioToneFilter filter;filter.configure(mode,22050);filter.apply(pcm.data(),pcm.size());for(int i=1025;i<(int)pcm.size();i++)after+=(double)pcm[i]*pcm[i];return sqrt(after/before);
}
int main(){
 assert(fabs(response(0,9000)-1)<1e-8);double bass=response(1,1000),treble=response(1,9000),warm=response(2,9000);assert(bass>.98&&bass<1.02&&treble<.15&&warm<treble);
 AudioToneFilter f;f.configure(1,22050);int16_t pcm[4096]={32767};f.apply(pcm,4096);f.reset();for(auto&v:pcm)v=0;f.apply(pcm,4096);for(auto v:pcm)assert(v==0);
 printf("TONE_FILTER_PASS soft_1k_gain=%.4f soft_9k_gain=%.4f warm_9k_gain=%.4f original_exact reset_silence\n",bass,treble,warm);
}
