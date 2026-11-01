from pathlib import Path
import struct,subprocess,sys,hashlib
elf=Path(sys.argv[1]);nm='C:/devkitPro/msys64/opt/devkitpro/devkitARM/bin/arm-none-eabi-addr2line.exe'
for path in sorted(Path('C:/Users/Administrator/Desktop').glob('crash_dump_*.dmp')):
 b=path.read_bytes()
 if len(b)<40 or b[:8]!=bytes.fromhex('dec0addefecaadde'):continue
 processor,kind,total,regs,code,stack,extra=struct.unpack_from('<7I',b,12)
 if processor!=11 or len(b)<40+regs+code+stack+extra:continue
 name=b[40+regs+code+stack:][:8].rstrip(b'\0').decode('ascii',errors='replace')
 r=struct.unpack_from('<23I',b,40);print(path.name,'process='+name,'exception='+str(kind),'PC='+hex(r[15]),'LR='+hex(r[14]),'sha256='+hashlib.sha256(b).hexdigest())
 if name=='Java3DS':print(subprocess.check_output([nm,'-f','-e',str(elf),hex(r[15]),hex(r[14])],text=True))
