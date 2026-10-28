#include "../native/audio/midi_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
static unsigned char* read_file(const char*path,int*size){FILE*f=fopen(path,"rb");assert(f);fseek(f,0,SEEK_END);*size=ftell(f);rewind(f);unsigned char*b=malloc(*size);assert(b);assert(fread(b,1,*size,f)==(size_t)*size);fclose(f);return b;}
static void word(FILE*f,unsigned v,int bytes){for(int i=0;i<bytes;i++)fputc(v>>(i*8),f);}
static void wav(FILE*f,int frames){fwrite("RIFF",1,4,f);word(f,36+frames*2,4);fwrite("WAVEfmt ",1,8,f);word(f,16,4);word(f,1,2);word(f,1,2);word(f,22050,4);word(f,44100,4);word(f,2,2);word(f,16,2);fwrite("data",1,4,f);word(f,frames*2,4);}
int main(int argc,char**argv){
 assert(argc==4);MidiEngine*e=midi_engine_open(argv[1],22050);assert(e);int16_t buffer[256];int total=0;
 const int tracks[]={0,1,2,3,4,10,6,7,8,9,12};
 for(int index=0;index<11;index++){
  int track=tracks[index],size;char path[512];snprintf(path,sizeof(path),"%s/%s-%d.%s",argv[2],index<6?"track":"effect",track,index<6?"mid":"wav");unsigned char*b=read_file(path,&size);
  int id=midi_engine_load(e,b,size);assert(id>=0);free(b);assert(midi_engine_control(e,id,MIDI_STATE,0)==300);
  assert(midi_engine_control(e,id,MIDI_START,1)==0);assert(midi_engine_control(e,id,MIDI_STATE,0)==400);
  unsigned hash=2166136261u;int nonzero=0,frames=0;FILE*out=NULL;
  if(track==4){snprintf(path,sizeof(path),"%s",argv[3]);out=fopen(path,"wb");assert(out);wav(out,0);}
  while(midi_engine_control(e,id,MIDI_STATE,0)==400){midi_engine_render(e,buffer,256);for(int j=0;j<256;j++){nonzero+=buffer[j]!=0;hash=(hash^(unsigned short)buffer[j])*16777619u;}if(out)fwrite(buffer,2,256,out);frames+=256;assert(frames<22050*240);}
  if(out){rewind(out);wav(out,frames);fclose(out);}assert(nonzero>30);
  int duration=midi_engine_control(e,id,MIDI_POSITION,0);assert(duration>0);
  printf("AUDIO_RENDER_PASS %s=%d duration_ms=%d nonzero=%d hash=%08x\n",index<6?"midi":"wav",track,duration,nonzero,hash);
  assert(midi_engine_control(e,id,MIDI_SEEK,0)==0);assert(midi_engine_control(e,id,MIDI_START,2)==0);
  int loopframes=0;while(midi_engine_control(e,id,MIDI_STATE,0)==400){midi_engine_render(e,buffer,256);loopframes+=256;assert(loopframes<=frames*2+256);}assert(abs(loopframes-frames*2)<=256);
  midi_engine_control(e,id,MIDI_SEEK,0);midi_engine_control(e,id,MIDI_START,-1);
  for(int j=0;j<frames/256+5;j++)midi_engine_render(e,buffer,256);
  assert(midi_engine_control(e,id,MIDI_STATE,0)==400);midi_engine_control(e,id,MIDI_STOP,0);assert(midi_engine_control(e,id,MIDI_STATE,0)==300);
  midi_engine_control(e,id,MIDI_VOLUME,0);midi_engine_control(e,id,MIDI_START,1);midi_engine_render(e,buffer,256);for(int j=0;j<256;j++)assert(buffer[j]==0);
  midi_engine_control(e,id,MIDI_CLOSE,0);assert(midi_engine_control(e,id,MIDI_STATE,0)==0);total++;
 }
 assert(midi_engine_load(e,"bad!",4)<0);MidiEngine*copy=midi_engine_clone(e);assert(copy);midi_engine_close(e);
 char path[512];snprintf(path,sizeof(path),"%s/track-0.mid",argv[2]);int size;unsigned char*bytes=read_file(path,&size);int id=midi_engine_load(copy,bytes,size);free(bytes);assert(id>=0);midi_engine_control(copy,id,MIDI_START,1);
 int nonzero=0;for(int i=0;i<40;i++){midi_engine_render(copy,buffer,256);for(int j=0;j<256;j++)nonzero+=buffer[j]!=0;}assert(nonzero>30);midi_engine_close(copy);
 printf("AUDIO_LIFECYCLE_PASS assets=%d shared_SoundFont_lifetime\n",total);return 0;
}
