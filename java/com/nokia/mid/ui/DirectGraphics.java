package com.nokia.mid.ui;
import javax.microedition.lcdui.Image;
public interface DirectGraphics {
 int FLIP_HORIZONTAL=8192,FLIP_VERTICAL=16384,ROTATE_90=90,ROTATE_180=180,ROTATE_270=270;
 int TYPE_USHORT_444_RGB=444,TYPE_USHORT_555_RGB=555,TYPE_USHORT_565_RGB=565,TYPE_USHORT_1555_ARGB=1555,TYPE_USHORT_4444_ARGB=4444,TYPE_INT_888_RGB=888,TYPE_INT_8888_ARGB=8888;
 void setARGBColor(int color);int getAlphaComponent();int getNativePixelFormat();
 void drawImage(Image image,int x,int y,int anchor,int manipulation);
 void drawPixels(short[]pixels,boolean transparency,int offset,int scan,int x,int y,int w,int h,int manipulation,int format);
 void drawPixels(int[]pixels,boolean transparency,int offset,int scan,int x,int y,int w,int h,int manipulation,int format);
 void getPixels(int[]pixels,int offset,int scan,int x,int y,int w,int h,int format);
 void getPixels(short[]pixels,int offset,int scan,int x,int y,int w,int h,int format);
}
