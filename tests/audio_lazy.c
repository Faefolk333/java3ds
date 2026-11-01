#include "../native/audio/midi_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
static int load(MidiEngine*e,const char*path){FILE*f=fopen(path,"rb");assert(f);fseek(f,0,SEEK_END);int n=ftell(f);rewind(f);void*b=malloc(n);assert(fread(b,1,n,f)==n);fclose(f);int h=midi_engine_load(e,b,n);free(b);assert(h>=0);return h;}
int main(){MidiEngine*e=midi_engine_open("romfs/audio/gm.sf2",22050),*clone=midi_engine_clone(e);assert(e&&clone&&!midi_engine_bank_loaded(e)&&!midi_engine_bank_loaded(clone));int wave=load(e,"work/audio-fixtures/effect-6.wav");assert(midi_engine_control(e,wave,MIDI_START,-1)==0);int16_t pcm[256];midi_engine_render(e,pcm,256);assert(!midi_engine_bank_loaded(e));int song=load(clone,"work/audio-fixtures/track-1.mid");assert(!midi_engine_bank_loaded(clone));assert(midi_engine_control(clone,song,MIDI_START,-1)==0);assert(midi_engine_bank_loaded(e)&&midi_engine_bank_loaded(clone));midi_engine_close(e);midi_engine_render(clone,pcm,256);assert(midi_engine_control(clone,song,MIDI_STATE,0)==400);midi_engine_close(clone);puts("AUDIO_LAZY_PASS no_bank_for_open_WAV_or_MIDI_parse shared_load_on_MIDI_start parent_close_safe");}
