"""Build the real EKA2L1 dyncom CPU subset for host or ARM11; no phone OS yet."""
from pathlib import Path
import argparse,os,subprocess,json
p=argparse.ArgumentParser();p.add_argument('target',choices=('host','3ds'));a=p.parse_args();root=Path(__file__).resolve().parents[1];os.chdir(root)
src=root/'third-party/eka2l1-cpu';out=root/'work'/('symbian-cpu-'+a.target);out.mkdir(exist_ok=True)
if a.target=='3ds':
 bin=Path('C:/devkitPro/msys64/opt/devkitpro/devkitARM/bin');cxx=bin/'arm-none-eabi-g++.exe';ar=bin/'arm-none-eabi-ar.exe';flags=['-march=armv6k','-mtune=mpcore','-mfloat-abi=hard','-mtp=soft','-D__3DS__']
else:
 bin=Path('C:/devkitPro/msys64/mingw64/bin');cxx=bin/'g++.exe';ar=bin/'ar.exe';flags=[]
os.environ['PATH']=str(bin)+';'+os.environ['PATH']
flags+=['-std=gnu++20','-O2','-fno-strict-aliasing','-fwrapv','-fno-rtti','-fno-exceptions','-ffunction-sections','-fdata-sections','-DEKA2L1_DYNCOM_DISABLE_VFP_TRACE=1','-DTRANS_CACHE_SIZE=8388608','-I'+str(root/'native/symbian/compat'),'-I'+str(src/'cpu/include'),'-I'+str(src/'common/include')]
files=list((src/'cpu/src/dyncom').rglob('*.cpp'))+[src/'cpu/src/12l1r/exclusive_monitor.cpp',src/'common/src/bytes.cpp'];objects=[]
stamp=out/'flags.json';settings=json.dumps([str(cxx),flags]);rebuild=not stamp.exists() or stamp.read_text()!=settings
headers=list(src.rglob('*.h'))+list(src.rglob('*.def'))+list((root/'native/symbian/compat').rglob('*.h'));header_time=max(x.stat().st_mtime for x in headers)
for file in files:
 obj=out/(file.relative_to(src).as_posix().replace('/','_')+'.o')
 if rebuild or not obj.exists() or obj.stat().st_mtime<max(file.stat().st_mtime,header_time):subprocess.run([str(cxx),*flags,'-c',str(file),'-o',str(obj)],check=True)
 objects.append(str(obj));print('CPU',a.target,file.name,flush=True)
subprocess.run([str(ar),'rcs',str(out/'libsymbian-cpu.a'),*objects],check=True);stamp.write_text(settings);print('SYMBIAN_CPU_LIBRARY_BUILT',a.target,flush=True)
