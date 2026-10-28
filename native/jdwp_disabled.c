// Offline console build: debugger is deliberately unavailable.
#include "jdwp.h"
s32 jdwp_start_server(MiniJVM *j) { return j->jdwp_enable ? -1 : 0; }
s32 jdwp_stop_server(MiniJVM *j) { (void)j; return 0; }
Runtime *jdwp_get_runtime(JdwpServer *s) { (void)s; return NULL; }
s32 jdwp_is_ignore_sync(JdwpServer *s) { (void)s; return 0; }
s32 jdwp_client_count(JdwpServer *s) { (void)s; return 0; }
s32 jdwp_has_active_stepping(JdwpServer *s) { (void)s; return 0; }
void event_on_breakpoint(JdwpServer *s,Runtime *r) { (void)s;(void)r; }
void event_on_class_prepare(JdwpServer *s,Runtime *r,JClass *c) { (void)s;(void)r;(void)c; }
void event_on_class_unload(JdwpServer *s,JClass *c) { (void)s;(void)c; }
void event_on_thread_death(JdwpServer *s,Instance *t) { (void)s;(void)t; }
void event_on_thread_start(JdwpServer *s,Instance *t) { (void)s;(void)t; }
void event_on_vmstart(JdwpServer *s,Instance *t) { (void)s;(void)t; }
void jdwp_check_breakpoint(Runtime *r) { (void)r; }
void jdwp_check_debug_step(Runtime *r) { (void)r; }
void jdwp_apply_suspend_policy(JdwpServer *s,u8 p,Runtime *r) { (void)s;(void)p;(void)r; }
