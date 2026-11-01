"""Add/update a small visible build marker without rebaking the model animation."""
from pathlib import Path
import argparse, io, json, struct
from PIL import Image, ImageDraw, ImageFont

root = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('version')
args = p.parse_args()
path = root/'assets/banner/banner.glb'
raw = path.read_bytes()
size = struct.unpack_from('<I', raw, 12)[0]
doc = json.loads(raw[20:20+size])
pos = 20 + size
length, kind = struct.unpack_from('<II', raw, pos)
assert kind == 0x004e4942
binary = bytearray(raw[pos+8:pos+8+length])

def view(data, target=None):
    binary.extend(b'\0' * (-len(binary) % 4))
    offset = len(binary);binary.extend(data)
    v = {'buffer':0,'byteOffset':offset,'byteLength':len(data)}
    if target:v['target']=target
    doc.setdefault('bufferViews',[]).append(v)
    return len(doc['bufferViews'])-1

def accessor(data, count, components, minimum=None, maximum=None):
    index = view(struct.pack('<'+'f'*len(data), *data), 34962)
    a={'bufferView':index,'componentType':5126,'count':count,'type':components}
    if minimum is not None:a['min']=minimum;a['max']=maximum
    doc.setdefault('accessors',[]).append(a)
    return len(doc['accessors'])-1

image=Image.new('RGBA',(192,32),(15,36,45,255));draw=ImageDraw.Draw(image)
font=ImageFont.truetype('C:/Windows/Fonts/consolab.ttf',20)
draw.rectangle((0,0,191,31),outline=(110,244,216),width=2)
draw.text((96,16),'BUILD '+args.version,font=font,anchor='mm',fill=(190,255,235))
stream=io.BytesIO();image.save(stream,format='PNG');png=stream.getvalue()
(root/'assets/banner/build-marker.png').write_bytes(png)
existing=next((n for n in doc['nodes'] if n.get('name')=='Build marker'),None)
if existing:
    primitive=doc['meshes'][existing['mesh']]['primitives'][0]
    texture=doc['materials'][primitive['material']]['pbrMetallicRoughness']['baseColorTexture']['index']
    doc['images'][doc['textures'][texture]['source']]['bufferView']=view(png)
else:
    image_index=len(doc.setdefault('images',[]));doc['images'].append({'bufferView':view(png),'mimeType':'image/png'})
    texture=len(doc.setdefault('textures',[]));doc['textures'].append({'source':image_index})
    material=len(doc['materials']);doc['materials'].append({'name':'Build marker','doubleSided':True,'pbrMetallicRoughness':{'baseColorTexture':{'index':texture},'metallicFactor':0,'roughnessFactor':1}})
    positions=accessor([-4,-9,2,4,-9,2,4,-7.7,2,-4,-7.7,2],4,'VEC3',[-4,-9,2],[4,-7.7,2])
    normals=accessor([0,0,1]*4,4,'VEC3')
    uv=accessor([0,0,1,0,1,1,0,1],4,'VEC2')
    doc['accessors'].append({'bufferView':view(struct.pack('<6H',0,1,2,0,2,3),34963),'componentType':5123,'count':6,'type':'SCALAR'})
    mesh=len(doc['meshes']);doc['meshes'].append({'name':'Build marker','primitives':[{'attributes':{'POSITION':positions,'NORMAL':normals,'TEXCOORD_0':uv},'indices':len(doc['accessors'])-1,'material':material}]})
    node=len(doc['nodes']);doc['nodes'].append({'name':'Build marker','mesh':mesh});doc['scenes'][doc.get('scene',0)]['nodes'].append(node)
doc['buffers'][0]['byteLength']=len(binary)
encoded=json.dumps(doc,separators=(',',':')).encode();encoded+=b' '*(-len(encoded)%4);binary+=b'\0'*(-len(binary)%4)
path.write_bytes(struct.pack('<4sII',b'glTF',2,28+len(encoded)+len(binary))+struct.pack('<II',len(encoded),0x4e4f534a)+encoded+struct.pack('<II',len(binary),0x004e4942)+binary)
info_path=root/'assets/banner/geometry.json';info=json.loads(info_path.read_text());info['visible_build_marker']=args.version;info_path.write_text(json.dumps(info,indent=2)+'\n',encoding='utf-8')
print('Banner stamped',args.version,'animation channels preserved')
