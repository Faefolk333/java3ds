#pragma once
#include <cstdint>
#include <cstdio>
#include <string>
using u64=uint64_t;using u32=uint32_t;using Handle=uint32_t;using Result=int;
#define R_FAILED(r) ((r)<0)
#define R_SUCCEEDED(r) ((r)>=0)
const int ARCHIVE_SDMC=0,PATH_EMPTY=0,PATH_ASCII=1,FS_OPEN_READ=1,MEDIATYPE_SD=1;
struct AM_TitleInfo {u64 titleID;};
struct FS_Path {};
inline FS_Path fsMakePath(int,const char*){return {};}
namespace fake {inline u64 title=0x000400000F3A3D00ULL;inline int opened=0,started=0,cancelled=0,finished=0,romfsClosed=0,romfsOpened=0;inline bool shortWrite=false;inline std::string bytes;inline void reset(){title=0x000400000F3A3D00ULL;opened=started=cancelled=finished=romfsClosed=romfsOpened=0;shortWrite=false;bytes.clear();}}
inline Result amInit(){fake::opened++;return 0;}
inline void amExit(){}
inline Result FSUSER_OpenFileDirectly(Handle*h,int,FS_Path,FS_Path,int,int){*h=1;return 0;}
inline Result AM_GetCiaFileInfo(int,AM_TitleInfo*i,Handle){i->titleID=fake::title;return 0;}
inline Result FSFILE_Close(Handle){return 0;}
inline void romfsExit(){fake::romfsClosed++;}
inline Result romfsInit(){fake::romfsOpened++;return 0;}
inline Result AM_StartCiaInstall(int,Handle*h){fake::started++;*h=2;return 0;}
inline Result FSFILE_Write(Handle,u32*w,u64 offset,const void*b,size_t n,int){if(offset!=fake::bytes.size())return -1;*w=n-(fake::shortWrite?1:0);fake::bytes.append((const char*)b,*w);return 0;}
inline Result AM_FinishCiaInstall(Handle){fake::finished++;return 0;}
inline Result AM_CancelCIAInstall(Handle){fake::cancelled++;return 0;}
using s64=int64_t;using ResourceLimitType=int;
const int RESLIMIT_COMMIT=1,CUR_PROCESS_HANDLE=0;
namespace heapfake {inline s64 maximum=0,used=0;inline bool fail=false;}
inline Result svcGetResourceLimit(Handle*h,int){*h=1;return heapfake::fail?-1:0;}
inline Result svcGetResourceLimitLimitValues(s64*v,Handle,ResourceLimitType*,int){*v=heapfake::maximum;return 0;}
inline Result svcGetResourceLimitCurrentValues(s64*v,Handle,ResourceLimitType*,int){*v=heapfake::used;return 0;}
inline Result svcCloseHandle(Handle){return 0;}
