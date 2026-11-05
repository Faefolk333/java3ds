#pragma once
#ifdef __cplusplus
extern "C" {
#endif
int crimson_audio_init(void);
void crimson_audio_shutdown(void);
void crimson_audio_pause(int paused);
void crimson_audio_suspend(int suspended);
void crimson_audio_tone(int mode);
int crimson_audio_load(const void* data,int size);
int crimson_audio_control(int handle,int operation,int value);
void crimson_audio_scene(int state,int level,int enabled);
#ifdef __cplusplus
}
#endif
