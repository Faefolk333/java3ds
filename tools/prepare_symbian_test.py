"""Prepare a LOCAL diagnostic SD ZIP from the owner's EKA1 files; never upload it."""
from pathlib import Path
import argparse,struct,re,zipfile,hashlib,json,binascii

def identity(name):
    match=re.fullmatch(r'([A-Za-z0-9_]+)(?:\[([0-9A-Fa-f]{8})\])?\.DLL',name,re.I)
    if not match: raise ValueError('Unsupported DLL identity: '+name)
    return match[1].upper()+'.DLL',int(match[2],16) if match[2] else 0

def image(data):
    def u32(at): return struct.unpack_from('<I',data,at)[0]
    if not 124<=len(data)<=8*1024*1024 or data[16:20]!=b'EPOC' or u32(20)!=0x2000:
        raise ValueError('Need uncompressed ARM EKA1 E32 input within 8 MiB')
    uid=binascii.crc_hqx(data[1:12:2],0)<<16|binascii.crc_hqx(data[:12:2],0)
    if u32(12)!=uid or (u32(44)&0xf0000000) or ((u32(44)&0xf000000) and u32(28)):
        raise ValueError('Invalid checked UID or unsupported E32 format')
    names=[]
    if u32(84):
        section=u32(108);end=section+u32(section);cursor=section+4
        if end>len(data) or u32(84)>256: raise ValueError('Invalid import section')
        for _ in range(u32(84)):
            if cursor+8>end: raise ValueError('Invalid import block')
            offset,count=struct.unpack_from('<II',data,cursor);cursor+=8+count*4
            if cursor>end or count>8192 or not 0<=offset<end-section: raise ValueError('Invalid import range')
            name=data[section+offset:end].split(b'\0',1)[0].decode('ascii')
            names.append(identity(name))
    return u32(0),u32(8),names

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('app','rom','libraries','output'): p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--game-libraries',type=Path,action='append',default=[],help='Owner game DLL directories; may repeat')
    a=p.parse_args();data=a.app.read_bytes();_,_,imports=image(data)
    rom=a.rom.read_bytes()
    if not 512<=len(rom)<=20*1024*1024 or struct.unpack_from('<II',rom,140)!=(0x50000000,len(rom)):
        raise ValueError('Need matching uncompressed EKA1 SYM.ROM')
    rom_files={f.name.upper():f for f in a.libraries.iterdir() if f.is_file()}
    game_files={}
    for directory in [a.app.parent,*a.game_libraries]:
        for f in directory.iterdir():
            if f.is_file() and f.suffix.lower()=='.dll':
                previous=game_files.get(f.name.upper())
                if previous and previous.read_bytes()!=f.read_bytes(): raise ValueError('Ambiguous game DLL: '+f.name)
                game_files[f.name.upper()]=f
    packed={};seen={};missing=[];game_names=[];rom_names=[]
    def resolve(name,uid):
        if name in seen:
            if uid and seen[name]!=uid: raise ValueError('Conflicting DLL UID: '+name)
            return
        source=rom_files.get(name)
        if source:
            with source.open('rb') as f: header=f.read(100)
            if len(header)==100:
                start=struct.unpack_from('<I',header,20)[0]-0x50000000-100
                actual_uid=struct.unpack_from('<I',header,8)[0]
                if start>=0 and rom[start:start+100]==header:
                    if uid and uid!=actual_uid: raise ValueError('ROM DLL UID mismatch: '+name)
                    seen[name]=actual_uid;packed['rom-libs/'+name]=header;rom_names.append(name);return
        source=game_files.get(name)
        if not source: missing.append(name);return
        content=source.read_bytes();kind,actual_uid,dependencies=image(content)
        if kind!=0x10000079 or (uid and actual_uid!=uid): raise ValueError('E32 DLL UID mismatch: '+name)
        if len(game_names)>=32: raise ValueError('Too many game DLLs')
        seen[name]=actual_uid;packed['game-libs/'+name]=content;game_names.append(name)
        for dependency in dependencies: resolve(*dependency)
    for dependency in imports: resolve(*dependency)
    if missing: raise ValueError('Missing dependency files: '+', '.join(sorted(set(missing))))
    a.output.parent.mkdir(parents=True,exist_ok=True);root='3ds/java-3ds/symbian/'
    with zipfile.ZipFile(a.output,'w',zipfile.ZIP_DEFLATED) as z:
        z.writestr(root+'test.app',data);z.writestr(root+'SYM.ROM',rom)
        for name,content in sorted(packed.items()): z.writestr(root+name,content)
        z.writestr('CTI-ME.txt','Soukromy test z vasich vlastnich dat. Nesdilet ani nenahravat na GitHub.\nRozbalte slozku 3ds do korene SD a spustte Symbian Probe -> Y.\nNejde zatim o spusteni hry. Vysledek: /3ds/java-3ds/symbian-loader.log\n')
    report={'private_local_only':True,'app_sha256':hashlib.sha256(data).hexdigest(),'rom_sha256':hashlib.sha256(rom).hexdigest(),'rom_libraries':sorted(rom_names),'game_libraries':sorted(game_names),'unresolved_game_libraries':[],'no_gameplay':True}
    a.output.with_suffix('.json').write_text(json.dumps(report,indent=2)+'\n')
    print('PRIVATE_LOCAL_TEST_ZIP',a.output,'ROM_headers',len(rom_names),'game_DLLs',len(game_names))
if __name__=='__main__': main()
