import bpy,json
from pathlib import Path
s=bpy.context.scene
info={'version':bpy.app.version_string,'scene':s.name,'fps':s.render.fps,'fps_base':s.render.fps_base,'start':s.frame_start,'end':s.frame_end,'resolution':[s.render.resolution_x,s.render.resolution_y],'camera':s.camera.name if s.camera else None,'objects':[],'actions':[]}
for o in s.objects:
 info['objects'].append({'name':o.name,'type':o.type,'location':list(o.location),'rotation':list(o.rotation_euler),'scale':list(o.scale),'dimensions':list(o.dimensions),'vertices':len(o.data.vertices) if o.type=='MESH' else None,'materials':[m.name for m in o.data.materials if m] if hasattr(o.data,'materials') else [],'modifiers':[{'name':m.name,'type':m.type} for m in o.modifiers],'action':o.animation_data.action.name if o.animation_data and o.animation_data.action else None,'hidden':o.hide_render})
for a in bpy.data.actions:
 info['actions'].append({'name':a.name,'range':list(a.frame_range),'curves':[{'path':f.data_path,'index':f.array_index,'keys':len(f.keyframe_points)} for f in getattr(a,'fcurves',[])]})
out=Path(__file__).resolve().parents[1]/'work/banner'
(out/'inspect.json').write_text(json.dumps(info,indent=2))
s.render.resolution_percentage=35;s.render.image_settings.file_format='PNG';s.render.filepath=str(out/'original.png');s.frame_set(s.frame_start)
if s.camera:bpy.ops.render.render(write_still=True)

