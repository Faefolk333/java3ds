#include "catalog.h"
#include "display_filter.h"
#include "pop_profile.h"
#include "miniz.h"
#include <dirent.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <cctype>
static std::string lower(std::string s){for(char&c:s)c=(char)std::tolower((unsigned char)c);return s;}
static std::string trim(const std::string&s){size_t a=s.find_first_not_of(" \t\r\n");if(a==std::string::npos)return "";size_t b=s.find_last_not_of(" \t\r\n");return s.substr(a,b-a+1);}
std::string joinPath(const std::string&a,const std::string&b){return a+(a.empty()||a.back()=='/'?"":"/")+b;}
std::string parentPath(const std::string&s){size_t n=s.find_last_of('/');return n==std::string::npos?".":n==0?"/":s.substr(0,n);}
static bool regular(const std::string&p){struct stat s;return !stat(p.c_str(),&s)&&S_ISREG(s.st_mode);}
static bool readFile(const std::string&p,std::vector<unsigned char>&b,size_t limit){FILE*f=fopen(p.c_str(),"rb");if(!f)return false;bool ok=fseek(f,0,SEEK_END)==0;long n=ftell(f);rewind(f);if(!ok||n<0||(size_t)n>limit){fclose(f);return false;}b.resize(n);ok=fread(b.data(),1,b.size(),f)==b.size()&&!ferror(f);fclose(f);return ok;}
static void attributes(const std::string&s,std::map<std::string,std::string>&props){std::string line,key;size_t p=0;while(p<s.size()){size_t e=s.find('\n',p);if(e==std::string::npos)e=s.size();line=s.substr(p,e-p);if(!line.empty()&&line.back()=='\r')line.pop_back();p=e+1;if(!line.empty()&&line[0]==' '&&!key.empty()){props[key]+=line.substr(1);continue;}size_t colon=line.find(':');if(colon==std::string::npos){key.clear();continue;}key=trim(line.substr(0,colon));if(!key.empty())props[key]=trim(line.substr(colon+1));}}
bool listGames(const std::string&p,std::vector<BrowserEntry>&out,std::string&error){out.clear();DIR*d=opendir(p.c_str());if(!d){error="Cannot read folder";return false;}dirent*e;while((e=readdir(d))){std::string n=e->d_name;if(n=="."||n==".."||n.empty()||n[0]=='.')continue;std::string path=joinPath(p,n);struct stat st;if(stat(path.c_str(),&st))continue;bool folder=S_ISDIR(st.st_mode);std::string ext=lower(n.size()>4?n.substr(n.size()-4):"");if(folder||ext==".jar"||ext==".jad")out.push_back({n,path,folder});if(out.size()>4096){closedir(d);error="Too many files (limit 4096)";return false;}}closedir(d);std::sort(out.begin(),out.end(),[](const BrowserEntry&a,const BrowserEntry&b){return a.folder!=b.folder?a.folder:lower(a.name)<lower(b.name);});return true;}
static bool zipEntry(mz_zip_archive&z,const char*name,std::vector<unsigned char>&out,size_t limit){int i=mz_zip_reader_locate_file(&z,name,nullptr,0);mz_zip_archive_file_stat st;if(i<0||!mz_zip_reader_file_stat(&z,i,&st)||st.m_uncomp_size>limit)return false;out.resize((size_t)st.m_uncomp_size);return mz_zip_reader_extract_to_mem(&z,i,out.data(),out.size(),0)!=0;}
static std::string decodeURL(const std::string&s){std::string r;for(size_t i=0;i<s.size();i++){if(s[i]=='%'&&i+2<s.size()&&std::isxdigit((unsigned char)s[i+1])&&std::isxdigit((unsigned char)s[i+2])){char h[3]={s[i+1],s[i+2],0};char c=(char)strtol(h,nullptr,16);if(!c)return "";r+=c;i+=2;}else r+=s[i];}return r;}
bool inspectGame(const std::string&path,GameInfo&g,std::string&error){
 g=GameInfo();g.source=path;g.jar=path;std::map<std::string,std::string>jad;std::vector<unsigned char>buf;
 FILE*sniff=fopen(path.c_str(),"rb");unsigned char magic[4]={};if(sniff){fread(magic,1,4,sniff);fclose(sniff);}bool zip=magic[0]=='P'&&magic[1]=='K'&&magic[2]==3&&magic[3]==4;
 if(!zip&&lower(path.size()>4?path.substr(path.size()-4):"")==".jad"){
  if(!readFile(path,buf,65536)){error="Cannot read JAD";return false;}attributes(std::string(buf.begin(),buf.end()),jad);
  std::string url=decodeURL(jad["MIDlet-Jar-URL"]);if(url.empty()){error="JAD has no MIDlet-Jar-URL";return false;}
  if(url.find("://")!=std::string::npos){url=url.substr(url.find_last_of('/')+1);}
  if(url.find(':')!=std::string::npos||(!url.empty()&&url[0]=='/')){error="JAD must reference a local relative JAR";return false;}
  g.jar=joinPath(parentPath(path),url);
  if(!regular(g.jar)){std::string sibling=path.substr(0,path.size()-4)+".jar";if(regular(sibling))g.jar=sibling;else{std::string doubleName=path.substr(0,path.size()-4);if(lower(doubleName.size()>4?doubleName.substr(doubleName.size()-4):"")==".jar"&&regular(doubleName))g.jar=doubleName;else{error="Descriptor only: matching game JAR missing";return false;}}}
 }
 if(!readFile(g.jar,buf,6*1024*1024)){error="Cannot read JAR (maximum 6 MiB)";return false;}g.crc=(uint32_t)mz_crc32(0,buf.data(),buf.size());g.pop=buf.size()==POP_SOURCE_SIZE&&g.crc==POP_SOURCE_CRC;
 mz_zip_archive z={};if(!mz_zip_reader_init_mem(&z,buf.data(),buf.size(),0)){error="JAR is not a valid ZIP";return false;}
 std::vector<unsigned char>manifest;if(!zipEntry(z,"META-INF/MANIFEST.MF",manifest,65536)){mz_zip_reader_end(&z);error="JAR has no MIDlet manifest";return false;}attributes(std::string(manifest.begin(),manifest.end()),g.properties);
 for(auto&kv:jad)g.properties[kv.first]=kv.second;
 for(int i=1;i<=64;i++){std::string key="MIDlet-"+std::to_string(i);auto entry=g.properties.find(key);if(entry==g.properties.end())break;const std::string&s=entry->second;size_t a=s.find(','),b=a==std::string::npos?a:s.find(',',a+1);if(b==std::string::npos)continue;GameEntry m{trim(s.substr(0,a)),trim(s.substr(b+1))};std::string cl=m.main;std::replace(cl.begin(),cl.end(),'.','/');cl+=".class";if(m.main.empty()||mz_zip_reader_locate_file(&z,cl.c_str(),nullptr,0)<0)continue;g.midlets.push_back(m);}
 // Native sizes are metadata/filename hints, never inferred from display hardware.
 std::string hint=path+" "+g.properties["Nokia-MIDlet-Original-Display-Size"];
 for(size_t i=0;i<hint.size();i++){int w=0,h=0;if(isdigit((unsigned char)hint[i])&&(i==0||!isdigit((unsigned char)hint[i-1]))&&sscanf(hint.c_str()+i,"%dx%d",&w,&h)==2&&w>=64&&w<=480&&h>=64&&h<=640&&w*h<=512*512){g.nativeWidth=w;g.nativeHeight=h;g.resolutionKnown=true;break;}}
 // Some touch builds describe their fixed canvas using vendor attributes.
 if(!g.resolutionKnown){int w=atoi(g.properties["LGE-MIDlet-TargetLCD-Width"].c_str()),h=atoi(g.properties["LGE-MIDlet-TargetLCD-Height"].c_str());if(w>=64&&w<=480&&h>=64&&h<=640&&w*h<=512*512){g.nativeWidth=w;g.nativeHeight=h;g.resolutionKnown=true;}}
 if(g.pop){g.nativeWidth=320;g.nativeHeight=240;g.resolutionKnown=true;}
 // This supplied LOTR build explicitly rejects landscape despite its filename.
 if(g.crc==0xd2c8911bu){g.nativeWidth=120;g.nativeHeight=160;g.resolutionKnown=true;g.properties["launcher.keypad"]="harem-siemens";}
 if((g.crc==0x68f550d4u||g.crc==0x9c8c91b8u||g.crc==0xa3d6b8f2u)){g.nativeWidth=240;g.nativeHeight=320;g.resolutionKnown=true;g.properties["launcher.compat.note"]="This build requires a portrait phone screen";}
 for(unsigned i=0;i<mz_zip_reader_get_num_files(&z);i++){mz_zip_archive_file_stat st;if(!mz_zip_reader_file_stat(&z,i,&st))continue;std::string n=st.m_filename;if(n.size()<6||n.substr(n.size()-6)!=".class"||st.m_uncomp_size>2*1024*1024)continue;std::vector<unsigned char>b;if(!zipEntry(z,n.c_str(),b,2*1024*1024))continue;std::string code(b.begin(),b.end());if(code.find("com/mascotcapsule/micro3d/")!=std::string::npos)g.properties["launcher.micro3d"]="experimental";else if(g.unsupported.empty()&&code.find("javax/microedition/m3g/")!=std::string::npos)g.properties["launcher.m3g"]="experimental";}
 if(g.crc==0xf604dae4u||g.crc==0x06253d5bu){g.nativeWidth=176;g.nativeHeight=208;g.resolutionKnown=true;g.properties["launcher.controls"]="vir2l-right-confirm";g.properties["launcher.keypad"]="numeric";}
 if(g.crc==0x0dbd9f30u)g.properties["launcher.raycaster"]="doom2-columns-v1";
 if(g.crc==0x2e771edcu)g.properties["launcher.raycaster"]="orcs2-columns-v1";
 if(mz_zip_reader_locate_file(&z,"Reg/Util.class",nullptr,0)>=0&&mz_zip_reader_locate_file(&z,"RegStarter.class",nullptr,0)>=0)g.unsupported="Requires unavailable Reg/Util native library";
 mz_zip_reader_end(&z);if(g.midlets.empty()){error="No launchable MIDlet found";return false;}g.title=g.properties["MIDlet-Name"];if(g.title.empty())g.title=g.midlets[0].title;
 char id[24];snprintf(id,sizeof(id),"%08lx",(unsigned long)g.crc);g.id=id;return true;
}
struct Cursor {std::vector<unsigned char>&b;size_t p;bool ok=true;unsigned u1(){if(p>=b.size()){ok=false;return 0;}return b[p++];}unsigned u2(){unsigned a=u1();return (a<<8)|u1();}uint32_t u4(){unsigned a=u2();return (uint32_t)(a<<16)|u2();}void skip(size_t n){if(p>b.size()||n>b.size()-p){ok=false;p=b.size();}else p+=n;}};
static void put2(std::vector<unsigned char>&b,unsigned v){b.push_back(v>>8);b.push_back(v);}
static bool redirectConnector(std::vector<unsigned char>&b){
 if(b.size()<10)return false;Cursor c{b,8};unsigned count=c.u2();std::vector<unsigned char>out(b.begin(),b.begin()+10);
 for(unsigned i=1;i<count&&c.ok;i++){size_t start=c.p;unsigned tag=c.u1();if(tag==1){unsigned n=c.u2();if(n>b.size()-c.p)return false;std::string value((char*)b.data()+c.p,n);c.skip(n);if(value=="javax/microedition/io/Connector"){out.push_back(1);const std::string replacement="crimson/ConnectionFactory";put2(out,replacement.size());out.insert(out.end(),replacement.begin(),replacement.end());continue;}}
 else if(tag==3||tag==4||tag==9||tag==10||tag==11||tag==12||tag==17||tag==18)c.skip(4);else if(tag==5||tag==6){c.skip(8);i++;}else if(tag==7||tag==8||tag==16||tag==19||tag==20)c.skip(2);else if(tag==15)c.skip(3);else return false;
 if(!c.ok)return false;out.insert(out.end(),b.begin()+start,b.begin()+c.p);}
 if(!c.ok)return false;out.insert(out.end(),b.begin()+c.p,b.end());b.swap(out);return true;
}
static bool resourcePatch(std::vector<unsigned char>&b){
 if(b.size()<10||memcmp(b.data(),"\xca\xfe\xba\xbe",4)||!redirectConnector(b))return false;Cursor c{b,8};unsigned count=c.u2();if(count>65520||count<1)return false;
 struct CP{unsigned tag=0,a=0,d=0;std::string text;};std::vector<CP>cp(count);for(unsigned i=1;i<count&&c.ok;i++){CP&v=cp[i];v.tag=c.u1();switch(v.tag){case 1:{unsigned n=c.u2();if(c.p>b.size()||n>b.size()-c.p)return false;v.text.assign((char*)b.data()+c.p,n);c.skip(n);break;}case 3:case 4:c.skip(4);break;case 5:case 6:c.skip(8);i++;break;case 7:case 8:case 16:case 19:case 20:v.a=c.u2();break;case 9:case 10:case 11:case 12:case 17:case 18:v.a=c.u2();v.d=c.u2();break;case 15:c.skip(3);break;default:return false;}}
 if(!c.ok)return false;size_t poolEnd=c.p;std::vector<unsigned>targets;
 for(unsigned i=1;i<count;i++)if(cp[i].tag==10&&cp[i].a<count&&cp[i].d<count){CP&cl=cp[cp[i].a];CP&nt=cp[cp[i].d];if(cl.a<count&&nt.a<count&&nt.d<count&&cp[cl.a].text=="java/lang/Class"&&cp[nt.a].text=="getResourceAsStream"&&cp[nt.d].text=="(Ljava/lang/String;)Ljava/io/InputStream;")targets.push_back(i);}
 if(targets.empty())return true;
 std::vector<unsigned char>extra;unsigned next=count;auto utf=[&](const char*s){unsigned i=next++;extra.push_back(1);put2(extra,strlen(s));extra.insert(extra.end(),s,s+strlen(s));return i;};auto one=[&](unsigned tag,unsigned a){unsigned i=next++;extra.push_back(tag);put2(extra,a);return i;};auto two=[&](unsigned tag,unsigned a,unsigned d){unsigned i=next++;extra.push_back(tag);put2(extra,a);put2(extra,d);return i;};unsigned clsName=utf("crimson/Resources"),cls=one(7,clsName),method=utf("open"),desc=utf("(Ljava/lang/Class;Ljava/lang/String;)Ljava/io/InputStream;"),nt=two(12,method,desc),ref=two(10,cls,nt);
 c.skip(6);unsigned interfaces=c.u2();c.skip(interfaces*2);
 for(int member=0;member<2;member++){unsigned n=c.u2();for(unsigned i=0;i<n&&c.ok;i++){c.skip(6);unsigned attrs=c.u2();for(unsigned a=0;a<attrs&&c.ok;a++){unsigned name=c.u2();uint32_t size=c.u4();size_t end=c.p+size;if(end<c.p||end>b.size())return false;if(member==1&&name<count&&cp[name].text=="Code"){
    c.skip(4);unsigned length=c.u4();size_t base=c.p,stop=base+length;if(stop<base||stop>end)return false;
    while(c.p<stop&&c.ok){size_t pos=c.p;unsigned op=c.u1(),step=1;if(op==182&&pos+2<stop){unsigned old=(b[pos+1]<<8)|b[pos+2];if(std::find(targets.begin(),targets.end(),old)!=targets.end()){b[pos]=184;b[pos+1]=ref>>8;b[pos+2]=ref;}}
     if(op==170||op==171){while((c.p-base)%4)c.skip(1);c.u4();if(op==170){int32_t low=(int32_t)c.u4(),high=(int32_t)c.u4();int64_t total=(int64_t)high-low+1;if(total<0||total>1000000)return false;c.skip((size_t)total*4);}else{unsigned pairs=c.u4();if(pairs>1000000)return false;c.skip((size_t)pairs*8);}continue;}
     if(op==196){unsigned sub=c.u1();c.skip(sub==132?4:2);continue;}if(op==185||op==186||op==200||op==201)step=5;else if(op==197)step=4;else if(op==17||op==19||op==20||op==132||(op>=153&&op<=168)||(op>=178&&op<=184)||op==187||op==189||op==192||op==193||op==198||op==199)step=3;else if(op==16||op==18||op==188||op==169||(op>=21&&op<=25)||(op>=54&&op<=58))step=2;else if(op>201)return false;c.skip(step-1);
    }if(c.p!=stop)return false;
   }c.p=end;}}}
 if(!c.ok)return false;std::vector<unsigned char>out;out.insert(out.end(),b.begin(),b.begin()+8);put2(out,next);out.insert(out.end(),b.begin()+10,b.begin()+poolEnd);out.insert(out.end(),extra.begin(),extra.end());out.insert(out.end(),b.begin()+poolEnd,b.end());b.swap(out);return true;
}
bool prepareGame(const GameInfo&game,const GameSettings&s,const std::string&profilePath,const std::string&dest,std::string&error,bool(*cancel)()){
 if(cancel&&cancel()){error="Launch cancelled";return false;}
 mz_zip_archive input={},output={},profile={};bool patch=game.pop&&s.width==400&&s.height==240;
 if(!mz_zip_reader_init_file(&input,game.jar.c_str(),0)){error="Cannot open game JAR";return false;}
 if(patch&&!mz_zip_reader_init_file(&profile,profilePath.c_str(),0)){mz_zip_reader_end(&input);error="Widescreen profile missing";return false;}
 std::string temp=dest+".part";bool ok=mz_zip_writer_init_file(&output,temp.c_str(),0)!=0;uint64_t total=0;unsigned count=mz_zip_reader_get_num_files(&input);
 if(count>4096)ok=false;
 for(unsigned i=0;ok&&i<count;i++){if(cancel&&cancel()){error="Launch cancelled";ok=false;break;}mz_zip_archive_file_stat st;if(!mz_zip_reader_file_stat(&input,i,&st)){ok=false;break;}if(st.m_is_directory)continue;if(st.m_uncomp_size>16*1024*1024||(total+=st.m_uncomp_size)>64*1024*1024){ok=false;break;}
  std::string name=st.m_filename;std::vector<unsigned char>data;bool replaced=patch&&zipEntry(profile,name.c_str(),data,16*1024*1024);
  bool klass=name.size()>6&&name.substr(name.size()-6)==".class";
  if(!replaced&&!klass){if(!mz_zip_writer_add_from_zip_reader(&output,&input,i))ok=false;continue;}
  if(!replaced){data.resize((size_t)st.m_uncomp_size);if(!mz_zip_reader_extract_to_mem(&input,i,data.data(),data.size(),0)){ok=false;break;}if(klass&&!resourcePatch(data)){error="Invalid or unsupported class file";ok=false;break;}}
  if(!mz_zip_writer_add_mem(&output,name.c_str(),data.data(),data.size(),MZ_BEST_SPEED))ok=false;
 }
 if(ok)ok=mz_zip_writer_finalize_archive(&output)!=0;mz_zip_writer_end(&output);mz_zip_reader_end(&input);if(patch)mz_zip_reader_end(&profile);
 if(ok){remove(dest.c_str());ok=rename(temp.c_str(),dest.c_str())==0;}if(!ok){remove(temp.c_str());if(error.empty())error="Cannot prepare game cache";}return ok;
}
bool writeLaunchConfig(const GameInfo&g,const GameSettings&s,const std::string&saves,const std::string&path,std::string&error){
 FILE*f=fopen(path.c_str(),"wb");if(!f){error="Cannot write launch settings";return false;}auto props=g.properties;props["launcher.main"]=g.midlets[s.midlet].main;props["launcher.width"]=std::to_string(s.width);props["launcher.height"]=std::to_string(s.height);props["launcher.keys"]=std::to_string(s.keys);props["launcher.profile"]=g.pop&&s.width==400&&s.height==240?"pop-classic":"generic";props["launcher.saves"]=joinPath(saves,g.id);
 std::string mapping;for(int i=0;i<14;i++){if(i)mapping+=",";mapping+=std::to_string(s.bindings[i]);}props["launcher.bindings"]=mapping;
 for(auto&kv:props)if(kv.first.find_first_of("\r\n=")==std::string::npos&&kv.second.find_first_of("\r\n")==std::string::npos)fprintf(f,"%s=%s\n",kv.first.c_str(),kv.second.c_str());bool ok=!ferror(f);if(fclose(f))ok=false;if(!ok)error="Cannot save launch settings";return ok;
}
bool loadSettings(const std::string&p,GameSettings&s){FILE*f=fopen(p.c_str(),"r");if(!f)return false;int w,h,scale,keys,midlet;bool ok=fscanf(f,"%d %d %d %d %d",&w,&h,&scale,&keys,&midlet)==5;int hidden=0,bindings[14]={},timeout=30,stereo=3,strength=3,filter=0,schema=0,tone=1,orientation=0;if(ok){fscanf(f,"%d",&hidden);for(int i=0;i<14;i++)if(fscanf(f,"%d",&bindings[i])!=1)break;fscanf(f,"%d",&timeout);fscanf(f,"%d %d",&stereo,&strength);fscanf(f,"%d %d",&filter,&schema);if(schema>=3)fscanf(f,"%d %d",&tone,&orientation);if(schema<2&&stereo==0)stereo=3;}fclose(f);if(ok&&w>=64&&w<=480&&h>=64&&h<=640&&w*h<=512*512&&scale>=0&&scale<=2&&keys>=0&&keys<=2&&midlet>=0&&midlet<64){s={w,h,scale,keys,midlet};s.hideBottom=hidden==1;s.bottomTimeout=timeout>=0&&timeout<=300?timeout:30;s.stereo=stereo>=0&&stereo<=3?stereo:3;s.filter=filter>=0&&filter<DISPLAY_FILTER_COUNT?filter:0;s.tone=tone>=0&&tone<=2?tone:1;s.orientation=orientation>=0&&orientation<=2?orientation:0;s.stereoStrength=strength>=1&&strength<=6?strength:3;for(int i=0;i<14;i++){int b=bindings[i];if(b==0||(b>=-7&&b<=-1)||(b>=48&&b<=57)||b==35||b==42||b==1001||b==1002)s.bindings[i]=b;}return true;}return false;}
bool saveSettings(const std::string&p,const GameSettings&s){std::string temp=p+".new";FILE*f=fopen(temp.c_str(),"w");if(!f)return false;bool ok=fprintf(f,"%d %d %d %d %d\n",s.width,s.height,s.scale,s.keys,s.midlet)>0;if(ok){fprintf(f,"%d",s.hideBottom?1:0);for(int b:s.bindings)fprintf(f," %d",b);fprintf(f,"\n%d\n%d %d\n%d 3 %d %d\n",s.bottomTimeout,s.stereo,s.stereoStrength,s.filter,s.tone,s.orientation);if(ferror(f))ok=false;}if(fclose(f))ok=false;if(ok){remove(p.c_str());ok=rename(temp.c_str(),p.c_str())==0;}if(!ok)remove(temp.c_str());return ok;}
