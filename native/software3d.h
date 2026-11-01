#pragma once
#include <math.h>
// A bounded software depth buffer, shared by the single active MIDP scene.
static float*micro_depth;
static int micro_depth_count;
static void micro_reset(void){free(micro_depth);micro_depth=NULL;micro_depth_count=0;}
static s32 micro_begin(Runtime*r,int count){if(count<1||count>512*512)return 0;if(!micro_depth)micro_depth=malloc(512*512*sizeof(float));if(!micro_depth){extern void crimson_release_oom_reserve(void);crimson_release_oom_reserve();push_ref(r->stack,exception_create(JVM_ERROR_OUTOFMEMORY,r));return RUNTIME_STATUS_EXCEPTION;}micro_depth_count=count;memset(micro_depth,0,count*sizeof(float));return 0;}
static s32 microBegin(Runtime*r,JClass*c){(void)c;return micro_begin(r,512*512);}
static s32 microBeginSize(Runtime*r,JClass*c){(void)c;int w=arg(r,0),h=arg(r,1);if(w<1||h<1||w>512*512/h)return 0;return micro_begin(r,w*h);}
static float micro_edge(float ax,float ay,float bx,float by,float x,float y){return (x-ax)*(by-ay)-(y-ay)*(bx-ax);}
static void micro_raster(uint32_t*d,int dw,int dh,float v[3][5],const float*p,const uint32_t*texture,int tw,int th,uint32_t color,int flags,int cx,int cy,int cw,int ch,float*providedDepth){
 float sx[3],sy[3],q[3];for(int i=0;i<3;i++){q[i]=p[6]?1.f/v[i][2]:1.f;sx[i]=p[0]+v[i][0]*p[4]*q[i];sy[i]=p[1]+v[i][1]*p[5]*q[i];if(!isfinite(sx[i])||!isfinite(sy[i]))return;}
 float area=micro_edge(sx[0],sy[0],sx[1],sy[1],sx[2],sy[2]);if(!isfinite(area)||fabsf(area)<.00001f)return;
 float lx=fminf(sx[0],fminf(sx[1],sx[2])),rx=fmaxf(sx[0],fmaxf(sx[1],sx[2])),ly=fminf(sy[0],fminf(sy[1],sy[2])),ry=fmaxf(sy[0],fmaxf(sy[1],sy[2]));
 // Clamp in floating point before integer conversion, including extreme near-plane projections.
 if(lx>dw||ly>dh||rx<0||ry<0)return;
 int x0=(int)fmaxf(0,fmaxf(cx,lx)),x1=(int)fminf(dw-1,fminf((float)cx+cw-1,rx));int y0=(int)fmaxf(0,fmaxf(cy,ly)),y1=(int)fminf(dh-1,fminf((float)cy+ch-1,ry));
 if(lx>dw||ly>dh||rx<0||ry<0||x0>x1||y0>y1)return;
 float*stereo=providedDepth?providedDepth:crimson_stereo_target(d);
 for(int y=y0;y<=y1;y++)for(int x=x0;x<=x1;x++){float ea=micro_edge(sx[1],sy[1],sx[2],sy[2],x+.5f,y+.5f),eb=micro_edge(sx[2],sy[2],sx[0],sy[0],x+.5f,y+.5f);if(area>0?(ea<0||eb<0):(ea>0||eb>0))continue;float a=ea/area,b=eb/area,c=1-a-b;if(a<0||b<0||c<0)continue;float inv=a*q[0]+b*q[1]+c*q[2];if(inv<=0)continue;float depth=p[6]?inv:1.f/(1000000.f+a*v[0][2]+b*v[1][2]+c*v[2][2]);int index=y*dw+x;if(depth<=micro_depth[index])continue;uint32_t pixel=color;
  if(texture){int u=(int)((a*v[0][3]*q[0]+b*v[1][3]*q[1]+c*v[2][3]*q[2])/inv),t=(int)((a*v[0][4]*q[0]+b*v[1][4]*q[1]+c*v[2][4]*q[2])/inv);if(u<0)u=0;if(t<0)t=0;if(u>=tw)u=tw-1;if(t>=th)t=th-1;pixel=texture[t*tw+u];if(!(flags&1))pixel|=0xff000000;}
  if(!(pixel>>24))continue;
  int blend=flags&6;if((blend==4||blend==6)&&!(pixel&0xffffff))continue;if(!blend){d[index]=over(d[index],pixel,1);micro_depth[index]=depth;}else{uint32_t old=d[index],out=0xff000000;for(int k=0;k<24;k+=8){int a=(old>>k)&255,b=(pixel>>k)&255,v=blend==2?(a+b)/2:blend==4?a+b:a-b;if(v<0)v=0;if(v>255)v=255;out|=(uint32_t)v<<k;}d[index]=out;}if(stereo)stereo[index]=depth;
 }
}
static void micro_draw(uint32_t*d,int dw,int dh,float input[3][5],float*p,const uint32_t*tex,int tw,int th,uint32_t color,int flags,int cx,int cy,int cw,int ch,float*providedDepth){
 if(providedDepth||crimson_stereo_target(d))crimson_stereo_models_drawn();
 float clipped[6][5];int count=0;
 if(p[6]){float nearPlane=p[2]>0?p[2]:1.f;for(int i=0;i<3;i++){float*a=input[i],*b=input[(i+1)%3];int ina=a[2]>=nearPlane,inb=b[2]>=nearPlane;if(ina)memcpy(clipped[count++],a,5*sizeof(float));if(ina!=inb){float t=(nearPlane-a[2])/(b[2]-a[2]);for(int k=0;k<5;k++)clipped[count][k]=a[k]+t*(b[k]-a[k]);count++;}}}else{memcpy(clipped,input,15*sizeof(float));count=3;}
 for(int i=1;i+1<count;i++){float v[3][5];memcpy(v[0],clipped[0],20);memcpy(v[1],clipped[i],20);memcpy(v[2],clipped[i+1],20);micro_raster(d,dw,dh,v,p,tex,tw,th,color,flags,cx,cy,cw,ch,providedDepth);}
}
static s32 microTriangle(Runtime*r,JClass*c){
 (void)c;int dw=arg(r,1),dh=arg(r,2);uint32_t*d=target(r,dw,dh);Instance*verts=arr(r,3),*proj=arr(r,4),*tex=arr(r,5);int tw=arg(r,6),th=arg(r,7);if(!micro_depth||!d||!verts||!proj||verts->arr_length<15||proj->arr_length<7||dw*dh>micro_depth_count)return 0;
 if(tex&&(tw<1||th<1||tw>1024||th>1024||tex->arr_length<tw*th))return 0;
 float input[3][5],p[7];memcpy(input,verts->arr_body,sizeof(input));memcpy(p,proj->arr_body,sizeof(p));for(int i=0;i<7;i++)if(!isfinite(p[i]))return 0;for(int i=0;i<15;i++)if(!isfinite(((float*)input)[i]))return 0;
 micro_draw(d,dw,dh,input,p,tex?(uint32_t*)tex->arr_body:NULL,tw,th,(uint32_t)arg(r,8),(proj->arr_length>7&&((float*)proj->arr_body)[7]==0?arg(r,9)&1:arg(r,9)),arg(r,10),arg(r,11),arg(r,12),arg(r,13),NULL);return 0;
}

