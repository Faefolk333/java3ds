"""Resample the supplied quiet stereo PCM banner sound and normalize its peak."""
import argparse, array, json, math, wave
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__);p.add_argument('source',type=Path);args=p.parse_args()
original=args.source.read_bytes();(root/'assets/banner/banner-source.wav').write_bytes(original)
with wave.open(str(args.source),'rb') as w:
 assert w.getnchannels()==2 and w.getsampwidth()==2
 source_rate=w.getframerate();source=array.array('h',w.readframes(w.getnframes()))
rate=32000;count=len(source)//2;result=[];radius=16;cutoff=.95*min(1,rate/source_rate)
weights={}
for i in range(round(count*rate/source_rate)):
 pos=i*source_rate/rate;at=int(pos);fraction=round(pos-at,8)
 if fraction not in weights:
  kernel=[]
  for k in range(-radius+1,radius+1):
   x=k-fraction;v=cutoff if abs(x)<1e-10 else math.sin(math.pi*cutoff*x)/(math.pi*x)
   kernel.append(v*(.5+.5*math.cos(math.pi*x/radius)))
  total=sum(kernel);weights[fraction]=[v/total for v in kernel]
 for channel in range(2):result.append(sum(source[max(0,min(count-1,at+k))*2+channel]*v for k,v in zip(range(-radius+1,radius+1),weights[fraction])))
peak=max(abs(v) for v in result);gain=(32767*10**(-1/20))/peak
pcm=array.array('h',(int(round(v*gain)) for v in result))
with wave.open(str(root/'assets/banner/banner.wav'),'wb') as w:
 w.setnchannels(2);w.setsampwidth(2);w.setframerate(rate);w.writeframes(pcm.tobytes())
report={'source_rate':source_rate,'output_rate':rate,'gain_db':20*math.log10(gain),'peak_dbfs':-1,'frames':len(pcm)//2,'leading_silence_removed':False}
(root/'assets/banner/audio-processing.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report))
