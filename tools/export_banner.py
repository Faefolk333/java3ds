"""Bake model transforms into the fixed HOME Menu camera; verify source camera motion."""
import bpy,math,json
from pathlib import Path
from mathutils import Matrix
root=Path(__file__).resolve().parents[1];out=root/'work/banner-v034';out.mkdir(exist_ok=True)
s=bpy.context.scene;start,end=s.frame_start,s.frame_end;fps=s.render.fps/s.render.fps_base;duration=(end-start+1)/fps
# The user's Desktop scene remains untouched; a compact derivative is saved separately.

objects=[o for o in s.objects if o.type in ('MESH','CURVE','FONT') and not o.hide_render];samples={o.name:[] for o in objects};camera=s.camera
camera_reference=None;camera_delta=0.0
home=Matrix.Translation((0,-44.786,1))@Matrix.Rotation(math.pi/2,4,'X')
for frame in range(start,end+1):
 s.frame_set(frame);current=[v for row in camera.matrix_world for v in row]
 if camera_reference is None:camera_reference=current
 camera_delta=max(camera_delta,max(abs(a-b) for a,b in zip(current,camera_reference)))
 vf=camera.data.view_frame(scene=s)[0];tanY=abs(vf.y/vf.z);sz=12.;sx=.86*sz*math.tan(math.pi/12)/tanY
 cameraBake=home@Matrix.Diagonal((sx,sx,sz,1))@camera.matrix_world.inverted()
 for o in objects:samples[o.name].append(cameraBake@o.matrix_world)
s.frame_set(start)
for o in list(s.objects):
 if o not in objects:bpy.data.objects.remove(o,do_unlink=True)
summary=[]
for o in objects:
 name=o.name;glass=any('wrap' in m.name.lower() for m in o.data.materials if m)
 bpy.ops.object.select_all(action='DESELECT');o.select_set(True);bpy.context.view_layer.objects.active=o;bpy.ops.object.convert(target='MESH');o=bpy.context.object;o.animation_data_clear();before=len(o.data.vertices)
 mod=o.modifiers.new('Weld duplicates','WELD');mod.merge_threshold=.0001;bpy.ops.object.modifier_apply(modifier=mod.name)
 target=720 if name.startswith('Grease') else 680
 if len(o.data.vertices)>target:
  mod=o.modifiers.new('HOME Menu budget','DECIMATE');mod.ratio=target/len(o.data.vertices);bpy.ops.object.modifier_apply(modifier=mod.name)
 mod=o.modifiers.new('Triangles','TRIANGULATE');bpy.ops.object.modifier_apply(modifier=mod.name)
 m=bpy.data.materials.new('Glass shell' if glass else 'Mint glass core');m.use_nodes=True;bs=m.node_tree.nodes.get('Principled BSDF');color=(.48,.94,.83,.32) if glass else (.33,.85,.68,1)
 bs.inputs['Base Color'].default_value=color;bs.inputs['Alpha'].default_value=color[3];bs.inputs['Metallic'].default_value=.12 if glass else .32;bs.inputs['Roughness'].default_value=.08 if glass else .16;bs.inputs['IOR'].default_value=1.46;bs.inputs['Transmission Weight'].default_value=.65 if glass else .15;bs.inputs['Coat Weight'].default_value=1;bs.inputs['Coat Roughness'].default_value=.04;m.diffuse_color=color;m.surface_render_method='DITHERED'
 if glass:
  center=sum((v.co for v in o.data.vertices),__import__('mathutils').Vector())/len(o.data.vertices)
  for v in o.data.vertices:v.co=center+(v.co-center)*1.025
 o.data.materials.clear();o.data.materials.append(m)
 for p in o.data.polygons:p.use_smooth=True
 o.parent=None;o.matrix_parent_inverse=Matrix.Identity(4);o.rotation_mode='QUATERNION'
 # Flattened animated camera-space transforms avoid animated-parent converter drift.
 for i,matrix in enumerate(samples[name]+[samples[name][0]]):
  o.matrix_basis=matrix
  for field in ('location','rotation_quaternion','scale'):o.keyframe_insert(data_path=field,frame=i)
 summary.append({'name':name,'glass':glass,'vertices_before':before,'vertices':len(o.data.vertices),'triangles':len(o.data.polygons)})
s.frame_start=0;s.frame_end=end-start+1;s.render.fps=int(fps);s.frame_set(0);bpy.ops.object.select_all(action='SELECT')
bpy.ops.export_scene.gltf(filepath=str(out/'banner.glb'),export_format='GLB',use_selection=True,export_animations=True,export_animation_mode='SCENE',export_frame_range=True,export_force_sampling=True,export_sampling_interpolation_fallback='LINEAR',export_materials='EXPORT',export_yup=True)
# Approximation preview uses the same fixed camera; actual PICA glass uses alpha/specular.
camData=bpy.data.cameras.new('HOME Menu fixed camera');cam=bpy.data.objects.new('HOME Menu fixed camera',camData);s.collection.objects.link(cam);cam.matrix_world=home;camData.sensor_fit='VERTICAL';camData.sensor_height=24;camData.lens=12/math.tan(math.pi/12);camData.clip_start=26.5;camData.clip_end=1000;s.camera=cam
s.world=bpy.data.worlds.new('Preview studio');s.world.use_nodes=True;s.world.node_tree.nodes['Background'].inputs[0].default_value=(.13,.15,.18,1)
for name,loc,energy,size in [('Softbox',(1,-30,15),18000,18),('Rim',(-12,-34,5),12000,10)]:
 data=bpy.data.lights.new(name,'AREA');ob=bpy.data.objects.new(name,data);s.collection.objects.link(ob);ob.location=loc;ob.rotation_euler=(math.radians(65),0,0);data.energy=energy;data.size=size
s.render.engine='CYCLES';s.cycles.samples=64;s.render.resolution_x=400;s.render.resolution_y=240;s.render.resolution_percentage=100;s.render.image_settings.file_format='PNG'
(out/'geometry.json').write_text(json.dumps({'fps':fps,'frames':s.frame_end,'duration_seconds':duration,'source_camera_max_delta':camera_delta,'camera_motion_detected':camera_delta>1e-6,'camera_space_baked':True,'objects':summary},indent=2))
for frame in (0,30,60,90,120):
 s.frame_set(frame);s.render.filepath=str(out/f'preview-{frame:03}.png');bpy.ops.render.render(write_still=True)
bpy.data.orphans_purge(do_local_ids=True,do_linked_ids=True,do_recursive=True)
# tools/save_banner_source.py imports the compact GLB into a fresh scene.
print('BANNER_BAKE_COMPLETE',json.dumps(summary))
