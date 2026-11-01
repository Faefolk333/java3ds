package ru.woesss.j2me.micro3d;
import javax.microedition.lcdui.Graphics;
public class SoftwareRender {
 public static boolean nativeBatch=true;
 private static boolean reported;
 private static int[] faces(Model model){
  if(model.softwareFaces!=null)return model.softwareFaces;
  int count=0;for(int pass=0;pass<2;pass++)for(Model.Polygon p:pass==0?model.polygonsT:model.polygonsC)if(p.texCoords!=null)count+=p.indices.length/3;
  int[] data=new int[count*13];int at=0;
  for(int pass=0;pass<2;pass++)for(Model.Polygon p:pass==0?model.polygonsT:model.polygonsC){byte[] m=p.texCoords;if(m==null)continue;for(int t=0;t<p.indices.length;t+=3){
   for(int v=0;v<3;v++)data[at+v]=p.indices[t+v];
   for(int v=0;v<3;v++){data[at+3+v*2]=m[(t+v)*5]&255;data[at+4+v*2]=m[(t+v)*5+1]&255;}
   data[at+9]=0xff000000|((m[t*5]&255)<<16)|((m[t*5+1]&255)<<8)|(m[t*5+2]&255);data[at+10]=(m[t*5+4]&1)|p.blendMode;data[at+11]=p.face;data[at+12]=p.pattern;at+=13;
  }}return model.softwareFaces=data;
 }
 public static void draw(FigureImpl figure,TextureImpl[] textures,float[] transform,float[] projection,Graphics graphics){
 Model model=figure.model;if(model==null)throw new IllegalStateException("disposed figure");int count=model.originalVertices.capacity()/3;if(figure.transformed==null||figure.transformed.length!=count*3)figure.transformed=new float[count*3];float[] transformed=figure.transformed;
 crimson.Native.microTransform(model.vertices.array(),transformed,transform);
 if(nativeBatch){int n=textures==null?0:textures.length;if(figure.texturePixels==null||figure.texturePixels.length!=n){figure.texturePixels=new int[n][];figure.textureSizes=new int[n*2];}for(int i=0;i<n;i++){TextureImpl t=textures[i];figure.texturePixels[i]=t==null?null:t.pixels;figure.textureSizes[i*2]=t==null?0:t.width;figure.textureSizes[i*2+1]=t==null?0:t.height;}graphics.microMesh(transformed,projection,faces(model),figure.texturePixels,figure.textureSizes,figure.getPattern());return;}
 float[] triangle=figure.triangle;int rendered=0;for(int pass=0;pass<2;pass++)for(Model.Polygon poly:pass==0?model.polygonsT:model.polygonsC){if((poly.pattern&figure.getPattern())!=poly.pattern)continue;TextureImpl tex=textures!=null&&poly.face>=0&&poly.face<textures.length?textures[poly.face]:null;byte[] material=poly.texCoords;if(material==null)continue;
 for(int t=0;t<poly.indices.length;t+=3){for(int v=0;v<3;v++){int index=poly.indices[t+v];for(int axis=0;axis<3;axis++)triangle[v*5+axis]=transformed[index*3+axis];triangle[v*5+3]=material[(t+v)*5]&255;triangle[v*5+4]=material[(t+v)*5+1]&255;}
 int color=0xff000000|((material[t*5]&255)<<16)|((material[t*5+1]&255)<<8)|(material[t*5+2]&255);graphics.microTriangle(triangle,projection,tex==null?null:tex.pixels,tex==null?0:tex.width,tex==null?0:tex.height,color,(material[t*5+4]&1)|poly.blendMode);rendered++;}}
 if(!reported&&rendered>0){reported=true;System.out.println("MICRO3D_RENDER triangles="+rendered);}
 }
}
