import bpy,math
from pathlib import Path
from mathutils import Matrix
r=Path(__file__).resolve().parents[1];bpy.ops.wm.read_factory_settings(use_empty=True);bpy.ops.import_scene.gltf(filepath=str(r/'assets/banner/banner.glb'))
s=bpy.context.scene;s.frame_start=0;s.frame_end=120;s.render.fps=24
camData=bpy.data.cameras.new('HOME Menu fixed camera');cam=bpy.data.objects.new('HOME Menu fixed camera',camData);s.collection.objects.link(cam);cam.matrix_world=Matrix.Translation((0,-44.786,1))@Matrix.Rotation(math.pi/2,4,'X');camData.sensor_fit='VERTICAL';camData.sensor_height=24;camData.lens=12/math.tan(math.pi/12);camData.clip_start=26.5;camData.clip_end=1000;s.camera=cam;s.render.resolution_x=400;s.render.resolution_y=240;s.frame_set(0)
bpy.ops.wm.save_as_mainfile(filepath=str(r/'assets/banner/source.blend'),compress=True)
