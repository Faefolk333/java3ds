#include "crimson_zip_cache.h"
#include "miniz.h"
#include "tinycthread.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
typedef struct {char *path;unsigned char *data;size_t size;mz_zip_archive zip;mtx_t lock;} CachedZip;
static CachedZip caches[4];static int count;static long long total;static unsigned hits;
static int same_path(const char*a,const char*b){
 if(!a||!b)return 0;
 if(!strncmp(a,"sdmc:",5))a+=5;if(!strncmp(b,"sdmc:",5))b+=5;
 while(*a&&*b){char x=*a++,y=*b++;if(x=='\\')x='/';if(y=='\\')y='/';if(x!=y)return 0;}return !*a&&!*b;
}
int crimson_zip_cache_add(const char*path){
 for(int i=0;i<count;i++)if(same_path(path,caches[i].path))return 0;
 if(!path||count==4)return -1;
 FILE*f=fopen(path,"rb");if(!f)return -2;
 if(fseek(f,0,SEEK_END)){fclose(f);return -3;}long n=ftell(f);rewind(f);
 if(n<=0||n>8*1024*1024-total){fclose(f);return -4;}
 CachedZip*z=&caches[count];memset(z,0,sizeof(*z));
 z->data=malloc(n);z->path=malloc(strlen(path)+1);
 if(!z->data||!z->path){fclose(f);free(z->data);free(z->path);memset(z,0,sizeof(*z));return -5;}
 strcpy(z->path,path);size_t got=fread(z->data,1,n,f);fclose(f);
 if(got!=(size_t)n||!mz_zip_reader_init_mem(&z->zip,z->data,n,0)){free(z->data);free(z->path);memset(z,0,sizeof(*z));return -6;}
 if(mtx_init(&z->lock,mtx_plain)!=thrd_success){mz_zip_reader_end(&z->zip);free(z->data);free(z->path);memset(z,0,sizeof(*z));return -7;}
 z->size=n;total+=n;count++;return 0;
}
void crimson_zip_cache_clear(void){for(int i=0;i<count;i++){CachedZip*z=&caches[i];mz_zip_reader_end(&z->zip);mtx_destroy(&z->lock);free(z->data);free(z->path);memset(z,0,sizeof(*z));}count=0;total=0;hits=0;}
long long crimson_zip_cache_hits(void){return __atomic_load_n(&hits,__ATOMIC_RELAXED);}
long long crimson_zip_cache_bytes(void){return total;}
int crimson_zip_cache_read(const char*path,const char*entry,int op,void*buffer,long long capacity,long long*result){
 CachedZip*z=NULL;for(int i=0;i<count;i++)if(same_path(path,caches[i].path)){z=&caches[i];break;}
 if(!z)return 0;*result=-1;if(!entry)return 1;if(*entry=='/')entry++;
 mtx_lock(&z->lock);__atomic_add_fetch(&hits,1,__ATOMIC_RELAXED);
 int index=mz_zip_reader_locate_file(&z->zip,entry,NULL,0);mz_zip_archive_file_stat st;
 if(index>=0&&mz_zip_reader_file_stat(&z->zip,index,&st)){
  if(op==CRIMSON_ZIP_INDEX)*result=index;
  else if(op==CRIMSON_ZIP_SIZE)*result=st.m_uncomp_size;
  else if(st.m_uncomp_size<=0x7fffffff){
   void*dst=buffer;size_t size=(size_t)st.m_uncomp_size;
   if(op==CRIMSON_ZIP_BYTEBUF){ByteBuf*b=buffer;if(b->_alloc_size<size)bytebuf_expand(b,size);dst=b->buf;capacity=b->_alloc_size;}
   if(capacity>=0&&(unsigned long long)capacity>=size&&mz_zip_reader_extract_to_mem(&z->zip,index,dst,size,0)){
    if(op==CRIMSON_ZIP_BYTEBUF)((ByteBuf*)buffer)->wp=size;*result=0;
   }
  }
 }
 mtx_unlock(&z->lock);return 1;
}
