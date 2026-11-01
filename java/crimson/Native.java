package crimson;
public final class Native {
 public static native int[] snapshot();
 public static native void readPixels(int[] target,int tw,int th,int[] dst,int offset,int scan,int x,int y,int w,int h);
 public static native int[] decode(byte[] bytes,int offset,int length,int[] dimensions);
 public static native void fill(int[] target,int width,int height,int x,int y,int w,int h,int color);
 public static native void blit(int[] target,int width,int height,int[] source,int sw,int sh,int x,int y,int cx,int cy,int cw,int ch);
 public static native void rgb(int[] target,int tw,int th,int[] pixels,int offset,int scan,int x,int y,int w,int h,boolean alpha,int cx,int cy,int cw,int ch);
 public static native void text(int[] target,int tw,int th,long[] glyphs,String text,int x,int y,int scale,int color,int cx,int cy,int cw,int ch);
 public static native boolean microPose(float[] source,float[] destination,byte[] bones,float[] action);
 public static native void microTransform(float[] source,float[] destination,float[] matrix);
 public static native void m3gSpan(int[] raster,short[] depth,int offset,int count,int x,float[] values,int[] state,int[] texture);
 public static native void microBegin();
 public static native void microTriangle(int[] target,int w,int h,float[] triangle,float[] projection,int[] texture,int tw,int th,int color,int flags,int cx,int cy,int cw,int ch);
 public static native void present();
 public static native int buttons();
 public static native boolean motion(int kind,int[] sample);
 public static native void music(int id,int loops,int volume);
 public static native int audioLoad(byte[] data);
 public static native int audioControl(int handle,int operation,int value);
 public static native void stage(int stage,int detail);
 public static native int gcCycles();
 public static native void region(int[] target,int tw,int th,int[] source,int sw,int sh,int sx,int sy,int w,int h,int t,int x,int y,int cx,int cy,int cw,int ch);
}
