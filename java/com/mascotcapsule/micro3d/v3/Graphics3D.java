package com.mascotcapsule.micro3d.v3;
import javax.microedition.lcdui.Graphics;import ru.woesss.j2me.micro3d.*;
public class Graphics3D {
 public static final int COMMAND_PARALLEL_SCALE=0x90000000,COMMAND_PARALLEL_SIZE=0x91000000,COMMAND_PERSPECTIVE_FOV=0x92000000,COMMAND_PERSPECTIVE_WH=0x93000000;
 private Graphics graphics;
 public void bind(Graphics g){if(graphics!=null||g==null)throw new IllegalStateException();graphics=g;g.microBegin();}
 public void release(Graphics g){if(g!=graphics)throw new IllegalArgumentException();graphics=null;}
 public void flush(){}
 public void dispose(){graphics=null;}
 public void drawFigure(Figure f,int x,int y,FigureLayout l,Effect3D e){renderFigure(f,x,y,l,e);}
 public void renderFigure(Figure f,int x,int y,FigureLayout l,Effect3D e){if(graphics==null)throw new IllegalStateException("not bound");AffineTrans a=l.affine;float[] m={a.m00/4096f,a.m01/4096f,a.m02/4096f,a.m03,a.m10/4096f,a.m11/4096f,a.m12/4096f,a.m13,a.m20/4096f,a.m21/4096f,a.m22/4096f,a.m23};float fx=1,fy=1;boolean perspective=l.projection==COMMAND_PERSPECTIVE_FOV||l.projection==COMMAND_PERSPECTIVE_WH;
 if(l.projection==COMMAND_PERSPECTIVE_FOV){fy=(float)(crimson.GameMain.width/2.0/Math.tan(l.angle*Math.PI/4096));fx=fy;}else if(l.projection==COMMAND_PERSPECTIVE_WH){fx=(float)crimson.GameMain.width*l.near*4096f/l.perspectiveWidth;fy=(float)crimson.GameMain.height*l.near*4096f/l.perspectiveHeight;}else if(l.projection==COMMAND_PARALLEL_SIZE){fx=(float)crimson.GameMain.width/l.parallelWidth;fy=(float)crimson.GameMain.height/l.parallelHeight;}else{fx=l.scaleX/4096f;fy=l.scaleY/4096f;}
 float[] projection={x+l.centerX,y+l.centerY,perspective?Math.max(1,l.near):0,l.far,fx,fy,perspective?1:0};TextureImpl[] textures=null;if(f.textures!=null){textures=new TextureImpl[f.textures.length];for(int i=0;i<textures.length;i++)textures[i]=f.textures[i].impl;}SoftwareRender.draw(f.impl,textures,m,projection,graphics);
 }
}
