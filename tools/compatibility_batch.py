"""Sequential host smoke tests; each game keeps its own log and frame snapshot."""
from pathlib import Path
import os,subprocess,json,re,shutil,argparse
p=argparse.ArgumentParser();p.add_argument('folder',type=Path);p.add_argument('--exe',default='work/java-host-next.exe');p.add_argument('--limit',type=int,default=40);p.add_argument('--ms',type=int,default=7000);p.add_argument('--start',type=int,default=0);p.add_argument('--out',default='work/batch-v033b');args=p.parse_args()
r=Path(__file__).resolve().parents[1];os.chdir(r);out=r/args.out;out.mkdir(exist_ok=True)
shutil.copy2(r/'romfs/java/midp.jar',out/'midp-snapshot.jar')
env=os.environ.copy();env['PATH']='C:/devkitPro/msys64/mingw64/bin;'+env['PATH'];env.update(JAVA_RUN_MS=str(args.ms),JAVA_ABORT_MS=str(args.ms+1500),JAVA_TEST_RUNS='1',JAVA_CAPTURE_EVERY='30');env.pop('JAVA_INPUT',None)
priority=('Age of Heroes III','loco_roco','The Dark Knight Rises','Worms 2011','Discworld','Blades and Magic','blades_and','Gladiator 3D','Outland 3D')
files=sorted(args.folder.glob('*.jar'));files.sort(key=lambda f:next((i for i,x in enumerate(priority) if f.name.startswith(x)),len(priority)))
results=[]
for index,jar in enumerate(files[args.start:args.limit],args.start):
 d=out/f'{index:02}';d.mkdir(exist_ok=True);target=d/'active.jar';entry={'index':index,'game':jar.name}
 prepare=subprocess.run([str(r/'work/catalog-probe.exe'),str(jar),str(target)],env=env,capture_output=True,text=True);entry['inspect']=prepare.stdout.strip()
 if prepare.returncode:entry['status']='prepare-failed';entry['error']=prepare.stderr
 elif 'Requires ' in prepare.stdout:entry['status']='unsupported-3d'
 else:
  m=re.search(r'NATIVE (\d+)x(\d+)',prepare.stdout);w,h=m.groups() if m else ('320','240')
  subprocess.run([str(r/'work/catalog-probe.exe'),str(jar),str(target),w,h],env=env,capture_output=True)
  config=d/'launch.cfg';shutil.copy2(r/'work/host-launch.cfg',config)
  frame=r/'work/host-frame.ppm'
  if frame.exists():frame.unlink()
  with (d/'runtime.log').open('w') as log:
   try:
    run=subprocess.run([str(r/args.exe),'work/minijvm_rt.jar',str(out/'midp-snapshot.jar')+';'+str(target),'crimson.GameMain',str(config),w,h,'auto'],env=env,stdout=log,stderr=subprocess.STDOUT,timeout=(args.ms+6500)/1000)
    entry['status']='returned' if run.returncode==0 else 'crash';entry['exit']=run.returncode
   except subprocess.TimeoutExpired:entry['status']='timeout'
  text=(d/'runtime.log').read_text(errors='replace');entry['vm_exit']=re.findall(r'VM_EXIT[^\n]+',text);entry['missing']=re.findall(r'(?:method not exist|native.*not found|java\.\w+\.\w+Exception)[^\n]*',text)[:20]
  if frame.exists():shutil.copy2(frame,d/'frame.ppm')
 results.append(entry);(out/'results.json').write_text(json.dumps(results,indent=2,ensure_ascii=False),encoding='utf8');print(index,entry['status'],jar.name,flush=True)
