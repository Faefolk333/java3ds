#include "midi_engine.h"
#include <stdlib.h>
#include <string.h>
#define TSF_IMPLEMENTATION
#include "../../third-party/TinySoundFont/tsf.h"
#define TML_IMPLEMENTATION
#include "../../third-party/TinySoundFont/tml.h"
#define SLOTS 32
typedef struct { tsf*synth;tml_message *messages,*cursor;uint8_t *pcm;int pcm_bytes,pcm_rate,pcm_bits,pcm_channels;uint64_t frame,duration;int loops,volume,loaded,playing; } Sequence;
struct MidiEngine {tsf* synth;Sequence slots[SLOTS];int rate,active;};
static unsigned le16(const uint8_t*p){return p[0]|p[1]<<8;}
static uint32_t le32(const uint8_t*p){return (uint32_t)le16(p)|(uint32_t)le16(p+2)<<16;}
static void reset_voice(tsf*synth){
 if(!synth)return;tsf_reset(synth);
 for(int c=0;c<16;c++){
  tsf_channel_sounds_off_all(synth,c);tsf_channel_midi_control(synth,c,121,0);
  tsf_channel_set_pitchwheel(synth,c,8192);tsf_channel_set_bank_preset(synth,c,c==9?128:0,0);
 }
}
static void message(tsf*synth,const tml_message*m){
 switch(m->type){
 case TML_PROGRAM_CHANGE:tsf_channel_set_presetnumber(synth,m->channel,m->program,m->channel==9);break;
 case TML_NOTE_ON:if(m->velocity)tsf_channel_note_on(synth,m->channel,m->key,m->velocity/127.f);else tsf_channel_note_off(synth,m->channel,m->key);break;
 case TML_NOTE_OFF:tsf_channel_note_off(synth,m->channel,m->key);break;
 case TML_PITCH_BEND:tsf_channel_set_pitchwheel(synth,m->channel,m->pitch_bend);break;
 case TML_CONTROL_CHANGE:tsf_channel_midi_control(synth,m->channel,m->control,m->control_value);break;
 }
}
MidiEngine* midi_engine_open(const char*path,int rate){
 if(rate<8000||rate>48000)return NULL;
 MidiEngine*e=calloc(1,sizeof(*e));if(!e)return NULL;e->active=-1;e->rate=rate;
 e->synth=tsf_load_filename(path);if(!e->synth){free(e);return NULL;}
 tsf_set_output(e->synth,TSF_MONO,rate,-9.f);
 if(!tsf_set_max_voices(e->synth,32)){midi_engine_close(e);return NULL;}
 reset_voice(e->synth);return e;
}
void midi_engine_close(MidiEngine*e){if(!e)return;for(int i=0;i<SLOTS;i++){if(e->slots[i].messages)tml_free(e->slots[i].messages);free(e->slots[i].pcm);if(e->slots[i].synth)tsf_close(e->slots[i].synth);}tsf_close(e->synth);free(e);}
MidiEngine* midi_engine_clone(MidiEngine*source){
 if(!source)return NULL;MidiEngine*e=calloc(1,sizeof(*e));if(!e)return NULL;
 e->active=-1;e->rate=source->rate;e->synth=tsf_copy(source->synth);
 if(!e->synth){free(e);return NULL;}tsf_set_output(e->synth,TSF_MONO,e->rate,-9.f);
 if(!tsf_set_max_voices(e->synth,32)){midi_engine_close(e);return NULL;}reset_voice(e->synth);return e;
}
int midi_engine_current(MidiEngine*e){if(!e)return -1;if(e->active>=0&&e->slots[e->active].playing)return e->active;for(int i=0;i<SLOTS;i++)if(e->slots[i].playing)return i;return -1;}
static int load_wav(Sequence*s,const uint8_t*b,int size,int rate){
 if(size<44||memcmp(b,"RIFF",4)||memcmp(b+8,"WAVE",4))return 0;
 int format=0;const uint8_t*pcm=NULL;unsigned bytes=0;
 for(unsigned p=12;p+8<=(unsigned)size;){
  unsigned n=le32(b+p+4);const uint8_t*d=b+p+8;if(n>(unsigned)size-p-8)return 0;
  if(!memcmp(b+p,"fmt ",4)){if(n<16||le16(d)!=1)return 0;s->pcm_channels=le16(d+2);s->pcm_rate=le32(d+4);s->pcm_bits=le16(d+14);format=1;}
  if(!memcmp(b+p,"data",4)){pcm=d;bytes=n;}
  p+=8+n+(n&1);
 }
 if(!format||!pcm||!bytes||s->pcm_channels<1||s->pcm_channels>2||s->pcm_rate<4000||s->pcm_rate>96000||(s->pcm_bits!=8&&s->pcm_bits!=16))return 0;
 unsigned stride=s->pcm_channels*(s->pcm_bits/8);if(bytes%stride)return 0;
 s->pcm=malloc(bytes);if(!s->pcm)return 0;memcpy(s->pcm,pcm,bytes);s->pcm_bytes=bytes;
 s->duration=((uint64_t)(bytes/stride)*rate+s->pcm_rate-1)/s->pcm_rate;return 1;
}
int midi_engine_load(MidiEngine*e,const void*data,int size){
 if(!e||!data||size<4||size>1024*1024)return -1;int i;for(i=0;i<SLOTS&&e->slots[i].loaded;i++);if(i==SLOTS)return -1;
 Sequence*s=&e->slots[i];memset(s,0,sizeof(*s));s->volume=100;s->loops=1;
 if(!memcmp(data,"MThd",4)){
  s->messages=tml_load_memory(data,size);if(!s->messages)return -1;
  unsigned length=0;tml_get_info(s->messages,NULL,NULL,NULL,NULL,&length);
  s->duration=(uint64_t)(length+250)*e->rate/1000; // Preserve a short release tail after the final event.
 }else if(!load_wav(s,data,size,e->rate))return -1;
 s->cursor=s->messages;s->loaded=1;return i;
}
static void render_sequence(MidiEngine*e,Sequence*s,int16_t*out,int frames);
static void seek(MidiEngine*e,Sequence*s,uint64_t frame){
 reset_voice(s->synth);s->cursor=s->messages;s->frame=0;
 // Replay controllers and notes through the renderer so resuming a stopped MIDI
 // preserves its phase instead of restarting all held notes at the seek point.
 int16_t discard[256];int active=e->active,playing=s->playing,loops=s->loops;
 e->active=(int)(s-e->slots);s->playing=1;s->loops=1;
 while(s->frame<frame&&s->playing){int n=(int)((frame-s->frame)>256?256:frame-s->frame);render_sequence(e,s,discard,n);}
 e->active=active;s->playing=playing;s->loops=loops;
}
int midi_engine_control(MidiEngine*e,int id,int op,int value){
 if(!e||id<0||id>=SLOTS||!e->slots[id].loaded)return op==MIDI_STATE?0:-1;
 Sequence*s=&e->slots[id];
 switch(op){
 case MIDI_START:
  if(!value||value < -1)return -1;
  if(e->active==id&&s->playing)return 0;
  if(s->messages&&!s->synth){s->synth=tsf_copy(e->synth);if(!s->synth)return -1;tsf_set_output(s->synth,TSF_MONO,e->rate,-9.f);if(!tsf_set_max_voices(s->synth,24)){tsf_close(s->synth);s->synth=NULL;return -1;}}
  e->active=id;if(s->frame>=s->duration)s->frame=0;s->loops=value;seek(e,s,s->frame);s->playing=1;return 0;
 case MIDI_STOP:s->playing=0;reset_voice(s->synth);return 0;
 case MIDI_CLOSE:
  if(e->active==id)e->active=-1;if(s->synth)tsf_close(s->synth);if(s->messages)tml_free(s->messages);free(s->pcm);memset(s,0,sizeof(*s));return 0;
 case MIDI_SEEK:{if(value<0)value=0;uint64_t frame=(uint64_t)value*e->rate/1000;if(frame>s->duration)frame=s->duration;
  if(s->playing)seek(e,s,frame);else{s->frame=frame;s->cursor=s->messages;}return (int)(s->frame*1000/e->rate);}
 case MIDI_VOLUME:s->volume=value<0?0:value>100?100:value;return s->volume;
 case MIDI_STATE:return s->playing?400:300;
 case MIDI_POSITION:return (int)(s->frame*1000/e->rate);
 case MIDI_KIND:return s->messages?1:2;
 default:return -1;
 }
}
static int pcm_sample(const Sequence*s,unsigned frame,int channel){
 unsigned i=frame*s->pcm_channels+channel;if(s->pcm_bits==8)return ((int)s->pcm[i]-128)*256;
 unsigned v=le16(s->pcm+i*2);return v<32768?(int)v:(int)v-65536;
}
static void render_sequence(MidiEngine*e,Sequence*s,int16_t*out,int frames){
 if(frames<0)return;memset(out,0,(size_t)frames*sizeof(*out));
 if(!s->playing)return;
 int done=0;if(s->synth)tsf_set_volume(s->synth,s->volume*0.0035f);
 while(done<frames&&s->playing){
  if(s->frame>=s->duration){
   if(s->loops==-1||s->loops>1){if(s->loops>1)s->loops--;s->frame=0;s->cursor=s->messages;reset_voice(s->synth);}
   else{s->playing=0;reset_voice(s->synth);break;}
  }
  int n=frames-done;if(n>256)n=256;if((uint64_t)n>s->duration-s->frame)n=(int)(s->duration-s->frame);
  if(s->messages){
   while(s->cursor&&(uint64_t)s->cursor->time*e->rate/1000<=s->frame){message(s->synth,s->cursor);s->cursor=s->cursor->next;}
   if(s->cursor){uint64_t next=(uint64_t)s->cursor->time*e->rate/1000;if(next>s->frame&&next-s->frame<(uint64_t)n)n=(int)(next-s->frame);}
   tsf_render_short(s->synth,out+done,n,0);
  }else{
   unsigned source_frames=s->pcm_bytes/(s->pcm_channels*(s->pcm_bits/8));
   for(int j=0;j<n;j++){uint64_t pos=(s->frame+j)*s->pcm_rate;unsigned f=pos/e->rate;unsigned next=f+1<source_frames?f+1:f;if(f>=source_frames)continue;
    int a=0,b=0;for(int c=0;c<s->pcm_channels;c++){a+=pcm_sample(s,f,c);b+=pcm_sample(s,next,c);}a/=s->pcm_channels;b/=s->pcm_channels;
    out[done+j]=(int16_t)((a+(int64_t)(b-a)*(pos%e->rate)/e->rate)*s->volume/100);
   }
  }
  done+=n;s->frame+=n;
 }
}

void midi_engine_render(MidiEngine*e,int16_t*out,int frames){
 if(frames<0)return;memset(out,0,(size_t)frames*sizeof(*out));if(!e)return;
 int16_t scratch[256];for(int i=0;i<SLOTS;i++)if(e->slots[i].playing){for(int offset=0;offset<frames;offset+=256){int n=frames-offset;if(n>256)n=256;render_sequence(e,&e->slots[i],scratch,n);for(int j=0;j<n;j++){int sum=out[offset+j]+scratch[j];out[offset+j]=(int16_t)(sum<-32768?-32768:sum>32767?32767:sum);}}}
}
