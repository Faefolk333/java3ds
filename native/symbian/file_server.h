#pragma once
#include "eka1_kernel.h"
#include "rom_image.h"
#include <cstdio>
namespace symbian {
// Read-only, explicitly mounted namespace: Z: from the validated ROM and one
// E: application file. No guest path is passed to the host filesystem.
class FileServer {
 AddressSpace&memory;RomImage&rom;
 struct Session {std::uint32_t handle=0;std::u16string path=u"C:\\";bool notify=true;};
 struct File {std::uint32_t handle=0,owner=0,position=0,size=0,address=0;std::FILE*stream=nullptr;std::u16string path;};
 std::array<Session,8> sessions{};std::array<File,32> files{};
 std::uint32_t nextFile=1;std::string application;
 bool normalize(const std::u16string&base,const std::u16string&name,std::u16string&out)const;
 int open(const std::u16string&path,File&file);
 void close(File&file);
public:
 explicit FileServer(AddressSpace&m,RomImage&r):memory(m),rom(r){}
 ~FileServer();
 bool mountApplication(const std::string&path);
 KernelResult request(std::uint32_t session,std::int32_t operation,const std::array<std::uint32_t,4>&args);
 struct Trace {std::uint32_t calls=0,opens=0,reads=0,bytes=0;std::int32_t operation=-1,result=0;std::u16string path;bool unsupported=false;} trace;
};
}
