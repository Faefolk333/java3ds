#pragma once
#include "address_space.h"
#include <string>
#include <vector>
namespace symbian {
struct RomLibrary {
 std::uint32_t uid3=0,codeBase=0,codeSize=0;
 std::vector<std::uint32_t> exports;
};
// Initial EKA1-only reader: mounts an uncompressed SYM.ROM at its real guest
// address. Library descriptors must match that exact ROM, not another phone.
class RomImage {
 std::uint32_t base=0,size=0;
public:
 bool mount(AddressSpace&memory,const char*path,std::string&error,bool(*keepRunning)()=nullptr);
 bool library(AddressSpace&memory,const char*path,RomLibrary&library,std::string&error)const;
 bool mounted()const{return size!=0;}
 std::uint32_t bytes()const{return size;}
};
bool e32LibraryIdentity(const std::string&name,std::string&filename,std::uint32_t&uid);
}
