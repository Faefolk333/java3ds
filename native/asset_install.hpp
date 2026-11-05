#pragma once
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <sys/stat.h>
// Returns 0 when already identical, 1 after a verified replacement, -1 on error.
static bool pop_same_file(const char*a,const char*b){
 FILE*x=fopen(a,"rb"),*y=fopen(b,"rb");if(!x||!y){if(x)fclose(x);if(y)fclose(y);return false;}
 unsigned char xb[4096],yb[4096];bool equal=true;
 for(;;){size_t nx=fread(xb,1,sizeof(xb),x),ny=fread(yb,1,sizeof(yb),y);if(nx!=ny||memcmp(xb,yb,nx)||ferror(x)||ferror(y)){equal=false;break;}if(nx==0)break;}
 fclose(x);fclose(y);return equal;
}
static bool pop_exists(const char*p){struct stat st;return stat(p,&st)==0;}
enum class PopAssetKind { Jar, ThreeDsx, Smdh };
static int pop_install_file(const char*source,const char*dest,char*error,size_t capacity,PopAssetKind kind=PopAssetKind::Jar){
 char temp[512],backup[512];if(snprintf(temp,sizeof(temp),"%s.part",dest)>=(int)sizeof(temp)||snprintf(backup,sizeof(backup),"%s.recovery",dest)>=(int)sizeof(backup)){snprintf(error,capacity,"path too long");return -1;}
 FILE*in=fopen(source,"rb");if(!in){snprintf(error,capacity,"embedded open errno=%d",errno);return -1;}
 unsigned char magic[4],end[22];size_t n=fread(magic,1,4,in);bool valid=false;
 if(kind==PopAssetKind::Jar)valid=n==4&&!memcmp(magic,"PK\003\004",4)&&fseek(in,-22,SEEK_END)==0&&fread(end,1,22,in)==22&&!memcmp(end,"PK\005\006",4)&&end[20]==0&&end[21]==0;
 else if(n==4&&fseek(in,0,SEEK_END)==0){long length=ftell(in);valid=kind==PopAssetKind::ThreeDsx?(!memcmp(magic,"3DSX",4)&&length>=32):(!memcmp(magic,"SMDH",4)&&length==0x36c0);}
 fclose(in);if(!valid){snprintf(error,capacity,"embedded asset signature/size invalid");return -1;}
 struct stat dstInfo;if(stat(dest,&dstInfo)==0&&!S_ISREG(dstInfo.st_mode)){snprintf(error,capacity,"destination is not a regular file");return -1;}
 if(pop_same_file(source,dest))return 0;
 in=fopen(source,"rb");FILE*out=fopen(temp,"wb");if(!in||!out){int e=errno;if(in)fclose(in);if(out)fclose(out);snprintf(error,capacity,"copy open errno=%d",e);return -1;}
 unsigned char buffer[4096];bool ok=true;
 while((n=fread(buffer,1,sizeof(buffer),in))!=0)if(fwrite(buffer,1,n,out)!=n){ok=false;break;}
 if(ferror(in)||ferror(out))ok=false;
 if(fflush(out))ok=false;
 if(fclose(out))ok=false;
 fclose(in);
 if(!ok||!pop_same_file(source,temp)){snprintf(error,capacity,"copy verification failed errno=%d",errno);remove(temp);return -1;}
 bool old=pop_exists(dest);
 if(old){if(pop_exists(backup)&&remove(backup)){snprintf(error,capacity,"old recovery removal errno=%d",errno);return -1;}if(rename(dest,backup)){snprintf(error,capacity,"backup rename errno=%d",errno);return -1;}}
 if(rename(temp,dest)){int e=errno;if(old)rename(backup,dest);snprintf(error,capacity,"install rename errno=%d",e);return -1;}
 if(!pop_same_file(source,dest)){snprintf(error,capacity,"installed asset verification failed");return -1;}
 if(pop_exists(backup))remove(backup);
 return 1;
}
