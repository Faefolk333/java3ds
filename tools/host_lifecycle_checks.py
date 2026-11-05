"""Exercise platform suspend/resume and cancellation with the real host VM."""
from pathlib import Path
import argparse,json,os,subprocess,time,zipfile
p=argparse.ArgumentParser();p.add_argument('--java-home',default=os.environ.get('JAVA_HOME'));p.add_argument('--host',required=True);p.add_argument('--fixtures',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
a.output.mkdir(parents=True,exist_ok=True);d=Path('work/lifecycle-checks');d.mkdir(exist_ok=True)
subprocess.run([str(Path(a.java_home)/'bin/javac.exe'),'--release','8','-cp','romfs/java/midp.jar;work/minijvm_rt.jar','-d',str(d),'tests/PauseCheck.java'],check=True)
jar=d/'pause.jar'
with zipfile.ZipFile(jar,'w') as z:z.write(d/'PauseCheck.class','PauseCheck.class')
env=os.environ.copy();env['PATH']='C:/devkitPro/msys64/mingw64/bin;'+env['PATH'];env.update(JAVA_TEST_RUNS='1',JAVA_RUN_MS='3000',JAVA_ABORT_MS='3500',JAVA_HOME_TEST='1')
results=[]
for phase in ('1','2'):
 env['JAVA_PAUSE_TEST']=phase;start=time.monotonic();log=a.output/('home-suspend-'+phase+'.log')
 with log.open('w') as f:r=subprocess.run([a.host,'work/minijvm_rt.jar','romfs/java/midp.jar;'+str(jar),'PauseCheck','work/host-launch.cfg','320','240'],env=env,stdout=f,stderr=subprocess.STDOUT,timeout=12)
 text=log.read_text();assert r.returncode in (0,2) and 'PAUSE_TEST PASS' in text and 'STAGE 72' in text,(phase,r.returncode,text)
 results.append({'case':'HOME suspend '+phase,'exit':r.returncode,'wall_ms':round((time.monotonic()-start)*1000)});print(results[-1],flush=True)
for k in ('JAVA_HOME_TEST','JAVA_PAUSE_TEST'):env.pop(k,None)
env.update(JAVA_TEST_RUNS='3',JAVA_TEST_CONTINUE_ABORT='1',JAVA_RUN_MS='20000',JAVA_ABORT_MS='3000')
for name,w,h in (('gladiator',320,240),('dawnstar',176,208),('elder-travels',176,208),('devil',320,240),('lotr-other',240,320)):
 start=time.monotonic();log=a.output/(name+'-cancel-restart.log')
 with log.open('w') as f:r=subprocess.run([a.host,'work/minijvm_rt.jar','romfs/java/midp.jar;'+str((a.fixtures/(name+'.jar')).resolve()),'crimson.GameMain',str((a.fixtures/(name+'.cfg')).resolve()),str(w),str(h),'auto'],env=env,stdout=f,stderr=subprocess.STDOUT,timeout=25)
 text=log.read_text();assert r.returncode==2 and text.count('result=2')==3 and text.count('STAGE 72')==3,(name,r.returncode,text[-3000:])
 results.append({'case':name+' cancel/restart','runs':3,'exit':r.returncode,'wall_ms':round((time.monotonic()-start)*1000)});print(results[-1],flush=True)
(a.output/'host-lifecycle.json').write_text(json.dumps(results,indent=2)+'\n')
