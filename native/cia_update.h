#pragma once
#include <3ds.h>
#include <string>
#include <cstdio>
#include "update_install.h"
static constexpr u64 JAVA3DS_TITLE_ID=0x000400000F3A3D00ULL;
// Called only after the user selects Install, while no game is running.
static bool installOwnCia(const std::string&path,unsigned expectedSize,const std::string&expectedHash,std::string&error){
 std::string hash;unsigned size=0;
 if(!updates::hashFile(path,hash,size)||size!=expectedSize||hash!=expectedHash){error="CIA verification failed; app kept";return false;}
 Result rc=amInit();if(R_FAILED(rc)){error="CIA install service unavailable";return false;}
 Handle source=0;rc=FSUSER_OpenFileDirectly(&source,ARCHIVE_SDMC,fsMakePath(PATH_EMPTY,""),fsMakePath(PATH_ASCII,path.c_str()),FS_OPEN_READ,0);
 AM_TitleInfo info={};if(R_SUCCEEDED(rc)){rc=AM_GetCiaFileInfo(MEDIATYPE_SD,&info,source);FSFILE_Close(source);}
 if(R_FAILED(rc)||info.titleID!=JAVA3DS_TITLE_ID){amExit();error="Rejected CIA: wrong application ID";return false;}
 FILE*f=fopen(path.c_str(),"rb");if(!f){amExit();error="Cannot read downloaded CIA";return false;}
 // Release RomFS before replacing this title. UI assets are compiled into code.
 romfsExit();Handle install=0;rc=AM_StartCiaInstall(MEDIATYPE_SD,&install);
 bool started=R_SUCCEEDED(rc);unsigned char buffer[32768];u64 offset=0;
 while(R_SUCCEEDED(rc)&&offset<size){size_t n=fread(buffer,1,sizeof(buffer),f);if(!n){rc=-1;break;}u32 written=0;rc=FSFILE_Write(install,&written,offset,buffer,n,0);if(R_SUCCEEDED(rc)&&written!=n)rc=-1;offset+=n;}
 if(ferror(f))rc=-1;fclose(f);
 if(R_SUCCEEDED(rc))rc=AM_FinishCiaInstall(install);else if(started)AM_CancelCIAInstall(install);
 amExit();
 if(R_FAILED(rc)){romfsInit();char msg[96];snprintf(msg,sizeof(msg),"CIA install failed: %08lx",(unsigned long)rc);error=msg;return false;}
 return true;
}
