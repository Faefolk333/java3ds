import crimson.*;
class k {static int b=4,c=2;static int[] m=new int[8],p={4,8,Integer.MAX_VALUE,0};}
public class RaycasterCheck {
 public static void main(String[]args)throws Exception{
  GameMain.properties.put("launcher.raycaster","orcs2-columns-v1");
  float[] z=RaycasterDepth.find(k.m);if(z==null||z.length!=8||z[0]!=.25f||z[1]!=.125f||z[2]!=0||z[3]!=0||z[4]!=z[0])throw new Exception("column depth mismatch");
  if(RaycasterDepth.find(new int[8])!=null)throw new Exception("unrelated menu array acquired world depth");
  k.p[0]=16;if(RaycasterDepth.find(k.m)[0]!=.0625f)throw new Exception("stale distances");
  Native.stereoColumns(new float[1],100,100,new int[1]);
  System.out.println("RAYCASTER_CHECK_PASS reflection identity distances zero_sentinel refresh bounded_native");
 }
}
