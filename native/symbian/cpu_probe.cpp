#include "cpu_probe.h"
#include "probe_guest.h"
#include <cpu/dyncom/arm_dyncom.h>
#include <cpu/12l1r/exclusive_monitor.h>
#include <vector>
#include <cstring>
#include <chrono>
namespace {
struct Probe {
 std::vector<std::uint8_t> memory;
 eka2l1::arm::r12l1::exclusive_monitor monitor{1};
 eka2l1::arm::dyncom_core cpu{&monitor,12};
 unsigned svc=0,faults=0;
 template<class T>bool read(std::uint32_t at,T*out){if(at>memory.size()||sizeof(T)>memory.size()-at)return false;std::memcpy(out,memory.data()+at,sizeof(T));return true;}
 template<class T>bool write(std::uint32_t at,T*value){if(at>memory.size()||sizeof(T)>memory.size()-at)return false;std::memcpy(memory.data()+at,value,sizeof(T));return true;}
 template<class T>std::int32_t exclusive(std::uint32_t at,T value,T expected){T current;if(!read(at,&current))return -1;if(current!=expected)return 0;write(at,&value);return 1;}
 Probe():memory(65536){
  std::memcpy(memory.data()+0x1000,symbian_probe_guest,sizeof(symbian_probe_guest));
  cpu.read_8bit=[this](auto a,auto p){return read(a,p);};
  // std::function signatures differ; bind each concrete width independently.
  cpu.read_16bit=[this](auto a,auto p){return read(a,p);};cpu.read_32bit=[this](auto a,auto p){return read(a,p);};cpu.read_64bit=[this](auto a,auto p){return read(a,p);};cpu.read_code=cpu.read_32bit;
  cpu.write_8bit=[this](auto a,auto p){return write(a,p);};cpu.write_16bit=[this](auto a,auto p){return write(a,p);};cpu.write_32bit=[this](auto a,auto p){return write(a,p);};cpu.write_64bit=[this](auto a,auto p){return write(a,p);};
  cpu.exclusive_write_8bit=[this](auto a,auto v,auto e){return exclusive(a,v,e);};cpu.exclusive_write_16bit=[this](auto a,auto v,auto e){return exclusive(a,v,e);};cpu.exclusive_write_32bit=[this](auto a,auto v,auto e){return exclusive(a,v,e);};cpu.exclusive_write_64bit=[this](auto a,auto v,auto e){return exclusive(a,v,e);};
  monitor.read_8bit=[this](auto,auto a,auto p){return read(a,p);};monitor.read_16bit=[this](auto,auto a,auto p){return read(a,p);};monitor.read_32bit=[this](auto,auto a,auto p){return read(a,p);};monitor.read_64bit=[this](auto,auto a,auto p){return read(a,p);};
  monitor.write_8bit=[this](auto,auto a,auto v,auto e){return exclusive(a,v,e);};monitor.write_16bit=[this](auto,auto a,auto v,auto e){return exclusive(a,v,e);};monitor.write_32bit=[this](auto,auto a,auto v,auto e){return exclusive(a,v,e);};monitor.write_64bit=[this](auto,auto a,auto v,auto e){return exclusive(a,v,e);};
  cpu.system_call_handler=[this](auto id){svc=id;cpu.stop();};
  cpu.exception_handler=[this](auto,auto){faults++;cpu.stop();return false;};
 }
 void run(std::uint32_t pc,bool thumb=false){svc=0;cpu.set_cpsr(thumb?0x30:0x10);cpu.set_pc(pc);cpu.run(200);}
};
}
SymbianCpuResult symbianCpuProbe(bool(*keepRunning)()){
 SymbianCpuResult result;Probe p;
 p.cpu.set_reg(2,0x3000);p.run(0x1000);std::uint32_t value=0;p.read(0x3000,&value);result.arm=p.svc==0x42&&p.cpu.get_reg(1)==55&&p.cpu.get_reg(3)==55&&value==55;
 eka2l1::arm::core::thread_context saved{};p.cpu.save_context(saved);
 p.cpu.set_reg(2,0x3100);p.run(0x1100,true);p.read(0x3100,&value);result.thumb=p.svc==0x43&&p.cpu.get_reg(0)==10&&value==10;
 p.cpu.load_context(saved);result.context=p.cpu.get_reg(1)==55&&p.cpu.get_reg(2)==0x3000&&!p.cpu.is_thumb_mode();
 p.cpu.set_reg(2,0x3000);p.run(0x1200);p.read(0x3000,&value);result.exclusive=p.svc==0x44&&value==56&&p.cpu.get_reg(3)==0;
 p.cpu.set_vfp(0,0x3fc00000);p.cpu.set_vfp(1,0x40100000);p.run(0x1300);bool single=p.svc==0x45&&p.cpu.get_vfp(2)==0x40700000;
 p.cpu.set_vfp(0,0);p.cpu.set_vfp(1,0x3ff80000);p.cpu.set_vfp(2,0);p.cpu.set_vfp(3,0x40020000);p.run(0x1500);result.vfp=single&&p.svc==0x46&&p.cpu.get_vfp(4)==0&&p.cpu.get_vfp(5)==0x400e0000;
 p.cpu.set_reg(5,0);p.cpu.set_pc(0x1400);p.cpu.set_cpsr(0x10);p.cpu.run(1000);result.budget=p.cpu.get_num_instruction_executed()>=1000&&p.cpu.get_num_instruction_executed()<1100&&p.cpu.get_reg(5)>0;
 for(int i=0;i<100;i++){if(keepRunning&&!keepRunning()){result.budget=false;break;}auto start=std::chrono::steady_clock::now();p.cpu.run(10000);result.benchmarkMicros+=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-start).count();result.benchmarkInstructions+=p.cpu.get_num_instruction_executed();}result.faults=p.faults;return result;
}
