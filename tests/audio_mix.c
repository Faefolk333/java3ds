#include "../native/audio/midi_engine.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static int load(MidiEngine*e,const char*path){FILE*f=fopen(path,"rb");assert(f);fseek(f,0,SEEK_END);int n=ftell(f);rewind(f);void*b=malloc(n);assert(fread(b,1,n,f)==n);fclose(f);int h=midi_engine_load(e,b,n);free(b);assert(h>=0);return h;}
int main(){MidiEngine*e=midi_engine_open("romfs/audio/gm.sf2",22050);assert(e);int a=load(e,"work/audio-fixtures/track-0.mid"),b=load(e,"work/audio-fixtures/track-1.mid"),w=load(e,"work/audio-fixtures/effect-6.wav");int16_t pcm[256];
 midi_engine_control(e,a,MIDI_START,-1);midi_engine_render(e,pcm,256);int before=midi_engine_control(e,a,MIDI_POSITION,0);midi_engine_control(e,b,MIDI_START,-1);assert(midi_engine_control(e,a,MIDI_STATE,0)==400);assert(midi_engine_control(e,a,MIDI_POSITION,0)==before);midi_engine_control(e,w,MIDI_START,-1);midi_engine_render(e,pcm,256);assert(midi_engine_control(e,a,MIDI_STATE,0)==400&&midi_engine_control(e,b,MIDI_STATE,0)==400&&midi_engine_control(e,w,MIDI_STATE,0)==400);
 before=midi_engine_control(e,a,MIDI_POSITION,0);midi_engine_control(e,b,MIDI_SEEK,500);assert(midi_engine_control(e,a,MIDI_POSITION,0)==before);midi_engine_control(e,b,MIDI_STOP,0);assert(midi_engine_control(e,a,MIDI_STATE,0)==400);midi_engine_control(e,a,MIDI_CLOSE,0);assert(midi_engine_control(e,w,MIDI_STATE,0)==400);midi_engine_render(e,pcm,256);midi_engine_close(e);puts("AUDIO_MIX_PASS 2_MIDI_1_WAV independent_start_stop_seek_close");}
