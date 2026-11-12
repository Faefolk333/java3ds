"""Build host integration checks for the portable EKA1 E32 loader/CPU session."""
from pathlib import Path
import os,subprocess,sys
r=Path(__file__).resolve().parents[1];os.chdir(r)
subprocess.run([sys.executable,'tools/generate_symbian_e32_guest.py'],check=True)
subprocess.run([sys.executable,'tools/build_symbian_cpu.py','host'],check=True)
b=Path('C:/devkitPro/msys64/mingw64/bin');os.environ['PATH']=str(b)+';'+os.environ['PATH']
flags=['-std=gnu++20','-O2','-fno-rtti','-fno-exceptions','-fno-strict-aliasing','-fwrapv','-DTRANS_CACHE_SIZE=8388608','-Inative/symbian/compat','-Ithird-party/eka2l1-cpu/cpu/include','-Ithird-party/eka2l1-cpu/common/include']
sources=['tests/symbian_loader_check.cpp']+['native/symbian/'+n+'.cpp' for n in ('e32_image','e32_session','rom_image','file_server','eka1_kernel','kernel_probe','loader_probe','image_probe')]
target='work/symbian-loader-check.exe';subprocess.run([str(b/'g++.exe'),*flags,*sources,'work/symbian-cpu-host/libsymbian-cpu.a','-o',target],check=True)
subprocess.run([target],check=True)
