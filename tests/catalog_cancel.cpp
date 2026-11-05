#include "../native/catalog.h"
#include <cassert>
#include <cstdio>
static int polls;
static bool cancelled(){return ++polls>=3;}
int main(){GameInfo g;GameSettings settings;std::string error;assert(inspectGame("romfs/demo/LauncherCheck.jar",g,error));const char*path="work/catalog-cancel.jar";remove(path);assert(!prepareGame(g,settings,"romfs/profiles/pop-classic.zip",path,error,cancelled));assert(error=="Launch cancelled"&&polls==3);FILE*f=fopen(path,"rb");assert(!f);f=fopen("work/catalog-cancel.jar.part","rb");assert(!f);puts("CATALOG_CANCEL_PASS during_entries no_partial_output");}
