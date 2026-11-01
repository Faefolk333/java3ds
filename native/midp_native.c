#include "jvm.h"
#include <errno.h>
#include "jvm_util.h"
#include "crimson_zip_cache.h"
#include "raster.h"
#include "stereo.h"
#include "audio/device_3ds.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_NO_STDIO
#define STBI_MAX_DIMENSIONS 16384
#include "../third-party/stb_image.h"
static int arg(Runtime*r,int n){return localvar_getInt(r->localvar,n);}
static Instance* arr(Runtime*r,int n){return localvar_getRefer(r->localvar,n);}
static s32 decode(Runtime*r,JClass*c){
 (void)c;Instance*b=arr(r,0);int o=arg(r,1),n=arg(r,2),w=0,h=0,channels;
 if(!b||o<0||n<0||o>b->arr_length-n){push_ref(r->stack,NULL);return 0;}
 if(!stbi_info_from_memory((unsigned char*)b->arr_body+o,n,&w,&h,&channels)||w<1||h<1||w>16384||h>16384||(uint64_t)w*h>1048576){jvm_printf("IMAGE: invalid or oversized dimensions %dx%d\n",w,h);push_ref(r->stack,NULL);return 0;}
 unsigned char*p=stbi_load_from_memory((unsigned char*)b->arr_body+o,n,&w,&h,&channels,4);
 if(!p){jvm_printf("IMAGE: %s\n",stbi_failure_reason());push_ref(r->stack,NULL);return 0;}
 Instance*dims=arr(r,3);if(!dims||dims->arr_length<2){stbi_image_free(p);push_ref(r->stack,NULL);return 0;}
 Instance*result=jarray_create_by_type_index(r,w*h,DATATYPE_INT);
 if(!result){stbi_image_free(p);push_ref(r->stack,NULL);return 0;}
 s32*d=(s32*)result->arr_body;((s32*)dims->arr_body)[0]=w;((s32*)dims->arr_body)[1]=h;
 for(int i=0;i<w*h;i++)d[i]=(u32)p[i*4+3]<<24|(u32)p[i*4]<<16|(u32)p[i*4+1]<<8|p[i*4+2];
 stbi_image_free(p);push_ref(r->stack,result);return 0;
}
static uint32_t* target(Runtime*r,int w,int h){
 Instance*t=arr(r,0);if(w<1||h<1||w>16384||h>16384||(uint64_t)w*h>1048576)return NULL;
 if(t)return t->arr_length>=w*h?(uint32_t*)t->arr_body:NULL;
 return w==crimson_width&&h==crimson_height?crimson_pixels:NULL;
}
static s32 fill(Runtime*r,JClass*c){(void)c;int w=arg(r,1),h=arg(r,2);uint32_t*d=target(r,w,h);if(d)crimson_fill(d,w,h,arg(r,3),arg(r,4),arg(r,5),arg(r,6),arg(r,7));return 0;}
static s32 blit(Runtime*r,JClass*c){(void)c;int w=arg(r,1),h=arg(r,2),sw=arg(r,4),sh=arg(r,5);uint32_t*d=target(r,w,h);Instance*s=arr(r,3);if(d&&s&&sw>0&&sh>0&&sw<=16384&&sh<=16384&&(uint64_t)sw*sh<=1048576&&s->arr_length>=sw*sh)crimson_blit(d,w,h,(uint32_t*)s->arr_body,sw,sh,arg(r,6),arg(r,7),arg(r,8),arg(r,9),arg(r,10),arg(r,11));return 0;}
static s32 region(Runtime*r,JClass*c){
 (void)c;int tw=arg(r,1),th=arg(r,2);uint32_t*d=target(r,tw,th);Instance*src=arr(r,3);
 int sw=arg(r,4),sh=arg(r,5),sx=arg(r,6),sy=arg(r,7),w=arg(r,8),h=arg(r,9),t=arg(r,10),dx=arg(r,11),dy=arg(r,12),cx=arg(r,13),cy=arg(r,14),cw=arg(r,15),ch=arg(r,16);
 if(!d||!src||sw<1||sh<1||sw>16384||sh>16384||(uint64_t)sw*sh>1048576||src->arr_length<sw*sh||sx<0||sy<0||w<1||h<1||sx+w>sw||sy+h>sh||t<0||t>8)return 0;
 int dw=t==8?w+80:(t>=4?h:w),dh=t==8?h:(t>=4?w:h);uint32_t*a=(uint32_t*)src->arr_body;float*z=crimson_stereo_target(d);float layer=crimson_stereo_layer();
 for(int y=0;y<dh;y++){int yy=dy+y;if(yy<0||yy>=th||yy<cy||yy>=cy+ch)continue;
 for(int x=0;x<dw;x++){int xx=dx+x;if(xx<0||xx>=tw||xx<cx||xx>=cx+cw)continue;
 int u=x,v=y;switch(t){case 8:u=x*w/dw;break;case 1:v=h-1-y;break;case 2:u=w-1-x;break;case 3:u=w-1-x;v=h-1-y;break;case 4:u=y;v=x;break;case 5:u=y;v=h-1-x;break;case 6:u=w-1-y;v=x;break;case 7:u=w-1-y;v=h-1-x;break;}
 uint32_t col=a[(sy+v)*sw+sx+u],alpha=col>>24;if(alpha&&z)z[yy*tw+xx]=layer;if(alpha==255)d[yy*tw+xx]=col;else if(alpha){uint32_t old=d[yy*tw+xx],inv=255-alpha;unsigned red=(((col>>16)&255)*alpha+((old>>16)&255)*inv)/255,green=(((col>>8)&255)*alpha+((old>>8)&255)*inv)/255,blue=((col&255)*alpha+(old&255)*inv)/255;d[yy*tw+xx]=0xff000000|(red<<16)|(green<<8)|blue;}
 }}return 0;
}
static uint32_t over(uint32_t dst,uint32_t src,int alpha){
 unsigned a=alpha?src>>24:255;if(a==255)return src|0xff000000;if(!a)return dst;unsigned inv=255-a,out=0xff000000;
 for(int n=0;n<24;n+=8)out|=((((src>>n)&255)*a+((dst>>n)&255)*inv)/255)<<n;return out;
}
static s32 readPixels(Runtime*r,JClass*c){
 (void)c;int tw=arg(r,1),th=arg(r,2);uint32_t*source=target(r,tw,th);Instance*dst=arr(r,3);int off=arg(r,4),scan=arg(r,5),x=arg(r,6),y=arg(r,7),w=arg(r,8),h=arg(r,9);if(!source||!dst||x<0||y<0||w<0||h<0||w>tw-x||h>th-y)return 0;
 Instance*sz=arr(r,10),*dz=arr(r,11);float*sourceDepth=crimson_stereo_target(source);if(crimson_stereo_mode()&&sz&&sz->arr_length>=tw*th)sourceDepth=(float*)sz->arr_body;float*destinationDepth=dz&&dz->arr_length>=dst->arr_length?(float*)dz->arr_body:NULL;
 for(int row=0;row<h;row++){int64_t index=(int64_t)off+(int64_t)row*scan;if(index<0||index+w>dst->arr_length)return 0;memmove((uint32_t*)dst->arr_body+index,source+(y+row)*tw+x,w*4);if(destinationDepth){if(sourceDepth)memmove(destinationDepth+index,sourceDepth+(y+row)*tw+x,w*4);else for(int col=0;col<w;col++)destinationDepth[index+col]=0;}}return 0;
}
static s32 rgb(Runtime*r,JClass*c){
 (void)c;int tw=arg(r,1),th=arg(r,2);uint32_t*d=target(r,tw,th);Instance*src=arr(r,3);if(!d||!src)return 0;
 int off=arg(r,4),scan=arg(r,5),dx=arg(r,6),dy=arg(r,7),w=arg(r,8),h=arg(r,9),alpha=arg(r,10),cx=arg(r,11),cy=arg(r,12),cw=arg(r,13),ch=arg(r,14);
 int x0=dx>cx?dx:cx,y0=dy>cy?dy:cy,x1=(int64_t)dx+w<(int64_t)cx+cw?dx+w:cx+cw,y1=(int64_t)dy+h<(int64_t)cy+ch?dy+h:cy+ch;
 if(x0<0)x0=0;if(y0<0)y0=0;if(x1>tw)x1=tw;if(y1>th)y1=th;
 uint32_t*p=(uint32_t*)src->arr_body;Instance*shadow=arr(r,15),*sourceShadow=arr(r,16);float*inputDepth=sourceShadow&&sourceShadow->arr_length>=src->arr_length?(float*)sourceShadow->arr_body:NULL;float*z=crimson_stereo_target(d);if(crimson_stereo_mode()&&shadow&&shadow->arr_length>=tw*th)z=(float*)shadow->arr_body;float layer=crimson_stereo_layer();
 for(int y=y0;y<y1;y++)for(int x=x0;x<x1;x++){int64_t index=(int64_t)off+(int64_t)(y-dy)*scan+x-dx;if(index>=0&&index<src->arr_length){if(z&&(!alpha||(p[index]>>24)))z[y*tw+x]=inputDepth?inputDepth[index]:layer;d[y*tw+x]=over(d[y*tw+x],p[index],alpha);}}
 if(z&&inputDepth)crimson_stereo_content_drawn();return 0;
}
static s32 text(Runtime*r,JClass*c){
 (void)c;int tw=arg(r,1),th=arg(r,2);uint32_t*d=target(r,tw,th);Instance*font=arr(r,3),*str=arr(r,4);if(!d||!font||!str)return 0;
 int dx=arg(r,5),dy=arg(r,6),scale=arg(r,7),color=arg(r,8)|0xff000000,cx=arg(r,9),cy=arg(r,10),cw=arg(r,11),ch=arg(r,12);if(scale<1||scale>4)return 0;
 Instance*shadow=arr(r,13);float*z=crimson_stereo_target(d);if(crimson_stereo_mode()&&shadow&&shadow->arr_length>=tw*th)z=(float*)shadow->arr_body;float layer=crimson_stereo_layer();
 int length=jstring_get_count(str,r);for(int i=0;i<length;i++){unsigned code=jstring_char_at(str,i,r);if(code>127)code=63;if(code>=font->arr_length)continue;uint64_t bits;memcpy(&bits,font->arr_body+code*8,8);
 for(int y=0;y<8*scale;y++){int yy=dy+y;if(yy<0||yy>=th||yy<cy||yy>=cy+ch)continue;for(int x=0;x<8*scale;x++){int xx=dx+i*8*scale+x;if(xx<0||xx>=tw||xx<cx||xx>=cx+cw)continue;if(bits&((uint64_t)1<<((y/scale)*8+x/scale))){d[yy*tw+xx]=color;if(z)z[yy*tw+xx]=layer;}}}}return 0;
}

