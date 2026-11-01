from pathlib import Path
import hashlib, shutil, zipfile

root=Path(__file__).resolve().parents[1]
out=root/'dist';out.mkdir(exist_ok=True)
target=Path('D:/nintendo 3ds/games/Java3DS-0.3.5')
target.mkdir(parents=True,exist_ok=True)
install=out/'Java3DS-0.3.5-private.zip'
licenses=[root/'THIRD-PARTY.md']
for folder in ('minijvm','TinySoundFont','GeneralUser-GS','curl','mbedtls','zlib','J2ME-Loader-API','FreeJ2ME-M3G'):
 licenses.extend(p for p in (root/'third-party'/folder).glob('LICENSE*') if p.is_file())
with zipfile.ZipFile(install,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for ext in ('3dsx','smdh'):
  z.write(root/f'work/arm/java_pocket.{ext}',f'3ds/java-3ds/java_pocket.{ext}')
 z.write(root/'README.md','README.md')
 z.write(root/'README-cs.md','CTI-ME.md')
 z.write(root/'VALIDATION.md','VALIDATION.md')
 z.write(root/'UPDATES.md','UPDATES.md')
 z.write(root/'CIA.md','CIA.md')
 z.write(root/'COMPATIBILITY.md','COMPATIBILITY.md')
 for p in (root/'validation/0.3.5').rglob('*'):
  if p.is_file():z.write(p,'validation/0.3.5/'+p.relative_to(root/'validation/0.3.5').as_posix())
 z.write(root/'work/arm/java3ds.cia','cias/Java3DS.cia')
 z.writestr('3ds/java-3ds/games/ADD-YOUR-GAMES-HERE.txt','Add your JAR/JAD games here. A JAD also needs its matching local JAR.\n')
 for p in licenses:z.write(p,'licenses/'+p.relative_to(root).as_posix())
 for folder in ('J2ME-Loader-API','FreeJ2ME-M3G'):z.write(root/'third-party'/folder/'NOTICE','licenses/'+folder+'/NOTICE')
 z.write(root/'third-party/stb_image.h','licenses/stb_image.h')
 z.write(root/'third-party/font8x8/font8x8_basic.h','licenses/font8x8_basic.h')
source=out/'Java3DS-0.3.5-source-private.zip'
with zipfile.ZipFile(source,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for folder in ('native','java','tools','tests','third-party','romfs','assets','validation'):
  for p in (root/folder).rglob('*'):
   if p.is_file() and '__pycache__' not in p.parts and p.name!='github-token.txt' and p.suffix not in ('.o','.exe','.pyc','.blend1','.blend2'):
    z.write(p,'java-3ds-launcher/'+p.relative_to(root).as_posix())
 for p in root.iterdir():
  if p.is_file() and p.suffix in ('.md','.ps1','.sh'):z.write(p,'java-3ds-launcher/'+p.name)
 z.write(root/'.gitignore','java-3ds-launcher/.gitignore')
 z.write(root/'.gitattributes','java-3ds-launcher/.gitattributes')
 for name in ('minijvm_rt.jar','daemon-v03.log','directgraphics-v03.log','controls-v03.log','soul-v03.log','soul-title-v03.log','mm2-game-v03.log','stop-loops-v03.log','stop-wait-v03.log','stop-sleep-v03.log','demo-v03.log','pop-v03.log','ui-v03.png','gm-test.log','device-test.log'):
  z.write(root/'work'/name,'java-3ds-launcher/work/'+name)

checks=[]
raw=out/"java3ds.3dsx"
shutil.copy2(root/"work/arm/java_pocket.3dsx",raw)
cia=out/"java3ds.cia"
shutil.copy2(root/"work/arm/java3ds.cia",cia)
for p in (install,source,raw,cia):
 if p.suffix==".zip":
  with zipfile.ZipFile(p) as z:assert z.testzip() is None
 shutil.copy2(p,target/p.name)
 assert hashlib.sha256(p.read_bytes()).digest()==hashlib.sha256((target/p.name).read_bytes()).digest()
 checks.append(hashlib.sha256(p.read_bytes()).hexdigest()+'  '+p.name)
(target/'SHA256SUMS.txt').write_text('\n'.join(checks)+'\n')
shutil.copy2(root/'README-cs.md',target/'CTI-ME.md')
print(target)
print('\n'.join(checks))
