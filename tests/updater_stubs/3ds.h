#pragma once
// Host service/thread adapters only; HTTPS, release parsing, hashing and file
// installation come from the production updater.cpp and libcurl.
#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
using u32=uint32_t;using u8=uint8_t;
#define R_FAILED(x) ((x)<0)
#define U64_MAX UINT64_MAX
struct LightLock{std::mutex mutex;};
inline void LightLock_Init(LightLock*){}
inline void LightLock_Lock(LightLock*l){l->mutex.lock();}
inline void LightLock_Unlock(LightLock*l){l->mutex.unlock();}
struct HostThread{std::thread worker;std::atomic<bool>done{false};};using Thread=HostThread*;
inline Thread threadCreate(void(*fn)(void*),void*arg,size_t,int,int,bool){Thread t=new HostThread;t->worker=std::thread([=]{fn(arg);t->done.store(true);});return t;}
inline int threadJoin(Thread t,uint64_t ns){auto start=std::chrono::steady_clock::now();while(!t->done.load()){if(ns!=U64_MAX&&std::chrono::steady_clock::now()-start>std::chrono::nanoseconds(ns))return -1;std::this_thread::sleep_for(std::chrono::milliseconds(5));}t->worker.join();return 0;}
inline void threadFree(Thread t){delete t;}
inline int socInit(u32*,size_t){return 0;}inline void socExit(){}
inline int sslcInit(int){return 0;}inline void sslcExit(){}
inline int sslcGenerateRandomData(u8*p,size_t n){memset(p,0,n);return 0;}
// Network TLS itself uses host libcurl's real secure random implementation.
inline void*memalign(size_t,size_t n){return malloc(n);}
