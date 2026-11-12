#pragma once
struct SymbianKernelResult {
 bool chunks=false,handles=false,sync=false,heap=false,abi=false,thumb=false;
 bool passed()const{return chunks&&handles&&sync&&heap&&abi&&thumb;}
};
SymbianKernelResult symbianKernelProbe();
