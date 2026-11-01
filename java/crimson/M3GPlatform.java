package crimson;
public final class M3GPlatform {
 public static boolean nativeRaster=true;public static long nativeSpans,nativePixels,fallbackPixels;
 public static final int LOG_DEBUG=0,LOG_WARNING=1,LOG_ERROR=2;
 public static boolean halfResM3GRaster=false,m3gDisableFog=false,M3GRenderUntexturedPolygons=false,M3GRenderWireframe=false;
 public static int m3gAntiAliasingMode=0,m3gBilinearFilterMode=0,m3gDitheringMode=0,m3gMipmapMode=0,m3gPerspectiveCorrectionMode=1,m3gPerspCorrSubFactor=7;
 public static void log(int level,String message){if(level>=LOG_WARNING)System.err.println("M3G: "+message);}
 public static java.io.InputStream getMIDletResourceAsStream(String name){return Resources.open(M3GPlatform.class,name.startsWith("/")?name:"/"+name);}
}
