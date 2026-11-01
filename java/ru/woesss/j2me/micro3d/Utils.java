package ru.woesss.j2me.micro3d;
import java.io.*;import java.nio.*;
class Utils {
 static final String TAG="Micro3D";
 static byte[] resource(String name)throws IOException {InputStream in=crimson.Resources.open(Utils.class,name.startsWith("/")?name:"/"+name);if(in==null)throw new IOException(name);ByteArrayOutputStream out=new ByteArrayOutputStream();try{byte[] b=new byte[1024];int n;while((n=in.read(b))!=-1)out.write(b,0,n);}finally{in.close();}return out.toByteArray();}
 static void transform(FloatBuffer src,FloatBuffer dst,FloatBuffer sn,FloatBuffer dn,ByteBuffer bones,float[] action){
  int count=bones.capacity()/56,vertex=0;float[][] matrices=new float[count][12];
  for(int i=0;i<count;i++){int n=bones.getInt(i*56),parent=bones.getInt(i*56+4);float[] m=matrices[i];for(int j=0;j<12;j++)m[j]=bones.getFloat(i*56+8+j*4);if(parent>=0){if(parent>=i)throw new IllegalArgumentException("bone parent");m=multiply(matrices[parent],m);}if(action!=null&&i*12+12<=action.length){float[] a=new float[12];System.arraycopy(action,i*12,a,0,12);m=multiply(m,a);}matrices[i]=m;
   for(int j=0;j<n;j++,vertex++){int o=vertex*3;if(o+2>=src.capacity())throw new IllegalArgumentException("bone count");float x=src.get(o),y=src.get(o+1),z=src.get(o+2);for(int row=0;row<3;row++)dst.put(o+row,m[row*4]*x+m[row*4+1]*y+m[row*4+2]*z+m[row*4+3]);}
  }
 }
 private static float[] multiply(float[] a,float[] b){float[] c=new float[12];for(int r=0;r<3;r++)for(int col=0;col<4;col++)c[r*4+col]=a[r*4]*b[col]+a[r*4+1]*b[4+col]+a[r*4+2]*b[8+col]+(col==3?a[r*4+3]:0);return c;}
}
