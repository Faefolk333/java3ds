"""Convert the optimized glTF with the pinned pycgfx tool (see CIA.md)."""
import json,sys,wave
from pathlib import Path
root=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(root/'work/cia-tools/pycgfx'))
from main import convert_gltf,write
from cgfx.shared import ColorFloat
from cgfx.sobj import BillboardMode,BoneFlag
from check_banner import check_cgfx
import gltflib
out=root/'assets/banner';out.mkdir(parents=True,exist_ok=True)
cgfx=convert_gltf(gltflib.GLTF.load(str(root/'assets/banner/banner.glb'),load_file_resources=True))
for name in cgfx.data.models:
 model=cgfx.data.models[name]
 # Upstream sorts opaque/transparent bones after constructing the Patricia
 # dictionary. Its stale branch indices resolve names to unrelated bones and
 # crash HOME Menu before this application's code runs. Rebuild after sorting.
 model.skeleton.bones.dict.regenerate()
 anchor=model.skeleton.bones["Facing anchor"];assert anchor is not None
 anchor.billboard_mode=BillboardMode.YAxial
 anchor.flags=(anchor.flags & ~BoneFlag.IsIdentity)|BoneFlag.IsLocalMatrixCalculate|BoneFlag.IsWorldMatrixCalculate
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
info={'cgfx_bytes':len(data),'binary_checks':checks,'audio':audio,'animations':animations,'geometry':json.loads((root/'assets/banner/geometry.json').read_text(encoding='utf-8'))}
(out/'manifest.json').write_text(json.dumps(info,indent=2)+'\n',encoding='utf-8')
print(json.dumps(info,indent=2))
