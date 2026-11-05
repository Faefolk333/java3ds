#pragma once
#include <cstdio>
// Diagnostic entry/first-export calls only. Returns false if there is no input;
// a true return says a report was produced, never that a game is playable.
bool symbianImageProbe(const char*folder,FILE*report,bool(*keepRunning)()=nullptr);
