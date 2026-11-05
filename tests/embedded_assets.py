from pathlib import Path
import struct,zipfile,io
root=Path(__file__).resolve().parents[1]
for name in ['java_pocket']:
 b=(root/f'work/arm/{name}.3dsx').read_bytes();assert b[:4]==b'3DSX';offset=struct.unpack_from('<I',b,40)[0];r=b[offset:];h=struct.unpack_from('<10I',r);assert h[0]==40
 files={}
 def directory(pos,prefix):
  parent,sibling,child,first,hash_,length=struct.unpack_from('<6I',r,h[3]+pos)
  label=r[h[3]+pos+24:h[3]+pos+24+length].decode('utf-16le');prefix=prefix+'/'+label if label else prefix
  while first!=0xffffffff:
   start=h[7]+first;par,nxt,data,size,hash_,length=struct.unpack_from('<IIQQII',r,start);label=r[start+32:start+32+length].decode('utf-16le');payload=r[h[9]+data:h[9]+data+size];files[(prefix+'/'+label).lstrip('/')]=payload;first=nxt
  while child!=0xffffffff:
   directory(child,prefix);child=struct.unpack_from('<I',r,h[3]+child+4)[0]
 directory(0,'')
 expected={'java/'+n for n in ['minijvm_rt.jar','midp.jar']}|{'certs/github-roots.pem','audio/gm.sf2','audio/pop-theme.mid','profiles/pop-classic.zip','demo/LauncherCheck.jar'}|{'ui/'+n+'.wav' for n in ('navigation','confirm','achievement')}|{'probe/java3ds-symbian-probe.'+n for n in ('3dsx','smdh')};assert set(files)==expected,files.keys()
 for path,payload in files.items():
  assert payload==(root/'romfs'/path).read_bytes(),path
  if path.endswith('.jar'):
   with zipfile.ZipFile(io.BytesIO(payload)) as z:assert z.testzip() is None
 print('EMBEDDED_ASSETS_PASS '+name+' verified_JARs=3 SoundFont=1 MIDI_theme=1 PoP_profile=1 UI_sounds=3')

