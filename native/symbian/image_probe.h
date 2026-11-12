#pragma once
#include <cstdio>
#include <cstdint>
// Bounded EKA1 runtime diagnostics and ROM AppRun entry. False means no input;
// a true return says a report was produced, never that a game is playable.
bool symbianImageProbe(const char*folder,FILE*report,bool(*keepRunning)()=nullptr,void(*progress)(const char*,std::uint32_t,std::uint32_t)=nullptr,unsigned nativeRepetitions=16);
