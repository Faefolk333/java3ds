import os, pathlib, subprocess, zipfile, hashlib
root=pathlib.Path(__file__).resolve().parents[1]
os.chdir(root)
os.environ['PATH']='C:/devkitPro/msys64/mingw64/bin;'+os.environ['PATH']
p=root/'work/catalog-fixtures';p.mkdir(exist_ok=True)
demo=root/'romfs/demo/LauncherCheck.jar'
def invoke(src,ok=True):
 r=subprocess.run(['work/catalog-probe.exe',src.as_posix(),'work/catalog-test.jar','400','240','auto'],capture_output=True,text=True)
 assert (r.returncode==0)==ok,(src,r.stdout,r.stderr)
 return r
data=demo.read_bytes();(p/'space game.jar').write_bytes(data)
(p/'choice.jad').write_text('MIDlet-Jar-URL: space%20game.jar\r\nMIDlet-Name: JAD title\r\nCheck-Autorun: yes\r\n')
assert 'TITLE JAD title' in invoke(p/'choice.jad').stdout
assert 'Check-Autorun=yes' in (root/'work/host-launch.cfg').read_text()
assert hashlib.sha256(data).digest()==hashlib.sha256((p/'space game.jar').read_bytes()).digest()
with zipfile.ZipFile(root/'work/catalog-test.jar') as z:
 assert z.read('check.txt').startswith(b'RESOURCE_OK')
 assert b'crimson/Resources' in z.read('LauncherCheck.class')
(p/'missing.jad').write_text('MIDlet-Jar-URL: missing.jar\n');invoke(p/'missing.jad',False)
(p/'remote.jad').write_text('MIDlet-Jar-URL: https://example.invalid/space%20game.jar\n');invoke(p/'remote.jad')
(p/'invalid.jar').write_bytes(b'not a zip');invoke(p/'invalid.jar',False)
with zipfile.ZipFile(p/'empty.jar','w') as z:z.writestr('META-INF/MANIFEST.MF','MIDlet-Name: No application\n')
invoke(p/'empty.jar',False)
with zipfile.ZipFile(demo) as src,zipfile.ZipFile(p/'multi.jar','w') as dest:
 for name in src.namelist():
  content=src.read(name)
  if name=='META-INF/MANIFEST.MF':content=content.replace(b'MIDlet-Name: Launcher Check',b'MIDlet-Name: Continued\r\n title')+b'MIDlet-2: Second,,LauncherCheck\r\n'
  dest.writestr(name,content)
r=invoke(p/'multi.jar');assert 'MIDLETS 2' in r.stdout and 'Continuedtitle' in r.stdout,r.stdout
original=pathlib.Path('D:/nintendo 3ds/games/PopClassic_Nokia_E71.jar')
before=hashlib.sha256(original.read_bytes()).digest();assert 'POP 1' in invoke(original).stdout
assert before==hashlib.sha256(original.read_bytes()).digest()
with zipfile.ZipFile(original) as a,zipfile.ZipFile('work/catalog-test.jar') as b,zipfile.ZipFile('romfs/profiles/pop-classic.zip') as profile:
 for name in a.namelist():
  if name.endswith('/'):continue
  if name in profile.namelist():assert b.read(name)==profile.read(name),name
  elif not name.endswith('.class'):assert a.read(name)==b.read(name),name
print('CATALOG_PASS JAR JAD URL_decoding continuation multi_MIDlet invalid_input source_unchanged resource_rewrite PoP_profile')
