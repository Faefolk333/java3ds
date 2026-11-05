"""Build an original uncompressed EKA1 E32 test file, without any phone assets."""
from pathlib import Path
import struct,subprocess,os,binascii
r=Path(__file__).resolve().parents[1];os.chdir(r)
b=Path('C:/devkitPro/msys64/opt/devkitpro/devkitARM/bin');os.environ['PATH']=str(b)+';'+os.environ['PATH']
out=r/'work/symbian-e32';out.mkdir(exist_ok=True)
subprocess.run([str(b/'arm-none-eabi-as.exe'),'-o',str(out/'guest.o'),'tests/symbian_e32_guest.S'],check=True)
subprocess.run([str(b/'arm-none-eabi-objcopy.exe'),'-O','binary',str(out/'guest.o'),str(out/'guest.bin')],check=True)
code=(out/'guest.bin').read_bytes();assert len(code)==0xc8
h=bytearray(124);pack=lambda at,*values:struct.pack_into('<'+'I'*len(values),h,at,*values)
pack(0,0x10000079,0x100039ce,0xe0000001)
uids=bytes(h[:12]);pack(12,binascii.crc_hqx(uids[1::2],0)<<16|binascii.crc_hqx(uids[::2],0))
pack(16,0x434f5045,0x2000,0,0,0,0,0)
data=struct.pack('<II',41,0);imp=struct.pack('<IIII',36,16,1,7)+b'PROBE[10000001].DLL\0';assert len(imp)==36
relocs=[0x2080,0x1084,0x3088,0x308c]
block=struct.pack('<II',0,8+len(relocs)*2)+struct.pack('<'+'H'*len(relocs),*relocs)
rel=struct.pack('<II',len(block),len(relocs))+block
co=124;do=co+len(code);io=do+len(data);ro=io+len(imp)
pack(44,3,len(code),len(data),4096,1024*1024,8192,8,0,0x10000000,0x00400000,1,co+0xa0,5,0xc0,co,do,io,ro,0,350)
image=bytes(h)+code+data+imp+rel
(out/'original-e32-test.app').write_bytes(image)
header='// Generated from tests/symbian_e32_guest.S and original E32 fixture metadata.\n#pragma once\nstatic const unsigned char symbian_e32_guest[]={\n'+','.join('0x%02x'%n for n in image)+'\n};\n'
p=r/'native/symbian/e32_guest.h'
if not p.exists() or p.read_text()!=header:p.write_text(header)
print('ORIGINAL_E32_GUEST_BYTES',len(image))
