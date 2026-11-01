"""Check CTRTool-extracted CIA assets, decompressed CGFX and original WAV PCM."""
import argparse, hashlib, json, struct, sys, wave
from pathlib import Path
root=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(root/'tools'))
from check_banner import check_cgfx

def lz11(data):
 assert data[0]==0x11
 size=int.from_bytes(data[1:4],'little');result=bytearray();pos=4
 while len(result)<size:
  flags=data[pos];pos+=1
  for bit in range(8):
   if len(result)>=size:break
   if flags&(0x80>>bit):
    a,b=data[pos:pos+2];pos+=2;kind=a>>4
    if kind==0:
     c=data[pos];pos+=1;length=((a&15)<<4|b>>4)+0x11;distance=((b&15)<<8|c)+1
    elif kind==1:
     c,d=data[pos:pos+2];pos+=2;length=((a&15)<<12|b<<4|c>>4)+0x111;distance=((c&15)<<8|d)+1
    else:length=kind+1;distance=((a&15)<<8|b)+1
    assert 0<distance<=len(result)
    for _ in range(length):result.append(result[-distance])
   else:result.append(data[pos]);pos+=1
 assert len(result)==size
 return result

def verify(extracted):
 files=[]
 for p in (root/'romfs').rglob('*'):
  if p.is_file():
   name=p.relative_to(root/'romfs');assert p.read_bytes()==(extracted/'romfs'/name).read_bytes(),name;files.append(name.as_posix())
 assert (extracted/'exefs/icon.bin').read_bytes()==(root/'work/arm/java3ds-cia.smdh').read_bytes()
 banner=(extracted/'exefs/banner.bin').read_bytes();assert banner==(root/'assets/banner/banner.bnr').read_bytes()
 assert banner[:4]==b'CBMD'
 start=struct.unpack_from('<I',banner,8)[0];audio=struct.unpack_from('<I',banner,0x84)[0]
 cgfx=lz11(banner[start:audio]);assert cgfx==(root/'assets/banner/banner.cgfx').read_bytes();checks=check_cgfx(cgfx)
 cwav=banner[audio:];assert cwav[:4]==b'CWAV' and struct.unpack_from('<I',cwav,12)[0]==len(cwav)
 info,data=struct.unpack_from('<I',cwav,0x18)[0],struct.unpack_from('<I',cwav,0x24)[0]
 assert cwav[info:info+4]==b'INFO' and cwav[data:data+4]==b'DATA'
 assert cwav[info+8]==1 and cwav[info+9]==0
 rate,_,frames=struct.unpack_from('<III',cwav,info+12);channels=struct.unpack_from('<I',cwav,info+28)[0];assert channels==2
 pcm=[]
 for ch in range(channels):
  offset=struct.unpack_from('<I',cwav,info+36+8*ch)[0];channel=info+28+offset;sample=struct.unpack_from('<I',cwav,channel+4)[0];pcm.append(cwav[data+8+sample:data+8+sample+frames*2])
 interleaved=bytearray(frames*4)
 for i in range(frames):interleaved[i*4:i*4+2]=pcm[0][i*2:i*2+2];interleaved[i*4+2:i*4+4]=pcm[1][i*2:i*2+2]
 with wave.open(str(root/'assets/banner/banner.wav'),'rb') as original:
  assert (original.getnchannels(),original.getsampwidth(),original.getframerate(),original.getnframes())==(2,2,rate,frames)
  assert interleaved==original.readframes(frames)
 return {'romfs_files_equal':files,'smdh_equal':True,'banner_equal':True,'packed_cgfx':checks,'cwav_original_pcm_equal':True,'cwav_sample_rate':rate,'cwav_frames':frames,'cia_sha256':hashlib.sha256((root/'work/arm/java3ds.cia').read_bytes()).hexdigest()}

if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('extracted',type=Path);parser.add_argument('--output',type=Path);args=parser.parse_args()
 report=json.dumps(verify(args.extracted),indent=2)+'\n'
 if args.output:args.output.write_text(report,encoding='utf-8')
 print(report)
