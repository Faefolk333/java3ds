"""Compare the optimized pixel loop with the last shipped rasterizer.

Extract the actual C functions, not a second implementation of their formulas.
The old source is supplied explicitly so this remains reproducible after commit.
"""
import argparse,json,os,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--reference',type=Path,required=True);p.add_argument('--output',type=Path);args=p.parse_args()
r=Path(__file__).resolve().parents[1];os.chdir(r)
def extract(s,prefix):
 s=s[s.index('static float micro_edge'):s.index('static void micro_draw')]
 return s.replace('micro_edge',prefix+'_edge').replace('micro_raster',prefix+'_raster')
native=(r/'native/midp_native.c').read_text();over=native[native.index('static uint32_t over'):native.index('static s32 readPixels')]
code='''#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
static float micro_depth[64*64];
static float* crimson_stereo_target(const uint32_t*d){return NULL;}
static int crimson_cancelled(void){return 0;}
'''+over+extract(args.reference.read_text(),'old')+extract((r/'native/software3d.h').read_text(),'new')+r'''
static uint32_t seed=131;
static float randomf(float max){seed=seed*1664525+1013904223;return (seed>>8)*(max/16777216.f);}
int main(){uint32_t a[4096],b[4096],tex[256];unsigned long changed=0,total=0;double before=0,after=0;
 for(int i=0;i<256;i++)tex[i]=(i%5?0xff000000:0x80000000)|((i*1234567)&0xffffff);
 for(int n=0;n<10000;n++){
  float v[3][5],p[7]={32,32,1,100,30,30,n%2};for(int k=0;k<3;k++){v[k][0]=randomf(4)-2;v[k][1]=randomf(4)-2;v[k][2]=randomf(5)+1;v[k][3]=randomf(16);v[k][4]=randomf(16);}
  memset(a,0x35,sizeof(a));memset(b,0x35,sizeof(b));int flags=n%7;
  memset(micro_depth,0,sizeof(micro_depth));clock_t t=clock();old_raster(a,64,64,v,p,n%3?tex:NULL,16,16,0xff65ac43,flags,3,4,57,55,NULL);before+=clock()-t;
  memset(micro_depth,0,sizeof(micro_depth));t=clock();new_raster(b,64,64,v,p,n%3?tex:NULL,16,16,0xff65ac43,flags,3,4,57,55,NULL);after+=clock()-t;
  for(int i=0;i<4096;i++){changed+=a[i]!=b[i];total++;}
 }
 printf("{\"triangles\":10000,\"compared_pixels\":%lu,\"changed_pixels\":%lu,\"reference_cpu_ms\":%.3f,\"optimized_cpu_ms\":%.3f}\n",total,changed,1000*before/CLOCKS_PER_SEC,1000*after/CLOCKS_PER_SEC);
}'''
(r/'work/micro-raster-compare.c').write_text(code)
env=os.environ.copy();env['PATH']='C:/devkitPro/msys64/mingw64/bin;'+env['PATH']
subprocess.run([os.environ.get('CC','C:/devkitPro/msys64/mingw64/bin/gcc.exe'),'-O2','-fno-strict-aliasing','work/micro-raster-compare.c','-lm','-o','work/micro-raster-compare.exe'],env=env,check=True)
result=json.loads(subprocess.check_output(['work/micro-raster-compare.exe'],env=env,text=True));assert result['changed_pixels']/result['compared_pixels']<.001,result
if args.output:args.output.write_text(json.dumps(result,indent=2)+'\n')
print('MICRO_RASTER_COMPARE_PASS',json.dumps(result))
