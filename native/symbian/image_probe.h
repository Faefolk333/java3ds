#pragma once
#include <cstdio>
#include <cstdint>
// Diagnostic entry/first-export calls only. Returns false if there is no input;
// a true return says a report was produced, never that a game is playable.
bool symbianImageProbe(const char*folder,FILE*report,bool(*keepRunning)()=nullptr,void(*progress)(const char*,std::uint32_t,std::uint32_t)=nullptr);
