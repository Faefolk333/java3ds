import crimson.Native;
public class MicroBatchCheck {
 static final int W=64,H=64;
 static void draw(int[] dst,float[] xyz,float[] p,int[] faces,int[][] tex,int[] sizes,int pattern,boolean batch){
  Native.fill(dst,W,H,0,0,W,H,0xff102030);Native.microBegin(W,H);
  if(batch){Native.microMesh(dst,W,H,xyz,p,faces,tex,sizes,pattern,2,3,59,58);return;}
  float[] v=new float[15];for(int t=0;t<faces.length;t+=13){if((faces[t+12]&pattern)!=faces[t+12])continue;for(int k=0;k<3;k++){int index=faces[t+k];for(int a=0;a<3;a++)v[k*5+a]=xyz[index*3+a];v[k*5+3]=faces[t+3+k*2];v[k*5+4]=faces[t+4+k*2];}int f=faces[t+11];Native.microTriangle(dst,W,H,v,p,f<0?null:tex[f],f<0?0:sizes[f*2],f<0?0:sizes[f*2+1],faces[t+9],faces[t+10],2,3,59,58);}
 }
 public static void main(String[]args)throws Exception{
  int[] a=new int[W*H],b=new int[W*H],faces=new int[80*13];float[] xyz=new float[80*9],p={32,32,1,100,30,30,1};int[][] tex={{0,0xffe02010,0x80ffffff,0xff30b080}};int[] sizes={2,2};
  for(int i=0;i<80;i++){int at=i*13,v=i*9;float x=(i%8-4)*.2f,y=(i/8-5)*.2f;xyz[v]=x;xyz[v+1]=y;xyz[v+2]=i%3==0?.5f:2;xyz[v+3]=x+.8f;xyz[v+4]=y;xyz[v+5]=2;xyz[v+6]=x;xyz[v+7]=y+.8f;xyz[v+8]=2;for(int k=0;k<3;k++)faces[at+k]=i*3+k;faces[at+3]=0;faces[at+4]=0;faces[at+5]=1;faces[at+6]=0;faces[at+7]=0;faces[at+8]=1;faces[at+9]=0xff800000|i*137;faces[at+10]=1;faces[at+11]=i%2==0?0:-1;faces[at+12]=i%3==0?2:0;}
  for(int mode=0;mode<2;mode++){p[6]=mode;for(int pattern=0;pattern<4;pattern++){draw(a,xyz,p,faces,tex,sizes,pattern,false);draw(b,xyz,p,faces,tex,sizes,pattern,true);for(int i=0;i<a.length;i++)if(a[i]!=b[i])throw new Exception("batch pixel mismatch "+mode+"/"+pattern+" at "+i);}}
  p[6]=1;long start=System.currentTimeMillis();for(int i=0;i<100;i++)draw(a,xyz,p,faces,tex,sizes,2,false);long slow=System.currentTimeMillis()-start;start=System.currentTimeMillis();for(int i=0;i<100;i++)draw(b,xyz,p,faces,tex,sizes,2,true);long fast=System.currentTimeMillis()-start;
  // Corrupt indices/NaN must be bounded rather than becoming a native crash.
  faces[0]=Integer.MAX_VALUE;xyz[9]=Float.NaN;draw(b,xyz,p,faces,tex,sizes,2,true);
  System.out.println("MICRO_BATCH_PASS exact_pixels perspective parallel clipping alpha patterns invalid_indices reference_ms="+slow+" batch_ms="+fast);
 }
}