static s32 microMesh(Runtime*r,JClass*c){
 (void)c;int dw=arg(r,1),dh=arg(r,2);uint32_t*d=target(r,dw,dh);Instance*vertices=arr(r,3),*projection=arr(r,4),*faces=arr(r,5),*textures=arr(r,6),*sizes=arr(r,7);
 if(!d||!micro_depth||dw*dh>micro_depth_count||!vertices||!projection||projection->arr_length<7||!faces||faces->arr_length%13)return 0;
 Instance*shadow=arr(r,13);float*providedDepth=crimson_stereo_mode()&&shadow&&shadow->arr_length>=dw*dh?(float*)shadow->arr_body:NULL;float*p=(float*)projection->arr_body,*xyz=(float*)vertices->arr_body;for(int k=0;k<7;k++)if(!isfinite(p[k]))return 0;
 int*data=(int*)faces->arr_body,pattern=arg(r,8);for(int pass=0;pass<2;pass++)for(int t=0;t<faces->arr_length;t+=13){int*f=data+t;int blend=projection->arr_length>7&&p[7]==0?0:f[10]&6;if((blend!=0)!=pass)continue;if((f[12]&pattern)!=f[12])continue;float input[3][5];int valid=1;for(int v=0;v<3;v++){int index=f[v];if(index<0||index>=vertices->arr_length/3){valid=0;break;}for(int k=0;k<3;k++){input[v][k]=xyz[index*3+k];if(!isfinite(input[v][k]))valid=0;}input[v][3]=f[3+v*2];input[v][4]=f[4+v*2];}if(!valid)continue;
  const uint32_t*texture=NULL;int tw=0,th=0,face=f[11];if(textures&&sizes&&face>=0&&face<textures->arr_length&&face<sizes->arr_length/2){Instance*image=(Instance*)(intptr_t)jarray_get_field(textures,face);tw=((int*)sizes->arr_body)[face*2];th=((int*)sizes->arr_body)[face*2+1];if(image&&tw>0&&th>0&&tw<=1024&&th<=1024&&image->arr_length>=tw*th)texture=(uint32_t*)image->arr_body;}
  micro_draw(d,dw,dh,input,p,texture,tw,th,(uint32_t)f[9],(projection->arr_length>7&&p[7]==0?f[10]&1:f[10]),arg(r,9),arg(r,10),arg(r,11),arg(r,12),providedDepth);
 }return 0;
}

