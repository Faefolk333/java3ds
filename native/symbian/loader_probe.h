#pragma once
struct SymbianLoaderResult {
 bool parsed=false,relocated=false,returned=false,imports=false,systemCall=false;
 bool protectedMemory=false,bounded=false,cancelled=false;
 bool passed()const{return parsed&&relocated&&returned&&imports&&systemCall&&protectedMemory&&bounded&&cancelled;}
};
SymbianLoaderResult symbianLoaderProbe();
