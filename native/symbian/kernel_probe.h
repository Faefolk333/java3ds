#pragma once
struct SymbianKernelResult {
 bool chunks=false,handles=false,sync=false,heap=false,abi=false,thumb=false,trap=false,frames=false,tls=false,scheduler=false,namedSync=false,lifecycle=false;
 bool passed()const{return chunks&&handles&&sync&&heap&&abi&&thumb&&trap&&frames&&tls&&scheduler&&namedSync&&lifecycle;}
};
SymbianKernelResult symbianKernelProbe();
