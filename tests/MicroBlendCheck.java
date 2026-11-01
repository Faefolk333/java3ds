import crimson.Native;
public class MicroBlendCheck {
 static final int W=32,H=32;
 static float[] p={16,16,1,100,8,8,0,1};
 static float[] triangle={-1,-1,2,0,0, 1,-1,2,0,0, 0,1,2,0,0};
 static int pixel(int flags,int src,int dst){int[] a=new int[W*H];Native.fill(a,W,H,0,0,W,H,dst);Native.microBegin(W,H);Native.microTriangle(a,W,H,triangle,p,new int[]{src},1,1,0xffffffff,flags,0,0,W,H);return a[16*W+16];}
 static void eq(int got,int expected)throws Exception{if(got!=expected)throw new Exception("blend got="+Integer.toHexString(got)+" expected="+Integer.toHexString(expected));}
 public static void main(String[] args)throws Exception{
  eq(pixel(0,0xff203040,0xff102030),0xff203040);
  eq(pixel(2,0xff203040,0xff102030),0xff182838);
  eq(pixel(4,0xff203040,0xff102030),0xff305070);
  eq(pixel(4,0xfff0f0f0,0xff203040),0xffffffff);
  eq(pixel(6,0xff203040,0xff304050),0xff101010);
  eq(pixel(6,0xff203040,0xff102030),0xff000000);
  eq(pixel(1,0x00203040,0xff102030),0xff102030);
  eq(pixel(4,0xff000000,0xff102030),0xff102030);
  p[7]=0;eq(pixel(4,0xff203040,0xff102030),0xff203040);
  p[7]=1;int[] a=new int[W*H];Native.fill(a,W,H,0,0,W,H,0xff000000);Native.microBegin(W,H);
  Native.microTriangle(a,W,H,triangle,p,null,0,0,0xff200000,4,0,0,W,H);
  Native.microTriangle(a,W,H,triangle,p,null,0,0,0xff000040,0,0,0,W,H);eq(a[16*W+16],0xff000040);
  float[] xyz={-1,-1,1,1,-1,1,0,1,1,-1,-1,2,1,-1,2,0,1,2};int[] faces={0,1,2,0,0,0,0,0,0,0xff200000,4,-1,0, 3,4,5,0,0,0,0,0,0,0xff000040,0,-1,0};
  Native.fill(a,W,H,0,0,W,H,0xff000000);Native.microBegin(W,H);Native.microMesh(a,W,H,xyz,p,faces,null,null,0,0,0,W,H);eq(a[16*W+16],0xff200040);
  System.out.println("MICRO_BLEND_PASS half add subtract saturation color_key neutral_black effect_disabled no_translucent_Z_write opaque_first");
 }
}
