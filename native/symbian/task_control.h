#pragma once
#include <cstdint>
#include <functional>
#include <utility>
namespace symbian {
// One synchronous task owns this control across file I/O, linking and execution.
// A transient false poll permanently cancels that task; later polls cannot
// restart it. Callers create a new control for an explicit retry.
class TaskControl {
 std::function<bool()> poll;
 std::function<void(const char*,std::uint32_t,std::uint32_t)> progress;
 bool stopped=false;
 const char*currentStage="starting";
 std::uint32_t completed=0,totalBytes=0;
public:
 explicit TaskControl(std::function<bool()> p={},std::function<void(const char*,std::uint32_t,std::uint32_t)> notify={}):poll(std::move(p)),progress(std::move(notify)){}
 bool checkpoint(){if(stopped)return false;if(poll&&!poll())stopped=true;return !stopped;}
 void cancel(){stopped=true;}
 bool cancelled()const{return stopped;}
 void report(const char*stage,std::uint32_t done=0,std::uint32_t total=0){
  if(stopped)return;currentStage=stage;completed=done;totalBytes=total;
  if(progress)progress(stage,done,total);
 }
 const char*stage()const{return currentStage;}
 std::uint32_t done()const{return completed;}
 std::uint32_t total()const{return totalBytes;}
};
}
