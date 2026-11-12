#pragma once
#include "address_space.h"
#include "task_control.h"
#include <string>
#include <vector>
namespace symbian {
struct RomLibrary {
 std::uint32_t uid3=0,codeBase=0,codeSize=0;
 std::uint32_t entry=0,references=0,dataSize=0,bssSize=0;
 std::vector<std::uint32_t> exports;
};
struct RomInitEntry {std::uint32_t uid=0,entry=0,references=0;};
struct RomFile {std::uint32_t address=0,size=0;};
// Initial EKA1-only reader: mounts an uncompressed SYM.ROM at its real guest
// address. Library descriptors must match that exact ROM, not another phone.
class RomImage {
 std::uint32_t base=0,size=0;
public:
 bool mount(AddressSpace&memory,const char*path,std::string&error,TaskControl*control=nullptr);
 bool library(AddressSpace&memory,const char*path,RomLibrary&library,std::string&error)const;
 bool initializationOrder(AddressSpace&memory,const std::vector<RomInitEntry>&roots,std::vector<RomInitEntry>&order,std::string&error,TaskControl*control=nullptr)const;
 bool findFile(AddressSpace&memory,const std::string&path,RomFile&file,std::string&error,TaskControl*control=nullptr)const;
 bool mounted()const{return size!=0;}
 std::uint32_t bytes()const{return size;}
};
bool e32LibraryIdentity(const std::string&name,std::string&filename,std::uint32_t&uid);
}
