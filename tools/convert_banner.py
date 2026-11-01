"""Convert the optimized glTF with the pinned pycgfx tool (see CIA.md)."""
import json,sys,wave
from pathlib import Path
root=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(root/'work/cia-tools/pycgfx'))
from main import convert_gltf,write
from cgfx.shared import ColorFloat
import gltflib
out=root/'assets/banner';out.mkdir(parents=True,exist_ok=True)
cgfx=convert_gltf(gltflib.GLTF.load(str(root/'work/banner/banner.glb'),load_file_resources=True))
for name in cgfx.data.models:
 model=cgfx.data.models[name]
 for material in model.materials:
  model.materials[material].material_color.emission=ColorFloat(.12,.24,.20,1)
  model.materials[material].material_color.ambient=ColorFloat(.1,.1,.1,1)
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
data=write(cgfx);assert len(data)<=512*1024,f'Banner too large: {len(data)}'
(out/'banner.cgfx').write_bytes(data)
with wave.open(str(out/'silence.wav'),'wb') as w:
 w.setnchannels(2);w.setsampwidth(2);w.setframerate(32000);w.writeframes(bytes(32000*4))
info={'cgfx_bytes':len(data),'animations':animations,'geometry':json.loads((root/'work/banner/geometry.json').read_text())}
(out/'manifest.json').write_text(json.dumps(info,indent=2)+'\n')
print(json.dumps(info,indent=2))
