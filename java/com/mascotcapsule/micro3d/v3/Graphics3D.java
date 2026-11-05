package com.mascotcapsule.micro3d.v3;
import javax.microedition.lcdui.Graphics;import ru.woesss.j2me.micro3d.*;
public class Graphics3D {
 public static final int COMMAND_PARALLEL_SCALE=0x90000000,COMMAND_PARALLEL_SIZE=0x91000000,COMMAND_PERSPECTIVE_FOV=0x92000000,COMMAND_PERSPECTIVE_WH=0x93000000;
 private Graphics graphics;
 public static final int PRIMITVE_TRIANGLES=0x3000000,PRIMITVE_QUADS=0x4000000,PDATA_COLOR_PER_COMMAND=1024,PDATA_COLOR_PER_FACE=2048,PDATA_TEXURE_COORD=12288,PATTR_COLORKEY=16,PATTR_BLEND_HALF=32,PATTR_BLEND_ADD=64,PATTR_BLEND_SUB=96;
 private static class Batch {int kind,count;float[] source,vertices;int[] faces;int[][] texture=new int[1][];int[] sizes=new int[2];Batch(int kind,int count){this.kind=kind;this.count=count;source=new float[count*(kind==3?9:12)];vertices=new float[source.length];faces=new int[count*(kind==3?13:26)];}}
 private final Batch[] batches=new Batch[16];private int nextBatch;
 private final float[] primitiveTransform=new float[12],primitiveProjection=new float[8];
 public void renderPrimitives(Texture texture,int x,int y,FigureLayout layout,Effect3D effect,int command,int count,int[] vertices,int[] normals,int[] coordinates,int[] colors){
  if(graphics==null)throw new IllegalStateException("not bound");if(layout==null||effect==null||vertices==null||normals==null||coordinates==null||colors==null)throw new NullPointerException();
  int kind=(command>>>24)&7;if(command<0||count<1||count>255||(kind!=3&&kind!=4))throw new IllegalArgumentException("Unsupported primitive kind");
  int perFace=kind==3?3:4,vertexCount=count*perFace,colorMode=command&3072;boolean textured=(command&12288)==12288;
  if(vertices.length<vertexCount*3||textured&&coordinates.length<vertexCount*2||!textured&&colorMode==1024&&colors.length<1||!textured&&colorMode==2048&&colors.length<count)throw new IllegalArgumentException("Primitive data too short");
  if(textured&&texture==null||!textured&&colorMode==0)return;
  Batch batch=null;for(int i=0;i<batches.length;i++)if(batches[i]!=null&&batches[i].kind==kind&&batches[i].count==count){batch=batches[i];break;}
  if(batch==null){batch=new Batch(kind,count);batches[nextBatch++%batches.length]=batch;}
  AffineTrans a=layout.affine;float[] m=primitiveTransform;m[0]=a.m00/4096f;m[1]=a.m01/4096f;m[2]=a.m02/4096f;m[3]=a.m03;m[4]=a.m10/4096f;m[5]=a.m11/4096f;m[6]=a.m12/4096f;m[7]=a.m13;m[8]=a.m20/4096f;m[9]=a.m21/4096f;m[10]=a.m22/4096f;m[11]=a.m23;
  float fx=1,fy=1;boolean perspective=layout.projection==COMMAND_PERSPECTIVE_FOV||layout.projection==COMMAND_PERSPECTIVE_WH;
  if(layout.projection==COMMAND_PERSPECTIVE_FOV){fy=(float)(graphics.getTargetWidth()/2.0/Math.tan(layout.angle*Math.PI/4096));fx=fy;}else if(layout.projection==COMMAND_PERSPECTIVE_WH){fx=(float)graphics.getTargetWidth()*layout.near*4096f/layout.perspectiveWidth;fy=(float)graphics.getTargetHeight()*layout.near*4096f/layout.perspectiveHeight;}else if(layout.projection==COMMAND_PARALLEL_SIZE){fx=(float)graphics.getTargetWidth()/layout.parallelWidth;fy=(float)graphics.getTargetHeight()/layout.parallelHeight;}else{fx=layout.scaleX/4096f;fy=layout.scaleY/4096f;}
  float[] p=primitiveProjection;p[0]=x+layout.centerX;p[1]=y+layout.centerY;p[2]=perspective?Math.max(1,layout.near):0;p[3]=layout.far;p[4]=fx;p[5]=fy;p[6]=perspective?1:0;p[7]=effect.isTransparency()?1:0;
  for(int i=0;i<batch.source.length;i++)batch.source[i]=vertices[i];crimson.Native.microTransform(batch.source,batch.vertices,m);
  int blend=(command&96)==32?2:(command&96)==64?4:(command&96)==96?6:0,flags=((command&16)!=0?1:0)|(effect.isTransparency()?blend:0);
  for(int face=0;face<count;face++)for(int triangle=0;triangle<(kind==3?1:2);triangle++){
   int at=(face*(kind==3?1:2)+triangle)*13;
   for(int v=0;v<3;v++){int corner=triangle==0?v:v==0?3:v==1?0:2;int index=face*perFace+corner;batch.faces[at+v]=index;batch.faces[at+3+v*2]=textured?coordinates[index*2]:0;batch.faces[at+4+v*2]=textured?coordinates[index*2+1]:0;}
   batch.faces[at+9]=0xff000000|(textured?0xffffff:colors[colorMode==1024?0:face]);batch.faces[at+10]=flags;batch.faces[at+11]=textured?0:-1;batch.faces[at+12]=0;
  }
  if(textured){batch.texture[0]=texture.impl.pixels;batch.sizes[0]=texture.impl.width;batch.sizes[1]=texture.impl.height;}
  graphics.microMesh(batch.vertices,p,batch.faces,textured?batch.texture:null,textured?batch.sizes:null,0);
 }
 public void bind(Graphics g){if(graphics!=null||g==null)throw new IllegalStateException();graphics=g;g.microBegin();}
 public void release(Graphics g){if(g!=graphics)throw new IllegalArgumentException();graphics=null;}
 public void flush(){}
 public void dispose(){graphics=null;}
 public void drawFigure(Figure f,int x,int y,FigureLayout l,Effect3D e){renderFigure(f,x,y,l,e);}
 public void renderFigure(Figure f,int x,int y,FigureLayout l,Effect3D e){if(graphics==null)throw new IllegalStateException("not bound");AffineTrans a=l.affine;float[] m={a.m00/4096f,a.m01/4096f,a.m02/4096f,a.m03,a.m10/4096f,a.m11/4096f,a.m12/4096f,a.m13,a.m20/4096f,a.m21/4096f,a.m22/4096f,a.m23};float fx=1,fy=1;boolean perspective=l.projection==COMMAND_PERSPECTIVE_FOV||l.projection==COMMAND_PERSPECTIVE_WH;
 if(l.projection==COMMAND_PERSPECTIVE_FOV){fy=(float)(graphics.getTargetWidth()/2.0/Math.tan(l.angle*Math.PI/4096));fx=fy;}else if(l.projection==COMMAND_PERSPECTIVE_WH){fx=(float)graphics.getTargetWidth()*l.near*4096f/l.perspectiveWidth;fy=(float)graphics.getTargetHeight()*l.near*4096f/l.perspectiveHeight;}else if(l.projection==COMMAND_PARALLEL_SIZE){fx=(float)graphics.getTargetWidth()/l.parallelWidth;fy=(float)graphics.getTargetHeight()/l.parallelHeight;}else{fx=l.scaleX/4096f;fy=l.scaleY/4096f;}
 float[] projection={x+l.centerX,y+l.centerY,perspective?Math.max(1,l.near):0,l.far,fx,fy,perspective?1:0,e==null||e.isTransparency()?1:0};TextureImpl[] textures=null;if(f.textures!=null){textures=new TextureImpl[f.textures.length];for(int i=0;i<textures.length;i++)textures[i]=f.textures[i].impl;}SoftwareRender.draw(f.impl,textures,m,projection,graphics);
 }
}
