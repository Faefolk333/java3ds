#pragma once
#include <atomic>
#include <mutex>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cassert>
typedef int Result;
typedef int16_t s16;
#define R_FAILED(x) ((x)<0)
#define U64_MAX UINT64_MAX
enum {NDSP_WBUF_FREE,NDSP_WBUF_QUEUED,NDSP_WBUF_PLAYING,NDSP_WBUF_DONE};
enum {NDSP_OUTPUT_STEREO,NDSP_INTERP_LINEAR,NDSP_FORMAT_MONO_PCM16};
struct LightLock{std::mutex value;};
struct ndspWaveBuf{int16_t*data_pcm16;uint32_t nsamples;unsigned status;};
typedef std::thread* Thread;
extern int audioFailure,audioDspExits,audioLinearFrees;
extern std::atomic<unsigned> audioBuffers,audioNonzero;
inline void LightLock_Init(LightLock*){}
inline void LightLock_Lock(LightLock*l){l->value.lock();}
inline void LightLock_Unlock(LightLock*l){l->value.unlock();}
inline Result ndspInit(){return audioFailure==1?-1:0;}
inline void ndspExit(){audioDspExits++;}
inline void*linearAlloc(size_t n){return audioFailure==2?nullptr:malloc(n);}
inline void linearFree(void*p){audioLinearFrees++;free(p);}
inline void ndspChnWaveBufClear(int c){assert(c==0);}
inline void ndspSetOutputMode(int){}
inline void ndspChnReset(int){}
inline void ndspChnSetInterp(int,int){}
inline void ndspChnSetRate(int,int r){assert(r==22050);}
inline void ndspChnSetFormat(int,int){}
inline void ndspChnSetMix(int,float*m){assert(m[0]==1&&m[1]==1);}
inline Result DSP_FlushDataCache(void*,size_t n){assert(n==2048);return audioFailure==4?-1:0;}
inline void ndspChnWaveBufAdd(int c,ndspWaveBuf*w){assert(c==0&&w->nsamples==1024&&w->data_pcm16);audioBuffers++;for(unsigned i=0;i<w->nsamples;i++)if(w->data_pcm16[i])audioNonzero++;w->status=NDSP_WBUF_DONE;std::this_thread::sleep_for(std::chrono::milliseconds(1));}
inline void svcSleepThread(int64_t n){std::this_thread::sleep_for(std::chrono::nanoseconds(n));}
inline Thread threadCreate(void(*fn)(void*),void*p,size_t,int,int,bool){return audioFailure==3?nullptr:new std::thread(fn,p);}
inline int threadJoin(Thread t,uint64_t){t->join();return 0;}
inline void threadFree(Thread t){delete t;}
