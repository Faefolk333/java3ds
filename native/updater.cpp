#include <3ds.h>
#include <malloc.h>
#include <curl/curl.h>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <sys/stat.h>
#include "updater.h"
#include "release_info.h"
#include "update_install.h"
extern "C" {
#include <mbedtls/sha256.h>
}
#ifdef JAVA3DS_CIA
#include "cia_update.h"
static const char*ASSET="java3ds.cia";
#else
static const char*ASSET="java3ds.3dsx";
#endif
#ifndef JAVA3DS_UPDATE_VERSION
#define JAVA3DS_UPDATE_VERSION "0.7.0"
#endif
#ifndef JAVA3DS_UPDATE_CA_FILE
#define JAVA3DS_UPDATE_CA_FILE "romfs:/certs/github-roots.pem"
#endif
static const char*VERSION=JAVA3DS_UPDATE_VERSION;
static std::atomic<int>state{0},percent{0};static std::atomic<bool>cancel{false};static LightLock lock;static Thread thread;
static std::string root,exe,message;static updates::Release release;
static void report(int s,const std::string&m){LightLock_Lock(&lock);message=m;LightLock_Unlock(&lock);state.store(s);}
static bool allowed(const std::string&u){if(u.find_first_of("\r\n")!=std::string::npos)return false;for(const char*h:{"https://api.github.com/","https://release-assets.githubusercontent.com/","https://objects.githubusercontent.com/","https://github.com/"})if(u.compare(0,strlen(h),h)==0)return true;return false;}
struct Download{std::vector<unsigned char>*memory;FILE*out;unsigned limit,total=0;bool binary;long status=0;};
static size_t header(char*p,size_t size,size_t count,void*arg){size_t n=size*count;Download&d=*(Download*)arg;if(n>=5&&!memcmp(p,"HTTP/",5)){std::string h(p,n);size_t space=h.find(' ');if(space!=std::string::npos)d.status=strtol(h.c_str()+space+1,nullptr,10);}return n;}
static size_t receive(char*p,size_t size,size_t count,void*arg){size_t n=size*count;Download&d=*(Download*)arg;if(cancel.load())return 0;if(d.status!=200)return n;if(n>d.limit-d.total)return 0;if(d.memory)d.memory->insert(d.memory->end(),p,p+n);if(d.out&&fwrite(p,1,n,d.out)!=n)return 0;d.total+=n;if(d.binary)percent.store((int)((uint64_t)d.total*100/d.limit));return n;}
static int progress(void*,curl_off_t,curl_off_t,curl_off_t,curl_off_t){return cancel.load()?1:0;}
static bool fetch(std::string url,const std::string&token,bool binary,std::vector<unsigned char>*memory,FILE*out,unsigned limit){
 const std::string originalUrl=url;
 for(int authentication=0;authentication<(token.empty()?1:2)&&!cancel.load();authentication++){
 url=originalUrl;
 for(int redirect=0;redirect<6&&!cancel.load();redirect++){
  if(!allowed(url)){report(5,"Update redirect was rejected");return false;}
  CURL*c=curl_easy_init();if(!c){report(5,"Cannot initialize HTTPS");return false;}
  Download d{memory,out,limit,0,binary};curl_slist*headers=nullptr;headers=curl_slist_append(headers,binary?"Accept: application/octet-stream":"Accept: application/vnd.github+json");headers=curl_slist_append(headers,"X-GitHub-Api-Version: 2022-11-28");
  if(authentication==1&&!token.empty()&&url.compare(0,23,"https://api.github.com/")==0){std::string auth="Authorization: Bearer "+token;headers=curl_slist_append(headers,auth.c_str());}
  curl_easy_setopt(c,CURLOPT_URL,url.c_str());curl_easy_setopt(c,CURLOPT_HTTPHEADER,headers);curl_easy_setopt(c,CURLOPT_USERAGENT,"Java3DS/0.7.0");curl_easy_setopt(c,CURLOPT_FOLLOWLOCATION,0L);curl_easy_setopt(c,CURLOPT_PROTOCOLS_STR,"https");
  curl_easy_setopt(c,CURLOPT_CAINFO,JAVA3DS_UPDATE_CA_FILE);curl_easy_setopt(c,CURLOPT_SSL_VERIFYPEER,1L);curl_easy_setopt(c,CURLOPT_SSL_VERIFYHOST,2L);
  curl_easy_setopt(c,CURLOPT_CONNECTTIMEOUT,12L);curl_easy_setopt(c,CURLOPT_TIMEOUT,600L);curl_easy_setopt(c,CURLOPT_LOW_SPEED_LIMIT,128L);curl_easy_setopt(c,CURLOPT_LOW_SPEED_TIME,20L);curl_easy_setopt(c,CURLOPT_NOSIGNAL,1L);
  curl_easy_setopt(c,CURLOPT_HEADERFUNCTION,header);curl_easy_setopt(c,CURLOPT_HEADERDATA,&d);curl_easy_setopt(c,CURLOPT_WRITEFUNCTION,receive);curl_easy_setopt(c,CURLOPT_WRITEDATA,&d);curl_easy_setopt(c,CURLOPT_NOPROGRESS,0L);curl_easy_setopt(c,CURLOPT_XFERINFOFUNCTION,progress);
  CURLcode rc=curl_easy_perform(c);long status=0;char*location=nullptr;curl_easy_getinfo(c,CURLINFO_RESPONSE_CODE,&status);curl_easy_getinfo(c,CURLINFO_REDIRECT_URL,&location);std::string next=location?location:"";curl_easy_cleanup(c);curl_slist_free_all(headers);
  if(rc!=CURLE_OK){report(5,std::string("HTTPS: ")+curl_easy_strerror(rc));return false;}
  if(status==301||status==302||status==303||status==307||status==308){if(next.empty())break;url=next;continue;}
  if(authentication==0&&!token.empty()&&(status==401||status==403||status==404))break;
  if(status!=200){report(5,status==401||status==403?"GitHub access denied / rate limit":status==404?"Test channel is private. Check during testing.":"GitHub HTTP "+std::to_string(status));return false;}return true;
 }
 }report(5,"Update cancelled / redirect limit");return false;
}
static void run(void*arg){
 std::string token;char tb[512]={};FILE*t=fopen((root+"/github-token.txt").c_str(),"rb");
 if(t){size_t n=fread(tb,1,sizeof(tb)-1,t);bool tooLong=fgetc(t)!=EOF;fclose(t);token.assign(tb,n);while(!token.empty()&&(unsigned char)token.back()<=32)token.pop_back();if(tooLong||token.find_first_of("\r\n \t")!=std::string::npos){token.clear();}}

 void*net=memalign(0x1000,1024*1024);if(!net||R_FAILED(socInit((u32*)net,1024*1024))){free(net);report(5,"Network unavailable: check Wi-Fi");return;}if(R_FAILED(sslcInit(0))){socExit();free(net);report(5,"Secure random service unavailable");return;}u8 entropyCheck[32];if(R_FAILED(sslcGenerateRandomData(entropyCheck,sizeof(entropyCheck)))){sslcExit();socExit();free(net);report(5,"Secure random generation failed");return;}if(curl_global_init(CURL_GLOBAL_DEFAULT)!=CURLE_OK){sslcExit();socExit();free(net);report(5,"HTTPS initialization failed");return;}
 if(!arg){std::vector<unsigned char>data;if(fetch("https://api.github.com/repos/Faefolk333/java3ds/releases/latest",token,false,&data,nullptr,1024*1024)){updates::Release found;if(!updates::parse(std::string(data.begin(),data.end()),found,ASSET))report(5,std::string("Release needs ")+ASSET+" + SHA-256");else if(updates::version(found.tag)<=updates::version(VERSION))report(5,"Java3DS is up to date");else{release=found;report(2,"Update "+found.tag+" available - A: install");}}}
 else{
  #ifdef JAVA3DS_CIA
  std::string temp=root+"/cache/java3ds-update.cia";
#else
  std::string temp=exe+".update";
#endif
  FILE*f=fopen(temp.c_str(),"wb");bool ok=f&&fetch(release.url,token,true,nullptr,f,release.size);if(f){if(fflush(f))ok=false;if(fclose(f))ok=false;}
  if(cancel.load())ok=false;
  std::string error;if(ok){
#ifdef JAVA3DS_CIA
   ok=installOwnCia(temp,release.size,release.sha,error);
#else
   ok=updates::install(exe,temp,release.size,release.sha,error);
#endif
if(ok){remove(temp.c_str());report(4,"Updated. Exit and reopen Java3DS.");}else report(5,error);}
  else if(state.load()!=5)report(5,"Update cancelled; original kept");if(!ok)remove(temp.c_str());
 }
 std::fill(token.begin(),token.end(),'\0');memset(tb,0,sizeof(tb));curl_global_cleanup();sslcExit();socExit();free(net);
}
void updaterInit(const std::string&r,const std::string&e){root=r;exe=e;LightLock_Init(&lock);}
void updaterCheck(){if(state.load()==1||state.load()==3)return;if(thread){threadJoin(thread,U64_MAX);threadFree(thread);thread=nullptr;}cancel.store(false);percent.store(0);report(1,"Checking GitHub over Wi-Fi...");thread=threadCreate(run,nullptr,96*1024,0x35,-2,false);if(!thread)report(5,"Cannot start update check");}
void updaterInstall(){if(state.load()!=2)return;if(thread){threadJoin(thread,U64_MAX);threadFree(thread);}cancel.store(false);percent.store(0);report(3,"Downloading verified update...");thread=threadCreate(run,(void*)1,96*1024,0x35,-2,false);if(!thread)report(5,"Cannot start download");}
bool updaterClose(){cancel.store(true);if(thread){if(R_FAILED(threadJoin(thread,2000000000ULL)))return false;threadFree(thread);thread=nullptr;}return true;}
int updaterState(){return state.load();}int updaterPercent(){return percent.load();}std::string updaterMessage(){LightLock_Lock(&lock);std::string s=message;LightLock_Unlock(&lock);return s;}
