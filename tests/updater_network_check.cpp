#include "../native/updater.h"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <cstdio>
static void waitFor(){for(int i=0;i<2400&&(updaterState()==1||updaterState()==3);i++)std::this_thread::sleep_for(std::chrono::milliseconds(50));printf("state=%d message=%s\n",updaterState(),updaterMessage().c_str());fflush(stdout);}
int main(int argc,char**argv){
 assert(argc==3);std::filesystem::path root=std::filesystem::absolute(argv[1]);std::filesystem::create_directories(root/"cache");assert(!std::filesystem::exists(root/"github-token.txt"));
 std::string exe=(root/"test.3dsx").string();std::ofstream(exe)<<"3DSXprevious";std::ofstream(root/"save-sentinel")<<"preserve";
 updaterInit(root.string(),exe);updaterCheck();waitFor();
 if(std::string(argv[2])=="private"){assert(updaterState()==5&&updaterMessage().find("private")!=std::string::npos);assert(updaterClose());puts("ANONYMOUS_PRIVATE_CHECK_PASS network_reached_no_token");return 0;}
 assert(updaterState()==2);updaterInstall();waitFor();assert(updaterState()==4);assert(updaterClose());
 std::ifstream previous(exe+".previous");std::string value;previous>>value;assert(value=="3DSXprevious");std::ifstream(root/"save-sentinel")>>value;assert(value=="preserve");assert(std::filesystem::file_size(exe)>1024*1024);
 puts("ANONYMOUS_UPDATE_PASS HTTPS_no_token_verified_SHA_size_magic_backup_preserved_save");
}
