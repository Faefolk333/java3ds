#pragma once
#include "address_space.h"
#include "task_control.h"
#include <string>
#include <vector>

namespace symbian {
struct E32Import {std::string library;std::uint32_t ordinal=0,codeOffset=0;};
struct E32Relocation {std::uint32_t offset=0;unsigned type=0;bool data=false;};
struct E32Header {
 std::uint32_t uid1=0,uid2=0,uid3=0,flags=0;
 std::uint32_t codeSize=0,dataSize=0,bssSize=0,codeBase=0,dataBase=0;
 std::uint32_t codeOffset=0,dataOffset=0,textSize=0,entry=0;
 std::uint32_t heapMin=0,heapMax=0,stack=0;
 std::uint32_t exportsOffset=0,exportsCount=0;
};
struct LoadedE32 {
 std::uint32_t codeBase=0,dataBase=0,entry=0;
 std::vector<std::uint32_t> exports;
 std::vector<E32Import> imports;
};
// Initial loader: uncompressed ARM EKA1 PE-import E32 files. Unsupported EKA2,
// compressed and SIS containers return errors; they are never executed as EKA1.
class E32Image {
 std::vector<std::uint8_t> bytes;
 E32Header info;
 std::vector<E32Import> imports;
 std::vector<E32Relocation> relocations;
 bool ready=false;
public:
 static constexpr std::size_t maxFileBytes=8*1024*1024;
 bool parse(std::vector<std::uint8_t> input,std::string&error);
 bool open(const char*path,std::string&error,TaskControl*control=nullptr);
 bool load(AddressSpace&memory,std::uint32_t codeBase,std::uint32_t dataBase,LoadedE32&loaded,std::string&error)const;
 const E32Header&header()const{return info;}
 std::size_t importCount()const{return imports.size();}
 std::size_t relocationCount()const{return relocations.size();}
};
}
