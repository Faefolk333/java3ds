import zipfile,struct,sys
out=set()
with zipfile.ZipFile(sys.argv[1]) as z:
 for n in z.namelist():
  if not n.endswith('.class'):continue
  b=z.read(n);i=1;p=10;cp=[None]*struct.unpack_from('>H',b,8)[0]
  while i<len(cp):
   t=b[p];p+=1
   if t==1:k=struct.unpack_from('>H',b,p)[0];p+=2;cp[i]=b[p:p+k].decode(errors='replace');p+=k
   elif t in (7,8,16):cp[i]=(t,struct.unpack_from('>H',b,p)[0]);p+=2
   elif t in (9,10,11,12,18):cp[i]=(t,*struct.unpack_from('>HH',b,p));p+=4
   elif t in (3,4):p+=4
   elif t in (5,6):p+=8;i+=1
   elif t==15:p+=3
   else:raise ValueError(t)
   i+=1
  for c in cp:
   if isinstance(c,tuple) and c[0] in (10,11):
    cl=cp[cp[c[1]][1]];nt=cp[c[2]]
    if cl.startswith(('javax/','com/')):out.add(cl+'.'+cp[nt[1]]+cp[nt[2]])
print('\n'.join(sorted(out)))
