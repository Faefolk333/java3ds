#pragma once
#include <atomic>
// HOME/applet suspension and lid sleep can nest. Waking the lid while HOME is
// still open must not restart VM/DSP work. Hooks only update atomic state.
struct LifecycleGate {
 enum {SLEEP=1,APPLET=2,EXIT=4};
 std::atomic<unsigned> reasons{0};
 std::atomic<bool> restored{false};
 bool set(unsigned reason,bool active){unsigned before=active?reasons.fetch_or(reason):reasons.fetch_and(~reason);unsigned after=active?before|reason:before&~reason;if(before&&!after)restored.store(true);return after!=0;}
 bool suspended()const{return reasons.load()!=0;}
};
