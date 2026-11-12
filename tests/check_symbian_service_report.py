"""Assert the native EPOC6 service diagnostic reached its declared boundary.

The loader CLI can successfully produce a report describing incomplete startup;
exit code zero by itself therefore does not validate any native service.
"""
import argparse,json,re
from pathlib import Path

p=argparse.ArgumentParser();p.add_argument('report',type=Path);p.add_argument('--output',type=Path);args=p.parse_args()
text=args.report.read_text(encoding='utf-8')
assert 'DIAGNOSTIC_STOP' not in text and 'DIAGNOSTIC_CANCELLED' not in text
def record(prefix):
 lines=[line for line in text.splitlines() if line.startswith(prefix+' ')]
 assert len(lines)==1,(prefix,len(lines))
 return dict(re.findall(r'(\w+)=([^\s]+)',lines[0])),lines[0]
for prefix,fields in {
 'DEPENDENCY_GRAPH':('ready',),
 'ROM_INITIALIZATION':('ready',),
 'NATIVE_TRAP':('normal','leave','nested','cleanup','heap_reuse'),
 'NATIVE_CONTEXT':('tls','scheduler','events','run_error','teardown'),
 'NATIVE_FILES':('connect','path','open','read','seek','eof','missing','close','rom','entry'),
 'NATIVE_FBS':('connect','shared','retained','close'),
 'APPLICATION_UID':('matches',),
}.items():
 data,_=record(prefix)
 for field in fields:assert data.get(field)=='1',(prefix,field,data)
_,run=record('ROM_APPRUN');assert 'result=unimplemented system call' in run and 'svc=00c000bf' in run and 'fault=00000000' in run
window,_=record('WINDOWSERVER');assert window['initializations']=='1' and window['operation']=='0' and window['command']=='25'
files,_=record('FILESERVER');assert int(files['opens'])>=2 and int(files['reads'])>=4 and int(files['bytes'])>=28 and files['unsupported']=='0'
record('DIAGNOSTIC_END')
result={'native_files':True,'native_FBS_connection':True,'WindowServer_initialized':True,'next_missing_command':'CreateScreenDevice (25)','no_phone_GUI_or_gameplay':True}
if args.output:args.output.write_text(json.dumps(result,indent=2)+'\n')
print('NATIVE_SERVICE_REPORT_PASS',json.dumps(result))
