"""Run using Blender --background --factory-startup --disable-autoexec source.blend --python tools/export_banner.py."""
import bpy, math, json
from pathlib import Path
from mathutils import Matrix
root=Path(__file__).resolve().parents[1];out=root/'work/banner';out.mkdir(parents=True,exist_ok=True)
s=bpy.context.scene;s.frame_set(1)
source_camera=s.camera.matrix_world.copy();vf=s.camera.data.view_frame(scene=s)[0]
tan_y=abs(vf.y/vf.z);scale_z=12.;scale_xy=scale_z*math.tan(math.pi/12)/tan_y
objects=[o for o in s.objects if o.type in ('MESH','CURVE','FONT') and not o.hide_render]
samples={o.name:[] for o in objects}
for frame in range(1,121):
 s.frame_set(frame)
 for o in objects:samples[o.name].append(o.matrix_world.copy())
s.frame_set(1)
# Retain the source transforms beneath a camera-normalization hierarchy.
for o in list(s.objects):
 if o not in objects:bpy.data.objects.remove(o,do_unlink=True)
def empty(name,matrix,parent=None):
 o=bpy.data.objects.new(name,None);s.collection.objects.link(o);o.parent=parent;o.matrix_local=matrix;return o
world=empty('BannerSpace',Matrix.Translation((0,-44.786,1)) @ Matrix.Rotation(math.pi/2,4,'X'))
scale=empty('ProjectionScale',Matrix.Diagonal((scale_xy,scale_xy,scale_z,1)),world)
origin=empty('SourceCamera',source_camera.inverted(),scale)
material=bpy.data.materials.new('Mint chrome');material.use_nodes=True
bs=material.node_tree.nodes.get('Principled BSDF');bs.inputs['Base Color'].default_value=(.33,.88,.74,1);bs.inputs['Metallic'].default_value=.35;bs.inputs['Roughness'].default_value=.32
bs.inputs['Emission Color'].default_value=(.12,.24,.20,1);bs.inputs['Emission Strength'].default_value=1
material.diffuse_color=(.33,.88,.74,1)
summary=[]
for o in objects:
 name=o.name;bpy.ops.object.select_all(action='DESELECT');o.select_set(True);bpy.context.view_layer.objects.active=o
 bpy.ops.object.convert(target='MESH');o=bpy.context.object;o.animation_data_clear()
 old=len(o.data.vertices);target=1700 if name.startswith('Grease') else 1000
 # Weld duplicate geometry before reducing tessellation.
 mod=o.modifiers.new('Compact mesh','WELD');mod.merge_threshold=.0001;bpy.ops.object.modifier_apply(modifier=mod.name)
 if len(o.data.vertices)>target:
  mod=o.modifiers.new('Banner vertex budget','DECIMATE');mod.ratio=target/len(o.data.vertices);bpy.ops.object.modifier_apply(modifier=mod.name)
 mod=o.modifiers.new('Triangles','TRIANGULATE');bpy.ops.object.modifier_apply(modifier=mod.name)
 o.data.materials.clear();o.data.materials.append(material)
 for p in o.data.polygons:p.use_smooth=True
 o.parent=origin;o.matrix_parent_inverse=Matrix.Identity(4);o.rotation_mode='QUATERNION'
 for index,matrix in enumerate(samples[name]):
  o.matrix_basis=matrix
  for field in ('location','rotation_quaternion','scale'):o.keyframe_insert(data_path=field,frame=index)
 o.matrix_basis=samples[name][0]
 for field in ('location','rotation_quaternion','scale'):o.keyframe_insert(data_path=field,frame=120)
 summary.append({'name':name,'vertices_before':old,'vertices':len(o.data.vertices),'triangles':len(o.data.polygons)})
s.frame_start=0;s.frame_end=120;s.render.fps=24;s.frame_set(0)
bpy.ops.object.select_all(action='SELECT')
bpy.ops.export_scene.gltf(filepath=str(out/'banner.glb'),export_format='GLB',use_selection=True,export_animations=True,export_animation_mode='SCENE',export_frame_range=True,export_force_sampling=True,export_sampling_interpolation_fallback='LINEAR',export_materials='EXPORT',export_yup=True)
# Preview the same fixed perspective camera used by HOME Menu.
cam_data=bpy.data.cameras.new('HOME Menu camera');cam=bpy.data.objects.new('HOME Menu camera',cam_data);s.collection.objects.link(cam);cam.matrix_world=world.matrix_world.copy();cam_data.type='PERSP';cam_data.sensor_fit='VERTICAL';cam_data.sensor_height=24;cam_data.lens=12/math.tan(math.pi/12);cam_data.clip_start=26.5;cam_data.clip_end=1000;s.camera=cam
s.world=bpy.data.worlds.new('Banner background');s.world.use_nodes=True;s.world.node_tree.nodes['Background'].inputs[0].default_value=(.08,.085,.10,1)
ld=bpy.data.lights.new('Preview light','AREA');lo=bpy.data.objects.new('Preview light',ld);s.collection.objects.link(lo);lo.location=(0,-30,20);lo.rotation_euler=(math.radians(65),0,0);ld.energy=15000;ld.shape='DISK';ld.size=30
s.render.engine='CYCLES';s.cycles.samples=16;s.render.resolution_x=400;s.render.resolution_y=240;s.render.resolution_percentage=100;s.render.image_settings.file_format='PNG'
for frame in (0,30,60,90,120):
 s.frame_set(frame);s.render.filepath=str(out/f'preview-{frame:03}.png');bpy.ops.render.render(write_still=True)
(out/'geometry.json').write_text(json.dumps({'fps':24,'frames':120,'duration_seconds':5,'objects':summary},indent=2))
bpy.ops.wm.save_as_mainfile(filepath=str(out/'banner-optimized.blend'))
