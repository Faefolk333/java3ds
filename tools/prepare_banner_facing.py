"""Adapt the working Faedrops mesh-bound billboard layout to Java3DS artwork.

Keep the authored GLB immutable. Bake the initial TRS into each mesh, then
express ALL authored motion as relative TRS, including Java's small rotation.
Unlike Faedrops' translation-only preparation, no authored channel is dropped.
"""
import copy,json,math,struct
from pathlib import Path
import numpy as np

def matrix(t,q,s):
 x,y,z,w=q
 r=np.array([[1-2*(y*y+z*z),2*(x*y-z*w),2*(x*z+y*w)],[2*(x*y+z*w),1-2*(x*x+z*z),2*(y*z-x*w)],[2*(x*z-y*w),2*(y*z+x*w),1-2*(x*x+y*y)]])
 m=np.eye(4);m[:3,:3]=r@np.diag(s);m[:3,3]=t;return m

def decompose(m):
 s=np.linalg.norm(m[:3,:3],axis=0);r=m[:3,:3]/s
 if np.linalg.det(r)<0:s[0]=-s[0];r[:,0]=-r[:,0]
 # Symmetric eigensolver handles rotations close to 180 degrees as well.
 k=np.array([[r[0,0]-r[1,1]-r[2,2],r[1,0]+r[0,1],r[2,0]+r[0,2],r[2,1]-r[1,2]],
 [r[1,0]+r[0,1],r[1,1]-r[0,0]-r[2,2],r[2,1]+r[1,2],r[0,2]-r[2,0]],
 [r[2,0]+r[0,2],r[2,1]+r[1,2],r[2,2]-r[0,0]-r[1,1],r[1,0]-r[0,1]],
 [r[2,1]-r[1,2],r[0,2]-r[2,0],r[1,0]-r[0,1],r.trace()]])/3
 _,v=np.linalg.eigh(k);q=v[:,-1]
 if q[3]<0:q=-q
 error=float(np.max(abs(matrix(m[:3,3],q,s)-m)))
 assert error<.001,('Relative animation contains shear',error)
 return m[:3,3],q,s

