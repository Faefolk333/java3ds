#include "../native/asset_install.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
int main(){
 namespace fs=std::filesystem;
 const fs::path root="work/asset-install-check";fs::create_directories(root);
 char error[160];
 struct Asset{const char*source;const char*name;PopAssetKind kind;};
 const Asset assets[]={
  {"romfs/java/midp.jar","midp.jar",PopAssetKind::Jar},
  {"romfs/probe/java3ds-symbian-probe.3dsx","probe.3dsx",PopAssetKind::ThreeDsx},
  {"romfs/probe/java3ds-symbian-probe.smdh","probe.smdh",PopAssetKind::Smdh}};
 for(const auto&a:assets){
  auto dest=(root/a.name).string();std::ofstream(dest,std::ios::binary)<<"previous";
  assert(pop_install_file(a.source,dest.c_str(),error,sizeof(error),a.kind)==1);
  assert(pop_same_file(a.source,dest.c_str()));
  assert(pop_install_file(a.source,dest.c_str(),error,sizeof(error),a.kind)==0);
  assert(!pop_exists((dest+".part").c_str())&&!pop_exists((dest+".recovery").c_str()));
 }
 auto dest=(root/"probe.3dsx").string();
 assert(pop_install_file(assets[1].source,dest.c_str(),error,sizeof(error))==-1);
 assert(pop_same_file(assets[1].source,dest.c_str()));
 auto bad=(root/"truncated.smdh").string();std::ofstream(bad,std::ios::binary)<<"SMDH";
 auto smdh=(root/"probe.smdh").string();
 assert(pop_install_file(bad.c_str(),smdh.c_str(),error,sizeof(error),PopAssetKind::Smdh)==-1);
 assert(pop_same_file(assets[2].source,smdh.c_str()));
 assert(pop_install_file(bad.c_str(),dest.c_str(),error,sizeof(error),PopAssetKind::ThreeDsx)==-1);
 assert(pop_same_file(assets[1].source,dest.c_str()));
 puts("ASSET_INSTALL_PASS real_JAR_3DSX_SMDH replacement identical validation_failure_preserves_destination");
}
