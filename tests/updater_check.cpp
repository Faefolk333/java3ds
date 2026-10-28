#include "../native/release_info.h"
#include "../native/update_install.h"
#include <cassert>
#include <fstream>
#include <iterator>
static std::string read(const char*p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
static void write(const char*p,const std::string&s){std::ofstream f(p,std::ios::binary);f<<s;}
int main(){
 using namespace updates;
 const char*target="work/update-fixture.3dsx",*temp="work/update-fixture.3dsx.update";
 std::string binary="3DSX"+std::string(8192,'x'),old="3DSX previous version",hash,error;unsigned size=0;
 write(target,old);write(temp,binary);assert(hashFile(temp,hash,size));assert(size==binary.size());
 std::string json="{\"draft\":false,\"prerelease\":false,\"tag_name\":\"v0.4.0\",\"assets\":[{\"name\":\"java3ds.3dsx\",\"size\":"+std::to_string(size)+",\"digest\":\"sha256:"+hash+"\",\"url\":\"https://api.github.com/repos/Faefolk333/java3ds/releases/assets/1234\"}]}";
 Release r;assert(parse(json,r));assert(r.sha==hash&&r.size==size);assert(version(r.tag)>version("0.3.0"));assert(version("v0.10.1")>version("0.9.9"));assert(version("0.4.0-rc1")==-1);
 std::string bad=json;bad.replace(bad.find("api.github.com"),14,"evil.test");assert(!parse(bad,r));bad=json;bad.replace(bad.find("sha256:"),7,"md5:");assert(!parse(bad,r));assert(!parse(json+"garbage",r));assert(!parse("{",r));
 assert(!install(target,temp,size-1,hash,error));assert(read(target)==old);
 write(temp,"3DSX tampered payload");assert(!install(target,temp,size,hash,error));assert(read(target)==old);
 write(temp,binary);assert(install(target,temp,size,hash,error));assert(read(target)==binary);assert(read("work/update-fixture.3dsx.previous")==old);
 write(temp,std::string(8196,'z'));assert(hashFile(temp,hash,size));assert(!install(target,temp,size,hash,error));assert(read(target)==binary);
 remove(target);remove(temp);remove("work/update-fixture.3dsx.previous");
 puts("UPDATER_PASS version manifest URL digest size integrity format install backup");
}
