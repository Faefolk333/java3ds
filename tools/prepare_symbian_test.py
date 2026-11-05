"""Prepare a LOCAL diagnostic SD ZIP from the owner's EKA1 files; never upload it."""
from pathlib import Path
import argparse,struct,re,zipfile,hashlib,json
p=argparse.ArgumentParser(description=__doc__)
for name in ('app','rom','libraries','output'):p.add_argument('--'+name,type=Path,required=True)
a=p.parse_args();b=a.app.read_bytes();u32=lambda n:struct.unpack_from('<I',b,n)[0]
assert b[16:20]==b'EPOC' and u32(20)==0x2000 and len(b)<=8*1024*1024,'Need a validated uncompressed EKA1 APP within 8 MiB'
rom=a.rom.read_bytes();assert len(rom)<=20*1024*1024 and struct.unpack_from('<II',rom,140)==(0x50000000,len(rom))
section=u32(108);end=section+u32(section);assert end<=len(b);cursor=section+4;names=[]
for _ in range(u32(84)):
 offset,count=struct.unpack_from('<II',b,cursor);cursor+=8+count*4;assert cursor<=end
 name=b[section+offset:end].split(b'\0',1)[0].decode('ascii');match=re.fullmatch(r'([A-Za-z0-9_]+)\[([0-9A-Fa-f]{8})\]\.DLL',name,re.I);assert match,name
 names.append(match.group(1)+'.DLL')
a.output.parent.mkdir(parents=True,exist_ok=True);included=[];missing=[];root='3ds/java-3ds/symbian/'
with zipfile.ZipFile(a.output,'w',zipfile.ZIP_DEFLATED) as z:
 z.write(a.app,root+'test.app');z.write(a.rom,root+'SYM.ROM')
 for name in sorted(set(names)):
  file=a.libraries/name
  if not file.is_file():missing.append(name);continue
  with file.open('rb') as f:h=f.read(100)
  code=struct.unpack_from('<I',h,20)[0];start=code-0x50000000-100
  if len(h)!=100 or start<0 or rom[start:start+100]!=h:missing.append(name);continue
  z.writestr(root+'rom-libs/'+name,h);included.append(name)
 z.writestr('CTI-ME.txt','Soukromy test z vasich vlastnich dat. Nesdilet ani nenahravat na GitHub.\nRozbalte slozku 3ds do korene SD. Ve verzi probe 0.3.12 spustte test tlacitkem Y.\nNejde o spusteni hry: test skonci na chybejici sluzbe jadra.\nVysledek: /3ds/java-3ds/symbian-loader.log\n')
report={'private_local_only':True,'app_sha256':hashlib.sha256(b).hexdigest(),'rom_sha256':hashlib.sha256(rom).hexdigest(),'rom_libraries':included,'unresolved_game_libraries':missing,'no_gameplay':True}
a.output.with_suffix('.json').write_text(json.dumps(report,indent=2)+'\n');print('PRIVATE_LOCAL_TEST_ZIP',a.output,'ROM_headers',len(included),'unresolved_libraries',missing)
