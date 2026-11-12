// Protocol facts: pinned EKA2L1 services/window/{op.h,opheader.h,classes/scrdvc.cpp}.
#include "window_server.h"
namespace symbian {
namespace {KernelResult done(int value){return {KernelResult::Kind::Returned,std::uint32_t(value),0,0};}}
bool WindowServer::configure(const RomDisplay&value){
 if(!value.width||value.width>1024||!value.height||value.height>1024)return false;
 for(const auto&s:sessions)if(s.handle)return false;
 int mode=0;switch(value.bits){case 1:mode=1;break;case 2:mode=2;break;case 4:mode=5;break;case 8:mode=6;break;case 12:mode=10;break;case 16:mode=7;break;case 24:mode=8;break;case 32:mode=12;break;default:return false;}
 display=value;displayMode=mode;return true;
}
KernelResult WindowServer::request(std::uint32_t handle,std::int32_t operation,const std::array<std::uint32_t,4>&args){
 if(operation==-1){for(auto&s:sessions)if(!s.handle){s={handle,handle,false};return done(0);}return done(-4);}
 Session*s=nullptr;for(auto&candidate:sessions)if(handle&&candidate.handle==handle)s=&candidate;if(!s)return done(-8);
 if(operation==-2){for(auto&o:objects)if(o.owner==handle)o={};*s={};return done(0);}
 calls++;lastOperation=operation;
 if(operation==2){if(!s->initialized){s->initialized=true;initializations++;}return done(handle);}
 if(!s->initialized)return done(-18);
 if(operation==0||operation==4)return commands(*s,args);
 return {};
}
KernelResult WindowServer::commands(Session&session,const std::array<std::uint32_t,4>&args){
 GuestDescriptor bytes(memory);if(!bytes.view(args[0],1,false)||bytes.length>65536)return done(-6);
 struct Command {unsigned op=0,length=0;std::uint32_t object=0,data=0;};
 std::vector<Command> list;std::uint32_t object=session.lastObject;unsigned at=0;
 while(at<bytes.length){
  if(bytes.length-at<4||list.size()==1024)return done(-6);std::uint16_t header[2];memory.read(bytes.data+at,header,4);at+=4;
  if(header[0]&0x8000){if(bytes.length-at<4)return done(-6);memory.read(bytes.data+at,&object);at+=4;}
  if(header[1]>bytes.length-at)return done(-6);
  list.push_back({unsigned(header[0]&0x7fff),header[1],object,bytes.data+at});at+=header[1];
 }
 // Run against shadow object/session state. Unknown or malformed commands and
 // bad reply descriptors leave the entire batch unapplied, including closes.
 auto staged=objects;auto state=session;auto next=nextObject;unsigned additions=0,groupAdditions=0,inquiries=0;
 std::array<std::int32_t,16> reply{};unsigned replyBytes=0,replies=0;int result=0;bool needsReply=false;std::u16string textReply;bool textOutput=false;
 for(const auto&c:list){
  if(!state.initialized)return done(-18);
  lastCommand=c.op;lastObject=c.object;state.lastObject=c.object;
  auto value=[&](unsigned offset=0){std::int32_t word=0;memory.read(c.data+offset,&word);return word;};
  if(c.object==session.handle){
   // EPOC6 predates the two custom-text-cursor session commands.
   auto sessionOp=c.op>=28?c.op+2:c.op;
   if(c.op==0){if(c.length)return done(-6);state.initialized=false;for(auto&o:staged)if(o.owner==session.handle)o={};result=0;continue;}
   if(c.op==21){
    if(c.length!=8&&c.length!=16)return done(-6);if(!displayMode)return done(-18);
    if(c.length==16){if(value(8)){bool found=false;for(const auto&o:staged)if(o.handle==std::uint32_t(value(8))&&o.owner==session.handle&&o.type==Object::Group)found=true;if(!found)return done(-6);}if(value(12)){bool found=false;for(const auto&o:staged)if(o.handle==std::uint32_t(value(12))&&o.owner==session.handle&&o.type==Object::Screen)found=true;if(!found)return done(-6);}}
    Object*slot=nullptr;for(auto&o:staged)if(!o.handle){slot=&o;break;}if(!slot||next>=0x7fffffff)return done(-4);
    slot->handle=next++;slot->owner=session.handle;slot->type=Object::Group;slot->client=value();slot->focus=value(4)!=0;slot->parent=c.length==16?value(8):0;result=slot->handle;groupAdditions++;continue;
   }
   if(sessionOp==40||sessionOp==41){if(c.length)return done(-6);result=0;for(const auto&o:staged)if(o.type==Object::Group&&(sessionOp==40?o.focus:o.defaultOwner))result=o.handle;continue;}
   if(c.op!=25)return {};
   if(c.length!=0&&c.length!=4&&c.length!=8)return done(-6);if(c.length&&value()!=0)return done(-1);if(!displayMode)return done(-18);
   Object*slot=nullptr;for(auto&o:staged)if(!o.handle){slot=&o;break;}if(!slot||next>=0x7fffffff)return done(-4);
   *slot={};slot->handle=next++;slot->owner=session.handle;slot->type=Object::Screen;result=slot->handle;additions++;continue;
  }
  Object*screen=nullptr;for(auto&o:staged)if(o.handle==c.object&&o.owner==session.handle)screen=&o;if(!screen)return done(-8);
  if(screen->type==Object::Group){
   result=0;
   // Old RWindowTreeNode lacks AbsPosition and advanced pointer events.
   unsigned groupOp=c.op;if(groupOp>=12)groupOp++;if(groupOp>=98)groupOp++;
   switch(groupOp){
   case 0:if(c.length)return done(-6);for(const auto&o:staged)if(o.handle&&o.parent==screen->handle)return done(-14);*screen={};break;
   case 37:if(c.length!=4)return done(-6);screen->focus=value()!=0;break;
   case 61:{
    if(c.length!=0&&c.length!=4)return done(-6);if(++replies>1)return done(-6);textReply=screen->name;textOutput=true;GuestDescriptor output(memory);if(!output.view(args[1],2,true)||!output.canWrite(textReply.size()))return done(-6);break;
   }
   case 62:{
    if(c.length!=4&&c.length!=8)return done(-6);GuestDescriptor name(memory);std::u16string text;
    if(!name.text(c.length==8?std::uint32_t(value(4)):args[2],text)||value()<0||std::uint32_t(value())!=text.size())return done(-6);screen->name=std::move(text);break;
   }
   case 64:if(c.length)return done(-6);for(auto&o:staged)if(o.type==Object::Group)o.defaultOwner=false;screen->defaultOwner=true;break;
   case 88:if(c.length)return done(-6);result=screen->handle;break;
   case 130:if(c.length)return done(-6);result=screen->client;break;
   default:return {};
   }continue;
  }
  inquiries++;result=0;
  switch(c.op){
  case 2:case 3:
   if(c.length)return done(-6);reply[0]=display.width*(c.op==2?15:1);reply[1]=display.height*(c.op==2?15:1);replyBytes=8;break;
  case 4:if(c.length)return done(-6);*screen={};break;
  case 5:case 6:case 7:case 8:
   if(c.length!=4)return done(-6);if(c.op<7)result=value()/15;else{auto pixels=std::int64_t(value())*15;if(pixels<INT32_MIN||pixels>INT32_MAX)return done(-6);result=pixels;}break;
  case 9:if(c.length)return done(-6);result=displayMode;break;
  case 14:case 15:
   if(c.length!=(c.op==14?20:12))return done(-6);
   if(value()!=int(display.width)||value(4)!=int(display.height)||value(c.op==14?16:8)!=0)return done(-5);break;
  case 16:case 17:case 20:case 21:{
   if(c.length!=(c.op>=20?4:0))return done(-6);if(c.op>=20&&value()!=0)return done(-6);
   bool twips=c.op==16||c.op==20;reply={};reply[0]=display.width;reply[1]=display.height;if(twips){reply[2]=display.width*15;reply[3]=display.height*15;}replyBytes=twips?20:12;break;
  }
  case 18:if(c.length)return done(-6);result=1;break;
  case 19:case 34:if(c.length!=4||value()!=0)return done(-6);break;
  case 22:if(c.length!=4||value()<0||value()>2)return done(-6);screen->enforcement=value();break;
  case 23:if(c.length)return done(-6);result=screen->enforcement;break;
  case 25:if(c.length!=4||value()!=0)return done(-6);result=1;break;
  case 26:if(c.length)return done(-6);result=display.bits>=24?0x1000000:1u<<display.bits;break;
  case 29:case 39:if(c.length)return done(-6);break;
  case 30:case 31:case 35:case 36:
   if(c.length!=(c.op==30||c.op==36?0:4))return done(-6);if(c.length&&value()!=0)return done(-6);reply={};replyBytes=8;break;
  case 32:case 33:
   if(c.length!=(c.op==32?4:0))return done(-6);if(c.length&&value()!=0)return done(-6);reply={};reply[0]=reply[1]=1;replyBytes=8;break;
  case 40:if(c.length)return done(-6);reply={};replyBytes=4;result=1;break;
  case 41:if(c.length!=4||value()!=0)return done(-6);result=displayMode;break;
  case 43:if(c.length!=4)return done(-6);result=0;break;
  case 54:if(c.length!=4||value()>0)return done(-6);break;
  default:return {};
  }
  bool producesReply=c.op==2||c.op==3||c.op==16||c.op==17||c.op==20||c.op==21||c.op==30||c.op==31||c.op==32||c.op==33||c.op==35||c.op==36||c.op==40;
  if(producesReply){if(++replies>1)return done(-6);GuestDescriptor output(memory);if(!output.view(args[1],1,true)||!output.canWrite(replyBytes))return done(-6);needsReply=true;}
 }
 // Queries share the protocol's single reply package; production clients send
 // one reply-producing command per synchronous request. Reject ambiguity.
 if(textOutput){GuestDescriptor output(memory);if(!output.view(args[1],2,true)||!output.write(textReply.data(),textReply.size()))return done(-6);}
 if(needsReply){GuestDescriptor output(memory);if(!output.view(args[1],1,true)||!output.write(reply.data(),replyBytes))return done(-6);}
 objects=std::move(staged);session=state;nextObject=next;created+=additions;groupsCreated+=groupAdditions;queries+=inquiries;return done(result);
}
}
