#include "../native/audio/midi_engine.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
int main(){MidiEngine*e=midi_engine_open("romfs/audio/gm.sf2",22050);assert(e);int16_t pcm[256];
 for(int program=0;program<128;program++){
  unsigned char m[]={77,84,104,100,0,0,0,6,0,0,0,1,0,96,77,84,114,107,0,0,0,15,0,192,0,0,144,60,100,96,128,60,0,0,255,47,0};
  m[24]=program;int id=midi_engine_load(e,m,sizeof(m));assert(id>=0);assert(!midi_engine_control(e,id,MIDI_START,1));int nonzero=0;for(int k=0;k<80;k++){midi_engine_render(e,pcm,256);for(int i=0;i<256;i++)nonzero+=pcm[i]!=0;}assert(nonzero>10);midi_engine_control(e,id,MIDI_CLOSE,0);
 }
 // Channel 10 percussion, independent from melodic program numbers.
 unsigned char m[]={77,84,104,100,0,0,0,6,0,0,0,1,0,96,77,84,114,107,0,0,0,12,0,153,38,100,96,137,38,0,0,255,47,0};
 int id=midi_engine_load(e,m,sizeof(m));assert(id>=0);midi_engine_control(e,id,MIDI_START,1);int nonzero=0;for(int k=0;k<40;k++){midi_engine_render(e,pcm,256);for(int i=0;i<256;i++)nonzero+=pcm[i]!=0;}assert(nonzero>10);midi_engine_close(e);puts("GM_PASS 128_programs percussion audible_PCM");return 0;}
