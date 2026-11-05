"""Check actual ARM startup call targets; declarations alone do not wrap startup."""
from pathlib import Path
import argparse,re,subprocess
p=argparse.ArgumentParser(description=__doc__);p.add_argument('elf',type=Path,nargs='+');a=p.parse_args()
objdump='C:/devkitPro/msys64/opt/devkitpro/devkitARM/bin/arm-none-eabi-objdump.exe'
for elf in a.elf:
 text=subprocess.check_output([objdump,'-d',str(elf)],text=True)
 caller='';wrapped=[];direct=[]
 for line in text.splitlines():
  symbol=re.match(r'^[0-9a-f]+ <([^>]+)>:',line)
  if symbol:caller=symbol.group(1)
  call=re.search(r'\b(?:bl|b)\s+[0-9a-f]+ <(__wrap___system_allocateHeaps|__system_allocateHeaps)>',line)
  if call:(wrapped if call.group(1).startswith('__wrap') else direct).append(caller)
 assert '__libctru_init' in wrapped,(elf,'startup bypasses bounded heap wrapper',wrapped)
 assert direct and set(direct)=={'__wrap___system_allocateHeaps'},(elf,'unbounded direct allocation call',direct)
 print('STARTUP_HEAP_LINK_PASS',elf.name,'CRT -> bounded wrapper -> SDK allocator')
