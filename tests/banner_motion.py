"""Evaluate serialized banner TRS against the original GLB, including HOME orbit.

This checks a specified orbit period, not HOME Menu hardware timing. A console
retest is still required; a guessed period cannot establish complete cancellation.
"""
import argparse,bisect,json,math,struct
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--cgfx',type=Path,default=root/'assets/banner/banner.cgfx');p.add_argument('--output',type=Path);args=p.parse_args()
b=args.cgfx.read_bytes()
def u(at):return struct.unpack_from('<I',b,at)[0]
def f(at):return struct.unpack_from('<f',b,at)[0]
def ptr(at):return at+struct.unpack_from('<i',b,at)[0]
def string(at):return b[at:b.index(b'\0',at)].decode()
def dictionary(at):
 assert b[at:at+4]==b'DICT';return {string(ptr(n+8)):ptr(n+12) for n in range(at+28,at+28+u(at+8)*16,16)}
def sample_curve(at,t):
 start,end=f(at),f(at+4)
 if t>end and b[at+9]==1:t=start+(t-start)%(end-start)
 count=u(at+16);assert count==1
 seg=ptr(at+20);flags=u(seg+8);assert not flags&1 and (flags>>2)&7==1 and (flags>>5)&7==6,'Expected linear float keys'
 n=u(seg+12);keys=[struct.unpack_from('<ff',b,seg+20+i*8) for i in range(n)]
 idx=bisect.bisect_right([k[0] for k in keys],t)-1
 if idx<0:return keys[0][1]
 if idx>=n-1:return keys[-1][1]
 a,v=keys[idx];z,w=keys[idx+1];return v+(w-v)*(t-a)/(z-a)
def value(member,index,t,default):
 flags=u(member);ignore=(0x10000,0x20000,0x40000,0x80000,0x100000,0x200000,0x800000,0x1000000,0x2000000)[index]
 constant=(0x40,0x80,0x100,0x200,0x400,0x800,0x2000,0x4000,0x8000)[index]
 at=member+20+index*4+(4 if index>=6 else 0)
 return default if flags&ignore else f(at) if flags&constant else sample_curve(ptr(at),t)
def multiply(a,b):return [[sum(a[y][i]*b[i][x] for i in range(4)) for x in range(4)] for y in range(4)]
def trs(scale,rotation,position):
 x,y,z=rotation;sx,cx=math.sin(x),math.cos(x);sy,cy=math.sin(y),math.cos(y);sz,cz=math.sin(z),math.cos(z)
 m=[[cz*cy,cz*sy*sx-sz*cx,cz*sy*cx+sz*sx,position[0]],[sz*cy,sz*sy*sx+cz*cx,sz*sy*cx-cz*sx,position[1]],[-sy,cy*sx,cy*cx,position[2]],[0,0,0,1]]
 for y in range(3):
  for x in range(3):m[y][x]*=scale[x]
 return m
def quaternion(q):
 x,y,z,w=q;return [math.atan2(2*(w*x+y*z),1-2*(x*x+y*y)),math.asin(max(-1,min(1,2*(w*y-z*x)))),math.atan2(2*(w*z+x*y),1-2*(y*y+z*z))]
data=b.index(b'DATA');model=dictionary(ptr(data+12))['COMMON'];skel=ptr(model+0xe0);bones=dictionary(ptr(skel+28))
animation=dictionary(ptr(data+12+9*8))['COMMON'];assert b[animation:animation+4]==b'CANM';members=dictionary(ptr(animation+28))
assert set(members)=={'Facing anchor','GreasePencil.001','GreasePencil.002','Text','Text.001'}
for name,at in members.items():assert string(ptr(at+4))==name and u(at+16)==5,'Bad animation target/type'
for at in bones.values():assert u(at+212)==0,'Non-inherited parent billboard is not a solution'
anchor=bones['Facing anchor'];assert struct.unpack_from('<9f',b,anchor+32)==(1,1,1,0,0,0,0,0,0)
def transform(name,t):
 at=bones[name];base=list(struct.unpack_from('<9f',b,at+32));member=members.get(name)
 if member is not None:base=[value(member,i,t,base[i]) for i in range(9)]
 return trs(base[:3],base[3:6],base[6:])
glb=(root/'assets/banner/banner.glb').read_bytes();json_size=struct.unpack_from('<I',glb,12)[0];g=json.loads(glb[20:20+json_size]);raw=glb[28+json_size:]
def accessor(index):
 a=g['accessors'][index];v=g['bufferViews'][a['bufferView']];size={'SCALAR':1,'VEC3':3,'VEC4':4}[a['type']];assert a['componentType']==5126
 start=v.get('byteOffset',0)+a.get('byteOffset',0);stride=v.get('byteStride',size*4)
 return [struct.unpack_from('<'+'f'*size,raw,start+i*stride) for i in range(a['count'])]
tracks={}
for anim in g['animations']:
 for c in anim['channels']:
  s=anim['samplers'][c['sampler']];tracks[c['target']['node'],c['target']['path']]=(accessor(s['input']),accessor(s['output']))
def source(index,seconds):
 node=g['nodes'][index];parts={k:node.get(k,v) for k,v in [('scale',[1,1,1]),('rotation',[0,0,0,1]),('translation',[0,0,0])]}
 for k in parts:
  if (index,k) in tracks:
   times,values=tracks[index,k];j=max(0,bisect.bisect_right([t[0] for t in times],seconds+1e-6)-1);parts[k]=values[j]
 return trs(parts['scale'],quaternion(parts['rotation']),parts['translation'])
info=json.loads((root/'assets/banner/manifest.json').read_text())['orbit_compensation'];period=info['seconds_per_turn'];total=f(animation+20);assert total==info['combined_seconds']*60
assert abs(value(members['Facing anchor'],4,total,0)+2*math.pi*total/(period*60))<1e-4
maximum=0;samples=0
for tick in range(0,int(total),15):
 home=trs([1,1,1],[0,2*math.pi*tick/(period*60),0],[0,0,0]);cancel=multiply(home,transform('Facing anchor',tick))
 for i,node in enumerate(g['nodes']):
  if 'mesh' not in node:continue
  name=node['name'];assert struct.unpack_from('<i',b,bones[name]+12)[0]==u(anchor+8)
  actual=multiply(cancel,transform(name,tick));expected=source(i,(tick%300)/60)
  error=max(abs(actual[y][x]-expected[y][x]) for y in range(4) for x in range(4));maximum=max(maximum,error);samples+=1
assert maximum<.001,(maximum,samples)
report={'serialized_TRS_samples':samples,'max_matrix_error':maximum,'assumed_HOME_orbit_seconds':period,'authored_loop_seconds':5,'hardware_timing_verified':False,'CGFX_bytes':len(b)}
if args.output:args.output.write_text(json.dumps(report,indent=2)+'\n')
print('BANNER_MOTION_PASS',json.dumps(report))
