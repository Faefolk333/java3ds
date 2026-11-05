package crimson;
import java.lang.reflect.Field;
/** Opt-in adapter for a verified JAR's software-rendered world buffer.
 * Wall-column distances do not expose per-object/floor depth: this remains an
 * approximate stereo view. Menus and the separately drawn HUD remain flat. */
public final class RaycasterDepth {
 private static int profile=-1;private static Field pixels,columns,width,height;private static boolean tried,reported;
 private static Field field(Class type,String name,Class value)throws Exception{Field[] fields=type.getDeclaredFields();for(int i=0;i<fields.length;i++)if(fields[i].getName().equals(name)&&fields[i].getType()==value){fields[i].setAccessible(true);return fields[i];}throw new NoSuchFieldException(name);}
 public static float[] find(int[] source){
  if(profile<0){String value=GameMain.property("launcher.raycaster","");profile="orcs2-columns-v1".equals(value)?1:"doom2-columns-v1".equals(value)?2:0;}if(profile==0||(Native.stereoMode()&1)==0)return null;
  try{
   if(!tried){tried=true;Class renderer=Class.forName(profile==1?"k":"j");pixels=field(renderer,profile==1?"m":"c",int[].class);columns=field(renderer,profile==1?"p":"d",int[].class);width=field(renderer,profile==1?"b":"i",Integer.TYPE);height=field(renderer,profile==1?"c":"j",Integer.TYPE);}
   if(pixels==null||pixels.get(null)!=source)return null;
   int w=width.getInt(null),h=height.getInt(null);int[] distances=(int[])columns.get(null);if(w<1||h<1||w>512||h>512||w*h>source.length||distances==null||distances.length<w)return null;
   float[] depth=RasterDepth.prepare(source);if(depth!=null){Native.stereoColumns(depth,w,h,distances);if(!reported){reported=true;System.out.println("RAYCASTER_DEPTH world="+w+"x"+h+" wall_columns="+distances.length);}}return depth;
  }catch(Exception failure){pixels=null;if(!reported){reported=true;System.out.println("Raycaster depth unavailable: "+failure.toString());}return null;}
 }
}
