#ifndef CRIMSON_ZIP_CACHE_H
#define CRIMSON_ZIP_CACHE_H
#include "bytebuf.h"
/* Explicit, read-only snapshots. Add/clear only before/after all VM threads. */
int crimson_zip_cache_add(const char *path);
void crimson_zip_cache_clear(void);
long long crimson_zip_cache_hits(void);
long long crimson_zip_cache_bytes(void);
enum { CRIMSON_ZIP_INDEX, CRIMSON_ZIP_SIZE, CRIMSON_ZIP_BUFFER, CRIMSON_ZIP_BYTEBUF };
int crimson_zip_cache_read(const char *path,const char *entry,int op,void *buffer,long long capacity,long long *result);
#endif
