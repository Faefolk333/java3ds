#include "../native/cia_update.h"
#include <cassert>
int main(){
 const char*path="work/cia-update-fixture.cia";std::string bytes(100003,'x'),hash,error;unsigned size;
 FILE*f=fopen(path,"wb");fwrite(bytes.data(),1,bytes.size(),f);fclose(f);assert(updates::hashFile(path,hash,size));
 fake::reset();assert(!installOwnCia(path,size+1,hash,error));assert(fake::opened==0);
 fake::reset();assert(!installOwnCia(path,size,std::string(64,'0'),error));assert(fake::opened==0);
 fake::reset();fake::title++;assert(!installOwnCia(path,size,hash,error));assert(fake::started==0&&fake::romfsClosed==0);
 fake::reset();fake::shortWrite=true;assert(!installOwnCia(path,size,hash,error));assert(fake::cancelled==1&&fake::finished==0&&fake::romfsOpened==1);
 fake::reset();assert(installOwnCia(path,size,hash,error));assert(fake::bytes==bytes&&fake::finished==1&&fake::cancelled==0&&fake::romfsClosed==1&&fake::romfsOpened==0);
 remove(path);puts("CIA_UPDATE_PASS hash size title-ID short-write cancel RomFS streamed-install");
}
