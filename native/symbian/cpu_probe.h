#pragma once
#include <cstdint>
struct SymbianCpuResult {
 bool arm=false,thumb=false,exclusive=false,vfp=false,context=false,budget=false;
 unsigned faults=0;std::uint64_t benchmarkInstructions=0,benchmarkMicros=0;
 bool passed()const{return arm&&thumb&&exclusive&&vfp&&context&&budget&&!faults;}
};
// Executes only the project's own small guest programs, never a phone ROM.
SymbianCpuResult symbianCpuProbe(bool(*keepRunning)()=nullptr);