static s32 stereoMode(Runtime*r,JClass*c){(void)c;push_int(r->stack,crimson_stereo_mode());return 0;}
static s32 stereoRect(Runtime*r,JClass*c){(void)c;Instance*depth=arr(r,0);int w=arg(r,1),h=arg(r,2),x=arg(r,3),y=arg(r,4),rw=arg(r,5),rh=arg(r,6);if(!crimson_stereo_mode()||!depth||w<1||h<1||(int64_t)w*h>depth->arr_length)return 0;float*z=(float*)depth->arr_body,layer=crimson_stereo_layer();int x0=x>0?x:0,y0=y>0?y:0,x1=(int64_t)x+rw<w?x+rw:w,y1=(int64_t)y+rh<h?y+rh:h;if(x0==0&&y0==0&&x1==w&&y1==h&&layer==-4){layer=0;crimson_stereo_finish_frame();}for(int yy=y0;yy<y1;yy++)for(int xx=x0;xx<x1;xx++)z[yy*w+xx]=layer;return 0;}
static s32 stereoRegion(Runtime*r,JClass*c){
 (void)c;if(!crimson_stereo_mode())return 0;int tw=arg(r,1),th=arg(r,2);uint32_t*d=target(r,tw,th);Instance*src=arr(r,3),*sourceDepth=arr(r,17),*destinationDepth=arr(r,18);float*z=crimson_stereo_target(d);if(destinationDepth&&(int64_t)tw*th<=destinationDepth->arr_length)z=(float*)destinationDepth->arr_body;
 int sw=arg(r,4),sh=arg(r,5),sx=arg(r,6),sy=arg(r,7),w=arg(r,8),h=arg(r,9),t=arg(r,10),dx=arg(r,11),dy=arg(r,12),cx=arg(r,13),cy=arg(r,14),cw=arg(r,15),ch=arg(r,16);
 if(!d||!z||!src||sw<1||sh<1||(int64_t)sw*sh>src->arr_length||sx<0||sy<0||w<1||h<1||(int64_t)sx+w>sw||(int64_t)sy+h>sh||t<0||t>8)return 0;
 float*source=sourceDepth&&sourceDepth->arr_length>=(int64_t)sw*sh?(float*)sourceDepth->arr_body:NULL,layer=crimson_stereo_layer();uint32_t*p=(uint32_t*)src->arr_body;int dw=t==8?w+80:(t>=4?h:w),dh=t>=4&&t<8?w:h,marked=0;
 for(int y=0;y<dh;y++){int yy=dy+y;if(yy<0||yy>=th||yy<cy||yy>=(int64_t)cy+ch)continue;for(int x=0;x<dw;x++){int xx=dx+x;if(xx<0||xx>=tw||xx<cx||xx>=(int64_t)cx+cw)continue;int u=x,v=y;switch(t){case 8:u=x*w/dw;break;case 1:v=h-1-y;break;case 2:u=w-1-x;break;case 3:u=w-1-x;v=h-1-y;break;case 4:u=y;v=x;break;case 5:u=y;v=h-1-x;break;case 6:u=w-1-y;v=x;break;case 7:u=w-1-y;v=h-1-x;break;}int i=(sy+v)*sw+sx+u;if(p[i]>>24){float value=source?source[i]:layer;z[yy*tw+xx]=value;if(value)marked=1;}}}if(marked)crimson_stereo_content_drawn();
 return 0;
}