def prepare(root):
 source=root/'assets/banner/banner.glb';blob=source.read_bytes();n=struct.unpack_from('<I',blob,12)[0];g=json.loads(blob[20:20+n]);raw=blob[28+n:];data=bytearray(raw)
 def accessor(i):
  a=g['accessors'][i];v=g['bufferViews'][a['bufferView']];width={'SCALAR':1,'VEC2':2,'VEC3':3,'VEC4':4}[a['type']];assert a['componentType']==5126
  return np.ndarray((a['count'],width),'<f4',buffer=raw,offset=v.get('byteOffset',0)+a.get('byteOffset',0),strides=(v.get('byteStride',width*4),4)).astype(float)
 tracks={}
 for anim in g.get('animations',[]):
  for c in anim['channels']:
   sampler=anim['samplers'][c['sampler']];assert sampler.get('interpolation','LINEAR')=='LINEAR';tracks[c['target']['node'],c['target']['path']]=(accessor(sampler['input'])[:,0],accessor(sampler['output']))
 def pose(i,t):
  node=g['nodes'][i];p={k:np.array(node.get(k,v),float) for k,v in [('translation',[0,0,0]),('rotation',[0,0,0,1]),('scale',[1,1,1])]}
  for key in p:
   if (i,key) not in tracks:continue
   times,values=tracks[i,key];j=max(0,np.searchsorted(times,t,side='right')-1)
   if j==len(times)-1:p[key]=values[j];continue
   f=(t-times[j])/(times[j+1]-times[j]);a,b=values[j],values[j+1]
   if key=='rotation':
    dot=float(a@b)
    if dot<0:b=-b;dot=-dot
    if dot<.9995:theta=math.acos(min(1,dot));p[key]=(a*math.sin((1-f)*theta)+b*math.sin(f*theta))/math.sin(theta)
    else:p[key]=a+(b-a)*f;p[key]/=np.linalg.norm(p[key])
   else:p[key]=a+(b-a)*f
  return matrix(p['translation'],p['rotation'],p['scale'])
 def add(values,kind):
  while len(data)%4:data.append(0)
  array=np.asarray(values,dtype='<f4');start=len(data);data.extend(array.tobytes());view=len(g['bufferViews']);g['bufferViews'].append({'buffer':0,'byteOffset':start,'byteLength':array.nbytes});index=len(g['accessors']);g['accessors'].append({'bufferView':view,'componentType':5126,'count':len(array),'type':kind});return index
 nodes=[];channels=[];samplers=[];records=[];seen=set();maximum=0
 for old,node in enumerate(g['nodes']):
  if 'mesh' not in node:continue
  initial=pose(old,0);inverse=np.linalg.inv(initial);normal=np.linalg.inv(initial[:3,:3]).T;name='name_'+node['name'].replace('.','_')
  for primitive in g['meshes'][node['mesh']]['primitives']:
   for semantic,index in primitive['attributes'].items():
    if semantic not in ('POSITION','NORMAL'):continue
    assert index not in seen;seen.add(index);values=accessor(index);baked=values@initial[:3,:3].T+initial[:3,3] if semantic=='POSITION' else values@normal.T
    if semantic=='NORMAL':baked/=np.maximum(np.linalg.norm(baked,axis=1)[:,None],1e-15)
    a=g['accessors'][index];v=g['bufferViews'][a['bufferView']];at=v.get('byteOffset',0)+a.get('byteOffset',0);stride=v.get('byteStride',12)
    for j,row in enumerate(baked):struct.pack_into('<3f',data,at+j*stride,*row)
    if semantic=='POSITION':a['min']=baked.min(axis=0).tolist();a['max']=baked.max(axis=0).tolist()
  times=sorted(set(t for (i,_),(ts,_) in tracks.items() if i==old for t in ts));outputs=[[],[],[]]
  for t in times:
   expected=pose(old,t);relative=expected@inverse;tr,q,sc=decompose(relative)
   rounded=[np.asarray(v,dtype=np.float32).astype(float) for v in (tr,q,sc)];error=float(np.max(abs(matrix(*rounded)@initial-expected)));maximum=max(maximum,error)
   for dest,value in zip(outputs,(tr,q,sc)):dest.append(value)
  index=len(nodes);nodes.append({'name':name,'mesh':node['mesh'],'translation':[0,0,0],'rotation':[0,0,0,1],'scale':[1,1,1]})
  times_index=add([[t] for t in times],'SCALAR');g['accessors'][times_index]['min']=[min(times)];g['accessors'][times_index]['max']=[max(times)]
  for field,values,kind in zip(('translation','rotation','scale'),outputs,('VEC3','VEC4','VEC3')):
   sampler=len(samplers);samplers.append({'input':times_index,'output':add(values,kind),'interpolation':'LINEAR'});channels.append({'sampler':sampler,'target':{'node':index,'path':field}})
  records.append({'authored':node['name'],'facing':name,'samples':len(times)})
 count=len(nodes);nodes.extend([{'name':'world'},{'name':'COMMON','children':[count]+list(range(count))}]);g['nodes']=nodes;g['scenes']=[{'nodes':[count+1]}];g['scene']=0;g['animations']=[{'name':'Authored motion','samplers':samplers,'channels':channels}];g['buffers']=[{'byteLength':len(data)}]
 text=json.dumps(g,separators=(',',':')).encode();text+=b' '*((-len(text))%4);data.extend(b'\0'*((-len(data))%4));result=struct.pack('<III',0x46546c67,2,12+8+len(text)+8+len(data))+struct.pack('<II',len(text),0x4e4f534a)+text+struct.pack('<II',len(data),0x004e4942)+data
 # Camera-baked source TRS has small anisotropic/shear rounding. The matrix
 # tolerance is in scene units (not pixels); retain/report the measured error.
 assert maximum<.005,maximum
 target=root/'assets/banner/banner-facing.glb';target.write_bytes(result)
 return target,{'method':'Faedrops 0.27 mesh-bound YAxial; baked base TRS; relative authored TRS','meshes':records,'max_relative_TRS_error':maximum,'rotation_channels_retained':count,'timed_counter_rotation':False}

if __name__=='__main__':
 path,info=prepare(Path(__file__).resolve().parents[1]);print(json.dumps(info,indent=2))
