#pragma once
#include <string>
#include <vector>
#include <map>
#include <stdint.h>
struct BrowserEntry{std::string name,path;bool folder;};
struct GameEntry{std::string title,main;};
struct GameInfo{std::string source,jar,title,id;std::map<std::string,std::string> properties;std::vector<GameEntry> midlets;int nativeWidth=320,nativeHeight=240;bool resolutionKnown=false;std::string unsupported;bool pop=false;uint32_t crc=0;};
struct GameSettings{int width=320,height=240,scale=0,keys=0,midlet=0;bool hideBottom=false;int bindings[14]={};int bottomTimeout=30;int stereo=0,stereoStrength=3;};
std::string joinPath(const std::string&a,const std::string&b);
std::string parentPath(const std::string&path);
bool listGames(const std::string&folder,std::vector<BrowserEntry>&out,std::string&error);
bool inspectGame(const std::string&path,GameInfo&out,std::string&error);
bool prepareGame(const GameInfo&game,const GameSettings&settings,const std::string&profile,const std::string&dest,std::string&error);
bool writeLaunchConfig(const GameInfo&game,const GameSettings&settings,const std::string&saveRoot,const std::string&path,std::string&error);
bool loadSettings(const std::string&path,GameSettings&settings);
bool saveSettings(const std::string&path,const GameSettings&settings);
