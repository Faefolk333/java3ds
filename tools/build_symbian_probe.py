"""Build a standalone hardware CPU/input probe, not a Symbian game launcher."""
from pathlib import Path
import os,subprocess,sys,shutil
r=Path(__file__).resolve().parents[1];os.chdir(r);subprocess.run([sys.executable,'tools/generate_symbian_guest.py'],check=True);subprocess.run([sys.executable,'tools/build_symbian_cpu.py','3ds'],check=True)
subprocess.run([sys.executable,'tools/generate_symbian_e32_guest.py'],check=True)
d=Path('C:/devkitPro/msys64/opt/devkitpro');b=d/'devkitARM/bin';os.environ['PATH']=str(b)+';'+str(d/'tools/bin')+';'+os.environ['PATH'];out=r/'work/arm';out.mkdir(exist_ok=True)
flags=['-std=gnu++20','-O2','-march=armv6k','-mtune=mpcore','-mfloat-abi=hard','-mtp=soft','-D__3DS__','-DTRANS_CACHE_SIZE=8388608','-fno-rtti','-fno-exceptions','-ffunction-sections','-fdata-sections','-I'+str(d/'libctru/include'),'-Inative/symbian/compat','-Ithird-party/eka2l1-cpu/cpu/include','-Ithird-party/eka2l1-cpu/common/include']
objects=[]
for name in ('cpu_probe','probe_3ds','e32_image','e32_session','rom_image','loader_probe','image_probe'):
 obj=out/('symbian-'+name+'.o');subprocess.run([str(b/'arm-none-eabi-g++.exe'),*flags,'-c','native/symbian/'+name+'.cpp','-o',str(obj)],check=True);objects.append(str(obj))
elf=out/'java3ds-symbian-probe.elf';smdh=out/'java3ds-symbian-probe.smdh';target=out/'java3ds-symbian-probe.3dsx'
heap=out/'symbian-startup_heap.o';subprocess.run([str(b/'arm-none-eabi-g++.exe'),*flags,'-c','native/cia_heap.cpp','-o',str(heap)],check=True)
subprocess.run([str(b/'arm-none-eabi-g++.exe'),*flags,'-specs=3dsx.specs','-Wl,--gc-sections','-Wl,--wrap=__system_allocateHeaps',str(heap),*objects,'work/symbian-cpu-3ds/libsymbian-cpu.a','-L'+str(d/'libctru/lib'),'-lctru','-lm','-o',str(elf)],check=True)
subprocess.run([str(d/'tools/bin/smdhtool.exe'),'--create','Java3DS Symbian CPU Probe','CPU / EKA1 loader test - no games yet','Faefolk333','assets/icon48.png',str(smdh)],check=True)
subprocess.run([str(d/'tools/bin/3dsxtool.exe'),str(elf),str(target),'--smdh='+str(smdh)],check=True);print('SYMBIAN_PROBE_BUILT',target)
embedded=r/'romfs/probe';embedded.mkdir(exist_ok=True)
for file in (target,smdh):shutil.copy2(file,embedded/file.name)
