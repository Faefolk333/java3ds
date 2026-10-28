package crimson;
public final class Native {
 public static native int[] snapshot();
 public static native int[] decode(byte[] bytes,int offset,int length);
 public static native void fill(int[] target,int width,int height,int x,int y,int w,int h,int color);
 public static native void blit(int[] target,int width,int height,int[] source,int sw,int sh,int x,int y,int cx,int cy,int cw,int ch);
 public static native void present();
 public static native int buttons();
 public static native void music(int id,int loops,int volume);
 public static native int audioLoad(byte[] data);
 public static native int audioControl(int handle,int operation,int value);
 public static native void stage(int stage,int detail);
 public static native int gcCycles();
 public static native void region(int[] target,int tw,int th,int[] source,int sw,int sh,int sx,int sy,int w,int h,int t,int x,int y,int cx,int cy,int cw,int ch);
}
