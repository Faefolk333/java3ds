#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
// 0 off, 1 model depth, 2 JSR-118 LayerManager depth, 3 both.
int crimson_stereo_config(int mode);
int crimson_stereo_mode(void);
// Call only after the VM worker has stopped.
void crimson_stereo_release(void);
float* crimson_stereo_target(const uint32_t* target);
float crimson_stereo_layer(void);
void crimson_stereo_set_layer(int index,int count);
void crimson_stereo_models_drawn(void);
void crimson_stereo_content_drawn(void);
void crimson_stereo_finish_frame(void);
void crimson_stereo_encode(int8_t* result,int count);
#ifdef __cplusplus
}
#endif
