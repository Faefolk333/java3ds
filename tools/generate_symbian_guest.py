"""Assemble the project's diagnostic ARM/Thumb/VFP instructions, not game data."""
from pathlib import Path
import os,subprocess
r=Path(__file__).resolve().parents[1];os.chdir(r);b=Path('C:/devkitPro/msys64/opt/devkitpro/devkitARM/bin');os.environ['PATH']=str(b)+';'+os.environ['PATH'];out=r/'work/symbian-guest';out.mkdir(exist_ok=True)
subprocess.run([str(b/'arm-none-eabi-as.exe'),'-o',str(out/'guest.o'),'tests/symbian_cpu_guest.S'],check=True)
subprocess.run([str(b/'arm-none-eabi-ld.exe'),'-Ttext=0x1000','--entry=probe_arm','-o',str(out/'guest.elf'),str(out/'guest.o')],check=True)
subprocess.run([str(b/'arm-none-eabi-objcopy.exe'),'-O','binary',str(out/'guest.elf'),str(out/'guest.bin')],check=True)
data=(out/'guest.bin').read_bytes();header='// Generated from tests/symbian_cpu_guest.S; original diagnostic instructions only.\n#pragma once\nstatic const unsigned char symbian_probe_guest[]={\n'+','.join('0x%02x'%v for v in data)+'\n};\n';p=Path('native/symbian/probe_guest.h')
if not p.exists() or p.read_text()!=header:p.write_text(header)
print('DIAGNOSTIC_GUEST_BYTES',len(data))
