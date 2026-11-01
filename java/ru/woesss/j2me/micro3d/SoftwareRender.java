package ru.woesss.j2me.micro3d;
import javax.microedition.lcdui.Graphics;
public class SoftwareRender {
 private static boolean reported;
 public static void draw(FigureImpl figure,TextureImpl[] textures,float[] transform,float[] projection,Graphics graphics){
 Model model=figure.model;if(model==null)throw new IllegalStateException("disposed figure");int count=model.originalVertices.capacity()/3;float[] transformed=new float[count*3];
 for(int i=0;i<count;i++){float x=model.vertices.get(i*3),y=model.vertices.get(i*3+1),z=model.vertices.get(i*3+2);for(int row=0;row<3;row++)transformed[i*3+row]=transform[row*4]*x+transform[row*4+1]*y+transform[row*4+2]*z+transform[row*4+3];}
 float[] triangle=new float[15];int rendered=0;for(int pass=0;pass<2;pass++)for(Model.Polygon poly:pass==0?model.polygonsT:model.polygonsC){if((poly.pattern&figure.getPattern())!=poly.pattern)continue;TextureImpl tex=textures!=null&&poly.face>=0&&poly.face<textures.length?textures[poly.face]:null;byte[] material=poly.texCoords;if(material==null)continue;
 for(int t=0;t<poly.indices.length;t+=3){for(int v=0;v<3;v++){int index=poly.indices[t+v];for(int axis=0;axis<3;axis++)triangle[v*5+axis]=transformed[index*3+axis];triangle[v*5+3]=material[(t+v)*5]&255;triangle[v*5+4]=material[(t+v)*5+1]&255;}
 int color=0xff000000|((material[t*5]&255)<<16)|((material[t*5+1]&255)<<8)|(material[t*5+2]&255);graphics.microTriangle(triangle,projection,tex==null?null:tex.pixels,tex==null?0:tex.width,tex==null?0:tex.height,color,tex==null?0:(material[t*5+4]&1));rendered++;}}
 if(!reported&&rendered>0){reported=true;System.out.println("MICRO3D_RENDER triangles="+rendered);}
 }
}
