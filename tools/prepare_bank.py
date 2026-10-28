"""General MIDI mobile bank. Retains all presets; resamples ordinary PCM samples.
Address-generator zones are adjusted by each referenced sample's resampling ratio.
Global/coarse address offsets retain original samples to preserve SF2 semantics.
"""
from pathlib import Path
import struct,array,hashlib,json,shutil
ROOT=Path(__file__).resolve().parents[1]
def chunks(b,p=0):
 while p<len(b):
  tag=b[p:p+4];n=struct.unpack_from('<I',b,p+4)[0];yield tag,b[p+8:p+8+n];p+=8+n+(n&1)
 assert p==len(b)
def chunk(tag,b):return tag+struct.pack('<I',len(b))+b+(b'\0' if len(b)&1 else b'')
def main():
 source=ROOT.parent/'wandering-fighter/vendor/GeneralUser-GS/GeneralUser-GS.sf2';data=source.read_bytes();lists={v[:4]:v[4:] for tag,v in chunks(data,12) if tag==b'LIST'};pd=dict(chunks(lists[b'pdta']));sd=dict(chunks(lists[b'sdta']));sh=[bytearray(pd[b'shdr'][i:i+46]) for i in range(0,len(pd[b'shdr']),46)];igen=bytearray(pd[b'igen']);ibag=list(struct.iter_unpack('<HH',pd[b'ibag']));inst=[v[0] for v in struct.iter_unpack('<20xH',pd[b'inst'])];excluded=set();zones=[]
 address={0,1,2,3,4,12,45,50};coarse={4,12,45,50}
 for index in range(len(inst)-1):
  global_offsets=False
  for z in range(inst[index],inst[index+1]):
   start,end=ibag[z][0],ibag[z+1][0];gens=[(p,*struct.unpack_from('<Hh',igen,p)) for p in range(start*4,end*4,4)];sample=next((value&65535 for p,op,value in gens if op==53),None);offsets=[g for g in gens if g[1] in address and g[2]]
   if sample is None:global_offsets|=bool(offsets);continue
   zones.append((sample,gens))
   if global_offsets or any(op in coarse for p,op,value in offsets):excluded.add(sample)
 for _ in range(2):
  for i,h in enumerate(sh[:-1]):
   link,kind=struct.unpack_from('<HH',h,42)
   if kind&6 and (i in excluded or link in excluded):excluded.update([i,link])
 factors={i:1 if i in excluded else max(1,(struct.unpack_from('<I',h,36)[0]+15999)//16000) for i,h in enumerate(sh[:-1])}
 for i,h in enumerate(sh[:-1]):
  link,kind=struct.unpack_from('<HH',h,42)
  if kind&6:factors[i]=factors[link]=min(factors[i],factors[link])
 pcm=array.array('h');pcm.frombytes(sd[b'smpl']);out=array.array('h')
 for i,h in enumerate(sh[:-1]):
  start,end,loopstart,loopend,rate=struct.unpack_from('<5I',h,20);assert start<=end<=len(pcm);factor=factors[i];base=len(out);sample=pcm[start:end]
  if factor==1:out.extend(sample)
  else:
   for p in range(0,len(sample),factor):part=sample[p:p+factor];out.append(round(sum(part)/len(part)))
  finish=len(out);out.extend([0]*46)
  a=base+round((loopstart-start)/factor);b=base+round((loopend-start)/factor);a=max(base,min(finish,a));b=max(a,min(finish,b));struct.pack_into('<5I',h,20,base,finish,a,b,max(1,round(rate/factor)))
 for sample,gens in zones:
  factor=factors[sample]
  if factor>1:
   for p,op,value in gens:
    if op in {0,1,2,3} and value:struct.pack_into('<h',igen,p+2,round(value/factor))
 pd[b'igen']=bytes(igen);pd[b'shdr']=b''.join(sh);sd[b'smpl']=out.tobytes();sd.pop(b'sm24',None)
 result=chunk(b'RIFF',b'sfbk'+chunk(b'LIST',b'INFO'+lists[b'INFO'])+chunk(b'LIST',b'sdta'+b''.join(chunk(t,v) for t,v in sd.items()))+chunk(b'LIST',b'pdta'+b''.join(chunk(t,v) for t,v in pd.items())))
 dest=ROOT/'romfs/audio/gm.sf2';dest.parent.mkdir(exist_ok=True,parents=True);dest.write_bytes(result);license=ROOT/'third-party/GeneralUser-GS';shutil.copy2(source.parent/'LICENSE.txt',license/'LICENSE.txt')
 report={'source':'GeneralUser GS 2.0.3, S. Christian Collins','source_sha256':hashlib.sha256(data).hexdigest(),'derived_sha256':hashlib.sha256(result).hexdigest(),'all_presets':len(pd[b'phdr'])//38-1,'samples':len(sh)-1,'bytes':len(result),'resampled_samples':sum(v>1 for v in factors.values()),'max_ordinary_sample_rate':16000};(license/'mobile-bank.json').write_text(json.dumps(report,indent=2));print(json.dumps(report,indent=2))
if __name__=='__main__':main()
