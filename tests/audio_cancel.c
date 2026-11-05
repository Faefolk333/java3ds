#include "../native/audio/midi_engine.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static int polls;
static int cancelled(void){return ++polls>=3;}
int main(){
 MidiEngine*e=midi_engine_open("romfs/audio/gm.sf2",22050);assert(e);FILE*f=fopen("work/audio-fixtures/track-1.mid","rb");assert(f);fseek(f,0,SEEK_END);int n=ftell(f);rewind(f);void*b=malloc(n);assert(fread(b,1,n,f)==n);fclose(f);int id=midi_engine_load(e,b,n);free(b);assert(id>=0&&midi_engine_control(e,id,MIDI_START,1)==0);
 midi_engine_set_cancel(e,cancelled);int position=midi_engine_control(e,id,MIDI_SEEK,10000);assert(polls==3&&position<100);
 midi_engine_control(e,id,MIDI_STOP,0);assert(midi_engine_control(e,id,MIDI_START,1)==-1);
 midi_engine_set_cancel(e,NULL);assert(midi_engine_control(e,id,MIDI_START,1)==0);midi_engine_close(e);printf("AUDIO_CANCEL_PASS bounded_seek position_ms=%d polls=%d reject_start recover\n",position,polls);
}
