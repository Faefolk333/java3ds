"""Build the pinned decoder-only OpenCORE AMR-NB archive for host or devkitARM."""
from pathlib import Path
import argparse,concurrent.futures,os,re,subprocess
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__);p.add_argument('target',choices=['host','arm']);args=p.parse_args()
base=root/'third-party/opencore-amr';amr=base/'opencore/codecs_v2/audio/gsm_amr';nb=amr/'amr_nb'
variables={'DEC_SRC_DIR':nb/'dec/src','ENC_SRC_DIR':nb/'enc/src','COMMON_SRC_DIR':nb/'common/src'}
make=(base/'amrnb/Makefile.am').read_text();make=re.sub(r'if AMRNB_ENCODER\n.*?endif','',make,flags=re.S);make='\n'.join(l for l in make.splitlines() if not l.lstrip().startswith('#'))
sources=[base/'amrnb/wrapper.cpp']
for var,name in re.findall(r'\$\((DEC_SRC_DIR|COMMON_SRC_DIR)\)/([a-zA-Z0-9_]+\.cpp)',make):
 file=variables[var]/name
 if file not in sources:sources.append(file)
out=root/'work'/('amr-'+args.target);out.mkdir(exist_ok=True)
if args.target=='host':
 compiler='C:/devkitPro/msys64/mingw64/bin/g++.exe';archiver='C:/devkitPro/msys64/mingw64/bin/ar.exe';arch=[]
else:
 tool='C:/devkitPro/msys64/opt/devkitpro/devkitARM/bin/arm-none-eabi-';compiler=tool+'g++.exe';archiver=tool+'ar.exe';arch=['-march=armv6k','-mtune=mpcore','-mfloat-abi=hard','-mtp=soft']
flags=arch+['-O2','-std=gnu++11','-fno-strict-aliasing','-fwrapv','-fno-exceptions','-fno-rtti','-ffunction-sections','-fdata-sections','-DDISABLE_AMRNB_ENCODER']
for inc in [base/'oscl',nb/'dec/src',nb/'common/include',nb/'dec/include',amr/'common/dec/include',nb/'enc/src',base/'amrnb']:flags+=['-I'+str(inc)]
env=os.environ.copy();env['PATH']='C:/devkitPro/msys64/mingw64/bin;'+env['PATH']
stamp=' '.join(flags);force=not (out/'flags.txt').exists() or (out/'flags.txt').read_text()!=stamp
def compile_one(file):
 obj=out/(file.stem+'.o')
 if force or not obj.exists() or obj.stat().st_mtime<file.stat().st_mtime:subprocess.run([compiler,*flags,'-c',str(file),'-o',str(obj)],check=True,env=env,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
 return str(obj)
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:objects=list(pool.map(compile_one,sources))
subprocess.run([archiver,'rcs',str(out/'libamrnb.a'),*objects],check=True,env=env)
(out/'flags.txt').write_text(stamp)
print('AMR_DECODER_BUILT',args.target,'sources',len(sources))
