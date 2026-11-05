#include "../native/lifecycle_gate.h"
#include <cassert>
#include <cstdio>
int main(){LifecycleGate gate;assert(!gate.suspended());assert(gate.set(gate.APPLET,true));assert(gate.set(gate.SLEEP,true));assert(gate.set(gate.SLEEP,false));assert(!gate.restored.exchange(false));assert(!gate.set(gate.APPLET,false));assert(gate.restored.exchange(false));assert(!gate.restored.exchange(false));assert(gate.set(gate.EXIT,true));assert(gate.set(gate.SLEEP,false));puts("LIFECYCLE_GATE_PASS nested_HOME_lid_wakeup restore_once exit_keeps_suspended");}
