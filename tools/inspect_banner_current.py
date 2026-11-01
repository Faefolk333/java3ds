import bpy,json
from pathlib import Path
s=bpy.context.scene;o=Path(__file__).resolve().parents[1]/'work/banner-v034';o.mkdir(exist_ok=True)
info={'fps':s.render.fps,'fps_base':s.render.fps_base,'start':s.frame_start,'end':s.frame_end,'camera':s.camera.name if s.camera else None,'camera_matrix':list(sum((list(row) for row in s.camera.matrix_world),[])) if s.camera else None,'objects':[]}
for ob in s.objects:
 if ob.hide_render:continue
 info['objects'].append({'name':ob.name,'type':ob.type,'vertices':len(ob.data.vertices) if ob.type=='MESH' else None,'dimensions':list(ob.dimensions),'material':[m.name for m in ob.data.materials if m] if hasattr(ob.data,'materials') else [],'parent':ob.parent.name if ob.parent else None,'action':ob.animation_data.action.name if ob.animation_data and ob.animation_data.action else None})
(o/'inspect.json').write_text(json.dumps(info,indent=2));print(json.dumps(info,indent=2))
