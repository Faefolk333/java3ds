#include "audio_device_stub.hpp"
#include "../native/audio/device_3ds.h"
#include "../native/audio/midi_engine.h"
#include <cstdio>
#include <vector>
int audioFailure,audioDspExits,audioLinearFrees;
bool audioNewModel=true,audioDenyCore2=false;int audioLastCore,audioCore2Attempts;
std::atomic<unsigned> audioBuffers{0},audioNonzero{0};
extern "C" int crimson_cancelled(void){return 0;}
int main(){
 audioFailure=1;assert(crimson_audio_init()==-1);assert(crimson_audio_load("MThd",4)<0);crimson_audio_shutdown();assert(audioDspExits==0);
 audioFailure=2;assert(crimson_audio_init()==-3);assert(audioDspExits==1);
 audioFailure=3;assert(crimson_audio_init()==-4);assert(audioDspExits==2&&audioLinearFrees==1);
 audioFailure=0;assert(crimson_audio_init()==0);
 assert(audioLastCore==2&&audioCore2Attempts>0);
 crimson_audio_scene(10,0,1);std::this_thread::sleep_for(std::chrono::milliseconds(80));assert(audioNonzero.load()>100);
 // Music survives an effect; pause and SOUND OFF stop the background layer.
 FILE*f=fopen("work/audio-fixtures/effect-6.wav","rb");assert(f);fseek(f,0,SEEK_END);std::vector<unsigned char> data(ftell(f));rewind(f);assert(fread(data.data(),1,data.size(),f)==data.size());fclose(f);
 int id=crimson_audio_load(data.data(),data.size());assert(id>=0);assert(crimson_audio_control(id,MIDI_START,-1)==0);
 std::this_thread::sleep_for(std::chrono::milliseconds(80));assert(audioBuffers.load()>3&&audioNonzero.load()>100);
 assert(crimson_audio_control(id,MIDI_STATE,0)==400);
 crimson_audio_control(id,MIDI_STOP,0);crimson_audio_scene(15,0,1);
 std::this_thread::sleep_for(std::chrono::milliseconds(10));unsigned silent=audioNonzero.load();std::this_thread::sleep_for(std::chrono::milliseconds(20));assert(silent==audioNonzero.load());
 crimson_audio_scene(10,0,1);std::this_thread::sleep_for(std::chrono::milliseconds(20));assert(audioNonzero.load()>silent);
 crimson_audio_scene(10,0,0);std::this_thread::sleep_for(std::chrono::milliseconds(10));silent=audioNonzero.load();std::this_thread::sleep_for(std::chrono::milliseconds(20));assert(silent==audioNonzero.load());
 crimson_audio_control(id,MIDI_CLOSE,0);f=fopen("work/audio-fixtures/track-1.mid","rb");assert(f);fseek(f,0,SEEK_END);data.resize(ftell(f));rewind(f);assert(fread(data.data(),1,data.size(),f)==data.size());fclose(f);
 id=crimson_audio_load(data.data(),data.size());assert(id>=0);assert(crimson_audio_control(id,MIDI_START,1)==0);
 std::this_thread::sleep_for(std::chrono::milliseconds(40));assert(audioNonzero.load()>silent);
 crimson_audio_pause(1);std::this_thread::sleep_for(std::chrono::milliseconds(4));int frozenPosition=crimson_audio_control(id,MIDI_POSITION,0);std::this_thread::sleep_for(std::chrono::milliseconds(10));assert(crimson_audio_control(id,MIDI_POSITION,0)==frozenPosition);crimson_audio_pause(0);
 crimson_audio_shutdown();assert(audioDspExits==3&&audioLinearFrees==2);
 unsigned old=audioBuffers.load();std::this_thread::sleep_for(std::chrono::milliseconds(10));assert(old==audioBuffers.load());
 audioFailure=4;assert(crimson_audio_init()==0);std::this_thread::sleep_for(std::chrono::milliseconds(20));crimson_audio_shutdown();assert(audioDspExits==4&&audioLinearFrees==3);
 audioFailure=0;audioDenyCore2=true;assert(crimson_audio_init()==0&&audioLastCore==-2);crimson_audio_shutdown();
 int attempts=audioCore2Attempts;audioNewModel=false;assert(crimson_audio_init()==0&&audioLastCore==-2&&audioCore2Attempts==attempts);crimson_audio_shutdown();
 printf("AUDIO_DEVICE_PASS MIDI_background WAV_overlay pause resume SOUND_OFF cache_failure DSP_failure allocation_failure thread_failure clean_shutdown\n");
}
