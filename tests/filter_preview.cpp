#include "../native/display_filter.h"
#include <cstdio>
#include <vector>
#include <cassert>
int main(int argc,char**argv){assert(argc==3);FILE*f=fopen(argv[1],"rb");assert(f);int w,h;assert(fscanf(f,"P6 %d %d 255",&w,&h)==2);fgetc(f);std::vector<uint8_t>rgb(w*h*3);assert(fread(rgb.data(),1,rgb.size(),f)==rgb.size());fclose(f);
 for(int mode=0;mode<DISPLAY_FILTER_COUNT;mode++){DisplayFilter filter={};display_filter_configure(&filter,mode);char file[1024];snprintf(file,sizeof(file),"%s-%d.ppm",argv[2],mode);f=fopen(file,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",w,h);for(int y=0;y<h;y++)for(int x=0;x<w;x++){int at=(y*w+x)*3;uint32_t c=0xff000000|rgb[at]<<16|rgb[at+1]<<8|rgb[at+2];c=display_filter_apply(&filter,c,x,y);unsigned char p[]={(uint8_t)(c>>16),(uint8_t)(c>>8),(uint8_t)c};fwrite(p,1,3,f);}fclose(f);}
}
