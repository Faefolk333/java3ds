"""Read-only inventory of supplied Symbian files; this is not an emulator.

Header layouts are documented by EKA2L1's loader/e32img and loader/sis_fields.
Only package controller metadata is decompressed; nothing is installed/run.
"""
import argparse,json,re,struct,zlib
from pathlib import Path

def fields(data,start,end):
 while start+8<=end:
  kind,length=struct.unpack_from('<II',data,start);start+=8
  if length&0x80000000 or length>end-start:return
  yield kind,start,start+length
  start+=(length+3)&~3

def inspect(path,root):
 with path.open('rb') as f:head=f.read(124)
 result={'file':path.relative_to(root).as_posix(),'bytes':path.stat().st_size}
 if len(head)>=124 and head[16:20]==b'EPOC':
  old=struct.unpack_from('<I',head,20)[0] in (0x1000,0x2000)
  cpu=struct.unpack_from('<I' if old else '<H',head,20 if old else 122)[0]
  result.update(format='E32 image',kernel='EKA1' if old else 'EKA2',uid3='%08x'%struct.unpack_from('<I',head,8)[0],cpu={0x1000:'x86',0x2000:'ARMv4',0x2001:'ARMv5',0x2002:'ARMv6'}.get(cpu,hex(cpu)),heap_min=struct.unpack_from('<I',head,56)[0],heap_max=struct.unpack_from('<I',head,60)[0],stack=struct.unpack_from('<I',head,64)[0])
 elif len(head)>=16 and struct.unpack_from('<I',head)[0]==0x10201a7a:
  result.update(format='SISX (S60v3 or newer)',uid3='%08x'%struct.unpack_from('<I',head,8)[0])
  # The compressed controller is small and precedes the large payload data.
  with path.open('rb') as f:data=f.read(min(path.stat().st_size,4*1024*1024))
  # SISContents wraps the entire (possibly huge) payload. Its declared end
  # can exceed this bounded metadata prefix; children still need strict bounds.
  outer=[]
  if len(data)>=24:
   kind,length=struct.unpack_from('<II',data,16)
   if kind==12 and not length&0x80000000:outer=[(kind,24,min(24+length,len(data)))]
  for kind,start,end in outer:
   if kind!=12:continue
   for child,a,b in fields(data,start,end):
    if child!=3 or b-a<12:continue
    algorithm,size=struct.unpack_from('<IQ',data,a)
    if size>4*1024*1024:continue
    try:
     controller=zlib.decompress(data[a+12:b]) if algorithm==1 else data[a+12:b] if algorithm==0 else b''
     if len(controller)!=size:continue
     names=[]
     for match in re.finditer(rb'(?:[\x20-\x7e]\x00){5,}',controller):
      name=match.group().decode('utf-16le')
      if re.search(r'\.(?:jar|exe|app|dll)$',name,re.I):names.append(name)
     result['controller_native_or_jar_targets']=sorted(set(names))[:64]
    except zlib.error:result['controller']='compressed metadata could not be read'
 elif path.suffix.lower()=='.7z':
  result['format']='7z game archive'
  try:
   import py7zr
   with py7zr.SevenZipFile(path,'r') as archive:names=archive.getnames()
   result['e32_candidates']=[n for n in names if n.lower().endswith(('.app','.exe'))]
   result['firmware_candidates']=[n for n in names if n.lower().endswith(('.rom','.rpkg','.vpl'))]
  except ImportError:result['archive_inventory']='Install optional py7zr to list archive members'
 elif path.suffix.lower() in ('.sis','.sisx'):result['format']='older SIS / unidentified package'
 else:result['format']='archive or resource'
 return result

if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('folder',type=Path);p.add_argument('--output',type=Path);args=p.parse_args();folder=args.folder.resolve()
 paths=sorted(path for path in folder.rglob('*') if path.is_file() and path.suffix.lower() in ('.app','.exe','.sis','.sisx','.rom','.rpkg','.vpl','.7z'))
 report={'folder':str(folder),'firmware_files':[str(p.relative_to(folder)) for p in paths if p.suffix.lower() in ('.rom','.rpkg','.vpl')],'execution_backend':'Not implemented for 3DS; EKA2L1 platform port and device firmware required','files':[inspect(path,folder) for path in paths]}
 text=json.dumps(report,indent=2,ensure_ascii=False)+'\n'
 if args.output:args.output.write_text(text,encoding='utf-8')
 print(text)
