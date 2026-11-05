"""Convert the optimized glTF with the pinned pycgfx tool (see CIA.md)."""
import json,sys,wave
from pathlib import Path
root=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(root/'work/cia-tools/pycgfx'))
from main import convert_gltf,write
from cgfx.shared import ColorFloat
from cgfx.sobj import BillboardMode,BoneFlag
from prepare_banner_facing import prepare
from check_banner import check_cgfx
import gltflib
out=root/'assets/banner';out.mkdir(parents=True,exist_ok=True)
source,facing=prepare(root)
gltf=gltflib.GLTF.load(str(source),load_file_resources=True)
cgfx=convert_gltf(gltf)
facing_names={node.name for node in gltf.model.nodes if node.mesh is not None}
for name in cgfx.data.models:
 model=cgfx.data.models[name]
 # Upstream sorts opaque/transparent bones after constructing the Patricia
 # dictionary. Its stale branch indices resolve names to unrelated bones and
 # crash HOME Menu before this application's code runs. Rebuild after sorting.
 model.skeleton.bones.dict.regenerate()
 model.skeleton.root_bone.billboard_mode=BillboardMode.Off
 for node_name in facing_names:
  bone=model.skeleton.bones[node_name];assert bone is not None
  bone.billboard_mode=BillboardMode.YAxial
  assert tuple(bone.position.values())==(0,0,0)
  assert tuple(bone.rotation.values())==(0,0,0)
 assert model.skeleton.bones["Build marker"] is None
 for mesh in model.meshes.data.contents:
  shape=model.shapes.data.contents[mesh.shape_index]
  sets=shape.primitive_sets.data.contents
  assert len(sets)==1 and sets[0].skinning_mode==0,'Only rigid banner nodes are supported'
  bones=sets[0].related_bones.data.contents
  assert len(bones)==1,'Rigid mesh must refer to one bone'
  mesh.mesh_node_name=model.skeleton.bones[bones[0]].name
 for material in model.materials:
  model.materials[material].material_color.emission=ColorFloat(.08,.16,.13,1)
  model.materials[material].material_color.specular[0]=ColorFloat(.95,1,1,1)
  model.materials[material].material_color.ambient=ColorFloat(.22,.28,.25,1)
animations=[]
for name in cgfx.data.skeletal_animations:
 a=cgfx.data.skeletal_animations[name];a.looping=True
 assert abs(a.frame_size-300)<.01,(name,a.frame_size)
 for member in a.member_animations_data:
  bone=a.member_animations_data[member]
  for component in ('pos_x','pos_y','pos_z','rot_x','rot_y','rot_z','scale_x','scale_y','scale_z'):
   curve=getattr(bone,component,None)
   if curve is not None and getattr(curve,'segments',None):
    # Repair upstream's copied Z start-frame assignment.
    curve.start_frame=curve.segments[0].start_frame
    curve.end_frame=curve.segments[-1].end_frame
 animations.append({'name':name,'frames_at_60_hz':a.frame_size,'looping':a.looping,'members':a.member_animations_data.len()})
assert animations,'The banner must include actual animation'
data=write(cgfx)
checks=check_cgfx(data)
(out/'banner.cgfx').write_bytes(data)
with wave.open(str(out/'banner.wav'),'rb') as w:
 assert w.getnchannels()==2 and w.getsampwidth()==2 and w.getcomptype()=='NONE','Expected stereo PCM16'
 duration=w.getnframes()/w.getframerate()
 assert 0<duration<=3,'Banner sound must be at most three seconds'
 audio={'channels':w.getnchannels(),'sample_bits':16,'sample_rate':w.getframerate(),'frames':w.getnframes(),'seconds':duration}
info={'cgfx_bytes':len(data),'binary_checks':checks,'audio':audio,'animations':animations,'facing_preparation':facing,'root_billboard':0,'mesh_billboard':5,'hardware_verified':False,'geometry':json.loads((root/'assets/banner/geometry.json').read_text(encoding='utf-8'))}
(out/'manifest.json').write_text(json.dumps(info,indent=2)+'\n',encoding='utf-8')
print(json.dumps(info,indent=2))
