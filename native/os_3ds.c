// Crimson platform adapter. No process launch or dynamic native libraries.
#include "jvm.h"
#include "jvm_util.h"
#include <unistd.h>
#include <sys/stat.h>
// newlib declares this C11 helper but this toolchain does not provide it.
int timespec_get(struct timespec *ts,int base) {
    if(base!=TIME_UTC || !ts) return 0;
    return clock_gettime(CLOCK_REALTIME,ts)==0 ? TIME_UTC : 0;
}
void os_setup_crash_handler(void) {}
s32 os_execute(Runtime *r, Instance *a, Instance *b, ArrayList *c, const c8 *cmd) {
    (void)a; (void)b; (void)c; (void)cmd;
    exception_throw(JVM_EXCEPTION_IO, r, NULL);
    return RUNTIME_STATUS_EXCEPTION;
}
s32 os_kill_process(s64 pid) { (void)pid; return -1; }
s32 os_waitfor_process(Runtime *r, s64 pid, s64 tid, s32 *code) {
    (void)r; (void)pid; (void)tid; if(code) *code=-1; return -1;
}
Utf8String *os_get_tmp_dir(void) { return utf8_create_c("sdmc:/3ds/pop-classic/tmp"); }
void os_set_file_length(FILE *f, s64 n) { ftruncate(fileno(f),(off_t)n); }
s32 os_mkdir(const c8 *p) { return mkdir(p,0777); }
s32 os_iswin(void) { return 0; }
s32 os_fileno(FILE *f) { return fileno(f); }
s32 os_append_libname(Utf8String *s, const c8 *n) { (void)s; (void)n; return -1; }
s32 os_load_lib_and_init(const c8 *n, Runtime *r) { (void)n; (void)r; return 0; }
void os_get_lang(Utf8String *s) { utf8_append_c(s,"en"); }
void os_get_uuid(MiniJVM *j, Utf8String *s) { (void)j; utf8_clear(s); utf8_append_c(s,"pop-local-3ds"); }
s32 conv_utf8_2_platform_encoding(ByteBuf *d, Utf8String *s) {
    if(!d || !s) return -1;
    bytebuf_expand(d,s->length+1); memcpy(d->buf,s->data,s->length); d->buf[s->length]=0; return s->length;
}
s32 conv_platform_encoding_2_utf8(Utf8String *d, const c8 *s) {
    if(!d || !s) return -1;
    utf8_clear(d); utf8_append_c(d,s); return d->length;
}
