#pragma once
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
// 1 confirmed, 0 cancelled, -1 game stopped. The launcher owns the applet UI.
int crimson_text_input(const char*label,const char*initial,int maximum,int constraints,char*result,size_t capacity);
#ifdef __cplusplus
}
#endif