static s32 microTransform(Runtime*r,JClass*c){
 (void)c;Instance*src=arr(r,0),*dst=arr(r,1),*matrix=arr(r,2);if(!src||!dst||!matrix||matrix->arr_length<12||dst->arr_length>src->arr_length||dst->arr_length%3)return 0;
 float*s=(float*)src->arr_body,*d=(float*)dst->arr_body,*m=(float*)matrix->arr_body;for(int i=0;i+2<dst->arr_length;i+=3){float x=s[i],y=s[i+1],z=s[i+2];for(int k=0;k<3;k++)d[i+k]=m[k*4]*x+m[k*4+1]*y+m[k*4+2]*z+m[k*4+3];}return 0;
}
static void micro_mul(float*out,const float*a,const float*b){for(int row=0;row<3;row++)for(int col=0;col<4;col++)out[row*4+col]=a[row*4]*b[col]+a[row*4+1]*b[col+4]+a[row*4+2]*b[col+8]+(col==3?a[row*4+3]:0);}
static s32 microPose(Runtime*r,JClass*c){
 (void)c;Instance*src=arr(r,0),*dst=arr(r,1),*bone=arr(r,2),*action=arr(r,3);int ok=0;float*matrices=NULL;
 if(!src||!dst||!bone||dst->arr_length<src->arr_length||bone->arr_length%56||bone->arr_length>560000)goto finish;
 int count=bone->arr_length/56,vertex=0;matrices=malloc((size_t)count*12*sizeof(float));if(count&&!matrices)goto finish;
 for(int i=0;i<count;i++){
  const unsigned char*b=(unsigned char*)bone->arr_body+i*56;int32_t n,parent;memcpy(&n,b,4);memcpy(&parent,b+4,4);float*m=matrices+i*12,temp[12];memcpy(m,b+8,48);
  if(n<0||parent>=i||n>(src->arr_length/3)-vertex)goto finish;
  if(parent>=0){micro_mul(temp,matrices+parent*12,m);memcpy(m,temp,48);}
  if(action&&i*12+12<=action->arr_length){micro_mul(temp,m,(float*)action->arr_body+i*12);memcpy(m,temp,48);}
  float*s=(float*)src->arr_body,*d=(float*)dst->arr_body;for(int j=0;j<n;j++,vertex++){int o=vertex*3;float x=s[o],y=s[o+1],z=s[o+2];for(int row=0;row<3;row++)d[o+row]=m[row*4]*x+m[row*4+1]*y+m[row*4+2]*z+m[row*4+3];}
 }
 ok=1;
 finish:free(matrices);push_int(r->stack,ok);return 0;
}
