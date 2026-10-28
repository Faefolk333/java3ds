#pragma once
#include <cstdio>
#include <cstring>
#include <string>
#include <stdint.h>
extern "C" {
#include <mbedtls/sha256.h>
}
namespace updates {
inline bool hashFile(const std::string&p,std::string&hash,unsigned&size){FILE*f=fopen(p.c_str(),"rb");if(!f)return false;mbedtls_sha256_context ctx;mbedtls_sha256_init(&ctx);mbedtls_sha256_starts(&ctx,0);unsigned char b[16384],digest[32];size=0;size_t n;while((n=fread(b,1,sizeof(b),f))){size+=n;mbedtls_sha256_update(&ctx,b,n);}bool ok=!ferror(f);fclose(f);mbedtls_sha256_finish(&ctx,digest);mbedtls_sha256_free(&ctx);char h[65];for(int i=0;i<32;i++)sprintf(h+i*2,"%02x",digest[i]);hash=h;return ok;}
inline bool install(const std::string&exe,const std::string&temp,unsigned expectedSize,const std::string&expectedHash,std::string&error){
 std::string hash;unsigned size=0;bool ok=hashFile(temp,hash,size)&&size==expectedSize&&hash==expectedHash;
 if(ok){FILE*f=fopen(temp.c_str(),"rb");char magic[4]={};ok=f&&fread(magic,1,4,f)==4&&!memcmp(magic,"3DSX",4);if(f)fclose(f);}
 if(!ok){error="Verification failed; original kept";return false;}
 std::string backup=exe+".previous";remove(backup.c_str());
 if(rename(exe.c_str(),backup.c_str())){error="Cannot back up installed app";return false;}
 if(rename(temp.c_str(),exe.c_str())){
  if(rename(backup.c_str(),exe.c_str()))error="Restore needed: rename .previous on SD";
  else error="Install failed; previous app restored";
  return false;
 }return true;
}
}
