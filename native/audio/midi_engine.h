#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Portable SF2/MIDI + PCM WAV player. Caller serializes access; no platform or VM dependency.
 * Active MIDI and PCM slots mix independently; engines share a bank loaded on first MIDI start. */
typedef struct MidiEngine MidiEngine;
enum { MIDI_START=1,MIDI_STOP,MIDI_CLOSE,MIDI_SEEK,MIDI_VOLUME,MIDI_STATE,MIDI_POSITION,MIDI_KIND };
MidiEngine* midi_engine_open(const char* soundfont,int sample_rate);
MidiEngine* midi_engine_clone(MidiEngine* source);
int midi_engine_current(MidiEngine* engine);
int midi_engine_bank_loaded(MidiEngine* engine);
void midi_engine_close(MidiEngine* engine);
int midi_engine_load(MidiEngine* engine,const void* bytes,int size);
int midi_engine_control(MidiEngine* engine,int handle,int op,int value);
void midi_engine_render(MidiEngine* engine,int16_t* output,int frames);
#ifdef __cplusplus
}
#endif
