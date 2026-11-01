#include "../native/audio/midi_engine.h"
#include "../third-party/opencore-amr/amrnb/interf_dec.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc,char**argv){
 assert(argc>1);MidiEngine*e=midi_engine_open("romfs/audio/gm.sf2",8000);assert(e);
 static const int lengths[]={13,14,16,18,20,21,27,32,6,0,0,0,0,0,0,1};
 for(int a=1;a<argc;a++){
  FILE*f=fopen(argv[a],"rb");assert(f);fseek(f,0,SEEK_END);int size=ftell(f);rewind(f);unsigned char*b=malloc(size);assert(b&&fread(b,1,size,f)==(size_t)size);fclose(f);
  int frames=0;for(int p=6;p<size;p+=lengths[(b[p]>>3)&15])frames++;
  int16_t*expected=malloc(frames*320);assert(expected);void*d=Decoder_Interface_init();assert(d);int frame=0;
  for(int p=6;p<size;p+=lengths[(b[p]>>3)&15])Decoder_Interface_Decode(d,b+p,expected+160*frame++,!(b[p]&4));Decoder_Interface_exit(d);
  int h=midi_engine_load(e,b,size);assert(h>=0&&!midi_engine_bank_loaded(e));
  assert(midi_engine_control(e,h,MIDI_DURATION,0)==frames*20);assert(!midi_engine_control(e,h,MIDI_START,1));
  int16_t actual[160];int nonzero=0;
  for(int n=0;n<frames;n++){midi_engine_render(e,actual,160);for(int i=0;i<160;i++){if(actual[i]!=expected[n*160+i]){fprintf(stderr,"AMR mismatch %s frame=%d sample=%d got=%d expected=%d\n",argv[a],n,i,actual[i],expected[n*160+i]);abort();}nonzero+=actual[i]!=0;}}
  free(expected);assert(nonzero>0);midi_engine_render(e,actual,160);assert(midi_engine_control(e,h,MIDI_STATE,0)==300);midi_engine_control(e,h,MIDI_CLOSE,0);
  assert(midi_engine_load(e,b,size-1)<0);free(b);
 }
 unsigned char bad[]={ '#','!','A','M','R','\n',0x4c };assert(midi_engine_load(e,bad,sizeof(bad))<0);bad[6]=0x80;assert(midi_engine_load(e,bad,sizeof(bad))<0);
 midi_engine_close(e);printf("AUDIO_AMR_PASS clips=%d bit_exact_PCM duration EOF truncated reserved invalid_header no_SoundFont\n",argc-1);
}
