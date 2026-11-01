package crimson;
// Bounded readback metadata. Java owns these arrays, so no native pointer can
// outlive a collected Image. Two slots cover the usual double-buffered renderer.
public final class RasterDepth {
 private static final Object[] keys=new Object[2];
 private static final float[][] values=new float[2][];
 private static int next;
 public static float[] find(Object pixels){for(int i=0;i<2;i++)if(keys[i]==pixels)return values[i];return null;}
 public static float[] prepare(Object pixels){float[] result=find(pixels);if(Native.stereoMode()==0)return null;int size=pixels instanceof int[]?((int[])pixels).length:((short[])pixels).length;if(result==null&&size<=512*512){try{result=new float[size];int slot=next++%2;keys[slot]=pixels;values[slot]=result;}catch(OutOfMemoryError e){return null;}}return result;}
 public static void copy(Object source,int offset,int scan,Object destination,int destOffset,int destScan,int w,int h){float[] src=find(source);if(src==null)return;float[] dst=prepare(destination);if(dst!=null)for(int row=0;row<h;row++)System.arraycopy(src,offset+row*scan,dst,destOffset+row*destScan,w);}
 public static void capture(int[] pixels,int offset,int scan,int x,int y,int w,int h,float[] source,int sw){
  float[] result=find(pixels);if(Native.stereoMode()==0)return;
  if(result==null&&source!=null&&pixels.length<=512*512){try{result=new float[pixels.length];int slot=next++%2;keys[slot]=pixels;values[slot]=result;}catch(OutOfMemoryError e){return;}}
  if(result!=null)for(int row=0;row<h;row++){int at=offset+row*scan;if(source!=null)System.arraycopy(source,(y+row)*sw+x,result,at,w);else for(int col=0;col<w;col++)result[at+col]=-4;}
 }
}
