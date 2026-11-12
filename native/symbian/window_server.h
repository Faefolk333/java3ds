#pragma once
#include "eka1_kernel.h"
#include "guest_descriptor.h"
#include "rom_image.h"
#include <vector>
namespace symbian {
// One virtual EKA1 screen with bounded screen-device objects. No windows,
// drawing or asynchronous event source is claimed by a screen query.
class WindowServer {
 AddressSpace&memory;
 struct Session {std::uint32_t handle=0,lastObject=0;bool initialized=false;};
 struct Object {enum {Screen=1,Group=2};std::uint32_t handle=0,owner=0;int enforcement=0;unsigned type=0;std::uint32_t client=0,parent=0;bool focus=false,defaultOwner=false;std::u16string name;};
 std::array<Session,8> sessions{};std::array<Object,32> objects{};
 std::uint32_t nextObject=0x10001;RomDisplay display{};int displayMode=0;
 KernelResult commands(Session&session,const std::array<std::uint32_t,4>&args);
public:
 explicit WindowServer(AddressSpace&m):memory(m){}
 bool configure(const RomDisplay&value);
 KernelResult request(std::uint32_t handle,std::int32_t operation,const std::array<std::uint32_t,4>&args);
 std::uint32_t calls=0,initializations=0,lastCommand=0,lastObject=0,created=0,queries=0,groupsCreated=0;
 std::int32_t lastOperation=-1;
 const RomDisplay&screen()const{return display;}
};
}
