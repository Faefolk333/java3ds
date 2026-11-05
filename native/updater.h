#pragma once
#include <string>
// State: 0 idle, 1 checking, 2 available, 3 downloading, 4 installed, 5 notice/error.
void updaterInit(const std::string&root,const std::string&executable);
void updaterCheck();void updaterInstall();bool updaterClose();
int updaterState();int updaterPercent();std::string updaterMessage();
