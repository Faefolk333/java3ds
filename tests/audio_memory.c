#include "../native/audio/midi_engine.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
extern void*__real_malloc(size_t);extern void*__real_calloc(size_t,size_t);extern void*__real_realloc(void*,size_t);extern void __real_free(void*);
static struct {void*p;size_t n;} blocks[8192];static size_t live,peak,largest;
static void removeBlock(void*p){for(int i=0;i<8192;i++)if(blocks[i].p==p){live-=blocks[i].n;blocks[i].p=NULL;return;}}
static void addBlock(void*p,size_t n){if(!p)return;for(int i=0;i<8192;i++)if(!blocks[i].p){blocks[i].p=p;blocks[i].n=n;live+=n;if(live>peak)peak=live;if(n>largest)largest=n;return;}assert(0);}
void*__wrap_malloc(size_t n){void*p=__real_malloc(n);addBlock(p,n);return p;}void*__wrap_calloc(size_t n,size_t z){void*p=__real_calloc(n,z);addBlock(p,n*z);return p;}void __wrap_free(void*p){if(p)removeBlock(p);__real_free(p);}void*__wrap_realloc(void*p,size_t n){void*q=__real_realloc(p,n);if(q){if(p)removeBlock(p);addBlock(q,n);}else if(!n&&p)removeBlock(p);return q;}
int main(){size_t startup=live;MidiEngine*a=midi_engine_open("romfs/audio/gm.sf2",22050);MidiEngine*b=midi_engine_clone(a);assert(a&&b&&!midi_engine_bank_loaded(a)&&live<128*1024);size_t idle=live;FILE*f=fopen("work/audio-fixtures/track-1.mid","rb");assert(f);unsigned char data[16384];int n=fread(data,1,sizeof(data),f);fclose(f);int id=midi_engine_load(a,data,n);assert(id>=0&&!midi_engine_bank_loaded(a));assert(midi_engine_control(a,id,MIDI_START,-1)==0);assert(largest<20*1024*1024);size_t one=live;int id2=midi_engine_load(b,data,n);assert(id2>=0&&midi_engine_control(b,id2,MIDI_START,-1)==0&&live<one+1024*1024);midi_engine_close(a);midi_engine_close(b);if(live!=startup){for(int i=0;i<8192;i++)if(blocks[i].p)fprintf(stderr,"LIVE_BLOCK %zu\n",blocks[i].n);}assert(live==startup);printf("AUDIO_MEMORY_PASS idle=%llu largest=%llu peak=%llu shared_bank no_growth_from_process_start\n",(unsigned long long)idle,(unsigned long long)largest,(unsigned long long)peak);}
