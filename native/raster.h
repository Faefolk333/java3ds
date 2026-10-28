#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
extern int crimson_width,crimson_height;
extern uint32_t crimson_pixels[512*512];
void crimson_fill(uint32_t* dst,int dw,int dh,int x,int y,int w,int h,uint32_t color);
void crimson_blit(uint32_t* dst,int dw,int dh,const uint32_t* src,int sw,int sh,int x,int y,int cx,int cy,int cw,int ch);
void crimson_publish(const uint32_t* pixels);
int crimson_buttons(void);
void crimson_music(int id,int loops,int volume);
void crimson_stage(int stage,int detail);
int crimson_gc_diagnostic(void);
int crimson_gc_completed(void);
int crimson_gc_wait_errors(void);
void crimson_request_stop(void);
int crimson_run_game(const char* boot,const char* cp,const char* mainclass);
#ifdef __cplusplus
}
#endif
