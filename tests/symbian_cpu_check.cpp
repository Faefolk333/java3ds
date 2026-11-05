#include "../native/symbian/cpu_probe.h"
#include <cstdio>
int main(){auto r=symbianCpuProbe();printf("SYMBIAN_CPU arm=%d thumb=%d exclusive=%d vfp=%d context=%d bounded_slice=%d faults=%u instructions=%llu us=%llu\n",r.arm,r.thumb,r.exclusive,r.vfp,r.context,r.budget,r.faults,(unsigned long long)r.benchmarkInstructions,(unsigned long long)r.benchmarkMicros);if(!r.passed())return 1;puts("SYMBIAN_CPU_PASS real_EKA2L1_dyncom_project_owned_guest_programs_no_phone_OS");}
