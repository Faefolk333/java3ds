from pathlib import Path
import os,subprocess,zipfile
import argparse,shutil,struct,zlib
p=argparse.ArgumentParser();p.add_argument("--java-home",default=os.environ.get("JAVA_HOME"));p.add_argument("--host",default="work/java-host.exe");p.add_argument("--m3g-jar",type=Path);args=p.parse_args()

os.environ['PATH']='C:/devkitPro/msys64/mingw64/bin;'+os.environ['PATH']
r=Path(__file__).resolve().parents[1];os.chdir(r);d=r/'work/runtime-checks';d.mkdir(exist_ok=True)
jdk=Path(args.java_home)/'bin' if args.java_home else Path(shutil.which('javac') or '').parent
if not (jdk/'javac.exe').exists():raise SystemExit('Set JAVA_HOME or --java-home to a JDK (Windows host checks).')
names=['RenderCompatibility','MotionCompatibility','M3GCompatibility','M3GSpanCheck','MicroPoseCheck','MicroBatchCheck','MicroBlendCheck','ArcCheck','GameCanvasFlushCheck','MediaPlayerCheck','RmsCompatibility','ControlsCheck','DirectGraphicsCheck','ShutdownReentry']
if args.m3g_jar:names.append('M3GAssetProbe')
subprocess.run([str(jdk/'javac.exe'),'--release','8','-cp','romfs/java/midp.jar;work/minijvm_rt.jar','-d',str(d)]+[f'tests/{n}.java' for n in names],check=True)
def chunk(kind,data):return struct.pack('>I',len(data))+kind+data+struct.pack('>I',zlib.crc32(kind+data)&0xffffffff)
scan=b'\0'+bytes([0x12,0xab,0x34,255])*4096
(d/'wide-atlas.png').write_bytes(b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',4096,2,8,6,0,0,0))+chunk(b'IDAT',zlib.compress(scan*2))+chunk(b'IEND',b''))
if args.m3g_jar:
 with zipfile.ZipFile(args.m3g_jar) as z:(d/'prize.m3g').write_bytes(z.read('prize.m3g'))
(d/'MANIFEST.MF').write_text('Manifest-Version: 1.0\nMIDlet-Name: Compatibility probes\nMIDlet-1: Probe,,MotionCompatibility\n')
subprocess.run([str(jdk/'jar.exe'),'cfm','work/runtime-checks-raw.jar',str(d/'MANIFEST.MF'),'-C',str(d),'.'],check=True)
subprocess.run(['work/catalog-probe.exe','work/runtime-checks-raw.jar','work/runtime-checks.jar','320','240'],check=True)
with zipfile.ZipFile('work/runtime-checks.jar') as z:assert b'crimson/ConnectionFactory' in z.read('MotionCompatibility.class')
env=os.environ.copy();env['PATH']='C:/devkitPro/msys64/mingw64/bin;'+env['PATH'];env.update(JAVA_RUN_MS='5000',JAVA_ABORT_MS='7000',JAVA_TEST_RUNS='2')
for n in names:
 main='javax.microedition.lcdui.'+n if n in ('RenderCompatibility','M3GCompatibility') else n
 with open(f'work/runtime-check-{n}.log','w') as f:
  p=subprocess.run([args.host,'work/minijvm_rt.jar','romfs/java/midp.jar;work/runtime-checks.jar',main,'work/host-launch.cfg','320','240'],stdout=f,stderr=subprocess.STDOUT,env=env,timeout=25)
 t=Path(f'work/runtime-check-{n}.log').read_text(errors='replace')
 assert p.returncode==0 and t.count('result=0')==2,(n,p.returncode,t)
 print(n,'PASS',flush=True)