static s32 stereoLayer(Runtime*r,JClass*c){(void)c;if(!arr(r,0))crimson_stereo_set_layer(arg(r,1),arg(r,2));return 0;}
static s32 stereoM3G(Runtime*r,JClass*c){(void)c;int w=arg(r,1),h=arg(r,2);uint32_t*d=target(r,w,h);Instance*depth=arr(r,3);Instance*shadow=arr(r,4);float*z=crimson_stereo_target(d);if(crimson_stereo_mode()&&shadow&&shadow->arr_length>=w*h)z=(float*)shadow->arr_body;if(z&&depth&&depth->arr_length>=w*h){int16_t*src=(int16_t*)depth->arr_body;for(int i=0;i<w*h;i++)z[i]=src[i]>=0&&src[i]<32767?1.f+(32767-src[i])/32767.f:0;crimson_stereo_models_drawn();}return 0;}

#include "software3d.h"
#include "m3g_span.h"
static s32 present(Runtime*r,JClass*c){(void)c;jthread_block_enter(r);crimson_publish(crimson_pixels);jthread_block_exit(r);crimson_stereo_finish_frame();return 0;}
extern int crimson_motion(int kind,int sample[3]);
static s32 motion(Runtime*r,JClass*c){(void)c;Instance*a=arr(r,1);int v[3],ok=a&&a->arr_length>=3&&crimson_motion(arg(r,0),v);if(ok)memcpy(a->arr_body,v,sizeof(v));push_int(r->stack,ok);return 0;}
static s32 buttons(Runtime*r,JClass*c){(void)c;push_int(r->stack,crimson_buttons());return 0;}
static s32 music(Runtime*r,JClass*c){(void)c;crimson_music(arg(r,0),arg(r,1),arg(r,2));return 0;}
static s32 audioLoad(Runtime*r,JClass*c){
 (void)c;Instance*b=arr(r,0);int result=-1;
 if(b&&b->arr_length>0&&b->arr_length<=1024*1024){
  int size=b->arr_length;void*copy=malloc(size);
  if(copy){memcpy(copy,b->arr_body,size);jthread_block_enter(r);result=crimson_audio_load(copy,size);jthread_block_exit(r);free(copy);}
 }
 push_int(r->stack,result);return 0;
}
static s32 audioControl(Runtime*r,JClass*c){
 (void)c;int id=arg(r,0),op=arg(r,1),value=arg(r,2);
 jthread_block_enter(r);int result=crimson_audio_control(id,op,value);jthread_block_exit(r);push_int(r->stack,result);return 0;
}
static s32 stage(Runtime*r,JClass*c){(void)c;crimson_stage(arg(r,0),arg(r,1));return 0;}
extern int crimson_gc_phase;
extern int crimson_gc_cycles;
extern int crimson_wait_errors;
int crimson_gc_wait_errors(void){return __atomic_load_n(&crimson_wait_errors,__ATOMIC_RELAXED);}
int crimson_gc_diagnostic(void){return __atomic_load_n(&crimson_gc_phase,__ATOMIC_RELAXED);}
int crimson_gc_completed(void){return __atomic_load_n(&crimson_gc_cycles,__ATOMIC_RELAXED);}
static s32 gcCycles(Runtime*r,JClass*c){(void)c;push_int(r->stack,crimson_gc_completed());return 0;}
static s32 snapshot(Runtime*r,JClass*c){Instance*a=jarray_create_by_type_index(r,crimson_width*crimson_height,DATATYPE_INT);if(a)memcpy(a->arr_body,crimson_pixels,crimson_width*crimson_height*4);push_ref(r->stack,a);return 0;}
static java_native_method methods[]={
 {"crimson/Native","readPixels","([III[IIIIIII[F[F)V",readPixels},
 {"crimson/Native","motion","(I[I)Z",motion},
 {"crimson/Native","m3gSpan","([I[SIII[F[I[I)V",m3gSpan},
 {"crimson/Native","microTransform","([F[F[F)V",microTransform},
 {"crimson/Native","microPose","([F[F[B[F)Z",microPose},
 {"crimson/Native","microBegin","()V",microBegin},
 {"crimson/Native","microBegin","(II)V",microBeginSize},
 {"crimson/Native","stereoMode","()I",stereoMode},
 {"crimson/Native","stereoRect","([FIIIIII)V",stereoRect},
 {"crimson/Native","stereoRegion","([III[IIIIIIIIIIIIII[F[F)V",stereoRegion},
 {"crimson/Native","stereoLayer","([III)V",stereoLayer},
 {"crimson/Native","stereoM3G","([III[S[F)V",stereoM3G},
 {"crimson/Native","microTriangle","([III[F[F[IIIIIIIII)V",microTriangle},
 {"crimson/Native","microMeshDepth","([III[F[F[I[[I[IIIIII[F)V",microMesh},
 {"crimson/Native","rgb","([III[IIIIIIIZIIII[F[F)V",rgb},
 {"crimson/Native","text","([III[JLjava/lang/String;IIIIIIII[F)V",text},
 {"crimson/Native","snapshot","()[I",snapshot},
 {"crimson/Native","decode","([BII[I)[I",decode},
 {"crimson/Native","fill","([IIIIIIII)V",fill},
 {"crimson/Native","blit","([III[IIIIIIIII)V",blit},
 {"crimson/Native","region","([III[IIIIIIIIIIIIII)V",region},
 {"crimson/Native","present","()V",present},
 {"crimson/Native","buttons","()I",buttons}
 ,{"crimson/Native","music","(III)V",music}
 ,{"crimson/Native","audioLoad","([B)I",audioLoad}
 ,{"crimson/Native","audioControl","(III)I",audioControl}
 ,{"crimson/Native","stage","(II)V",stage}
 ,{"crimson/Native","gcCycles","()I",gcCycles}
};
extern MethodInfo *pop_last_method;extern unsigned pop_method_calls;
static unsigned pop_sample_lock,pop_sample_disabled;
extern const char* crimson_launch_config(void);
static void*oom_reserve;
void crimson_release_oom_reserve(void){free(oom_reserve);oom_reserve=NULL;}
static MiniJVM*active_vm;static unsigned vm_lock;static int pause_requested;
void crimson_request_stop(void){while(__atomic_exchange_n(&vm_lock,1,__ATOMIC_ACQUIRE)){}if(active_vm)__atomic_store_n(&active_vm->crimson_stop,1,__ATOMIC_RELEASE);__atomic_store_n(&vm_lock,0,__ATOMIC_RELEASE);}
void crimson_pause_game(int paused){while(__atomic_exchange_n(&vm_lock,1,__ATOMIC_ACQUIRE)){}__atomic_store_n(&pause_requested,paused,__ATOMIC_RELEASE);if(active_vm)__atomic_store_n(&active_vm->crimson_paused,paused,__ATOMIC_RELEASE);__atomic_store_n(&vm_lock,0,__ATOMIC_RELEASE);}
static void active_set(MiniJVM*j){while(__atomic_exchange_n(&vm_lock,1,__ATOMIC_ACQUIRE)){}active_vm=j;if(j)__atomic_store_n(&j->crimson_paused,__atomic_load_n(&pause_requested,__ATOMIC_RELAXED),__ATOMIC_RELEASE);__atomic_store_n(&vm_lock,0,__ATOMIC_RELEASE);}
int crimson_run_game(const char*boot,const char*cp,const char*mainclass){
 __atomic_store_n(&pop_sample_disabled,0,__ATOMIC_RELAXED);
 __atomic_store_n(&pop_last_method,NULL,__ATOMIC_RELAXED);
 __atomic_store_n(&pop_method_calls,0,__ATOMIC_RELAXED);
 crimson_stage(40,0);
 int bootError=crimson_zip_cache_add(boot);if(bootError){crimson_stage(40,-bootError);jvm_printf("Cannot cache bootstrap JAR code=%d errno=%d path=%s\n",bootError,errno,boot);crimson_zip_cache_clear();return -40;}
#ifdef __3DS__
 const char separator=':';
#else
 const char separator=';';
#endif
 const char*part=cp;int jars=1;
 while(*part){const char*end=strchr(part,separator);size_t length=end?(size_t)(end-part):strlen(part);
  if(length>=4&&!memcmp(part+length-4,".jar",4)){
   char*path=malloc(length+1);if(!path){crimson_zip_cache_clear();return -41;}
   memcpy(path,part,length);path[length]=0;int error=crimson_zip_cache_add(path);free(path);
   if(error){jvm_printf("Cannot cache application JAR: %d\n",error);crimson_zip_cache_clear();return -42;}jars++;
  }
  if(!end)break;part=end+1;
 }
 jvm_printf("RAM cache ready: %d JARs, %lld bytes\n",jars,crimson_zip_cache_bytes());crimson_stage(41,jars);
 oom_reserve=malloc(128*1024);
 jvm_init_mem_alloc();MiniJVM*j=jvm_create();if(!j){crimson_zip_cache_clear();jvm_destroy_mem_alloc();crimson_release_oom_reserve();return -1;}j->jdwp_enable=0;j->max_heap_size=24*1024*1024;
 int result=jvm_init(j,(char*)boot,(char*)cp);
 const char*configuration=crimson_launch_config();if(configuration)sys_properties_set_c(j,"launcher.config",configuration);
#ifndef __3DS__
 const char*saveDir=getenv("CRIMSON_HOST_SAVE_DIR");
 sys_properties_set_c(j,"pop.saves",saveDir?saveDir:"work/vm-saves");
#endif
 active_set(j);
 if(!result){native_reg_lib(j,methods,sizeof(methods)/sizeof(methods[0]));ArrayList*a=arraylist_create(0);result=call_main(j,(char*)mainclass,a);crimson_stage(70,result);arraylist_destroy(a);}
 while(__atomic_exchange_n(&pop_sample_lock,1,__ATOMIC_ACQUIRE)){}
 __atomic_store_n(&pop_sample_disabled,1,__ATOMIC_RELAXED);
 __atomic_store_n(&pop_sample_lock,0,__ATOMIC_RELEASE);
 active_set(NULL);crimson_stage(71,result);set_jvm_state(j,JVM_STATUS_STOPED);thread_stop_all(j);jvm_destroy(j);crimson_stage(72,result);jvm_printf("RAM cache hits: %lld\n",crimson_zip_cache_hits());crimson_zip_cache_clear();jvm_destroy_mem_alloc();micro_reset();crimson_release_oom_reserve();__atomic_store_n(&pause_requested,0,__ATOMIC_RELEASE);return result;
}

extern MethodInfo *pop_last_method;
extern unsigned pop_method_calls;
unsigned pop_vm_sample(char *out,unsigned capacity){
 unsigned calls=__atomic_load_n(&pop_method_calls,__ATOMIC_RELAXED);
 if(__atomic_exchange_n(&pop_sample_lock,1,__ATOMIC_ACQUIRE)){snprintf(out,capacity,"sample busy");return calls;}
 MethodInfo*m=__atomic_load_n(&pop_last_method,__ATOMIC_RELAXED);
 if(!__atomic_load_n(&pop_sample_disabled,__ATOMIC_RELAXED)&&m)snprintf(out,capacity,"%s.%s%s",utf8_cstr(m->_this_class->name),utf8_cstr(m->name),utf8_cstr(m->descriptor));else snprintf(out,capacity,"bootstrap/ended");
 __atomic_store_n(&pop_sample_lock,0,__ATOMIC_RELEASE);return calls;
}
