#pragma once
#include <stdint.h>
// Reproject one scanline. Empty disocclusion samples extend the nearest farther
// surface; this cannot reconstruct geometry hidden in the original image.
static void stereo_line(const uint32_t*src,const int8_t*depth,int width,int strength,int eye,uint32_t*out){
 int best[400],owner[400];if(width<1||width>400)return;
 for(int x=0;x<width;x++){best[x]=-132;owner[x]=-1;}
 for(int x=0;x<width;x++){int z=depth[x],priority=z==-128?129:z==0?-130:z,s=(z==-128?0:z)*strength;int shift=(s>=0?s+1016:s-1016)/2032;int dest=x+eye*shift;if(dest>=0&&dest<width&&priority>=best[dest]){best[dest]=priority;owner[dest]=x;}}
 for(int x=0;x<width;x++)if(owner[x]<0){int l=x-1,r=x+1;while(l>=0&&owner[l]<0)l--;while(r<width&&owner[r]<0)r++;int pick=l<0?r:r>=width?l:best[l]<best[r]?l:r;out[x]=pick>=0&&pick<width?src[owner[pick]]:src[x];}else out[x]=src[owner[x]];
}
