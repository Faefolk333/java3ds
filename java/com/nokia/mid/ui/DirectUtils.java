package com.nokia.mid.ui;
import javax.microedition.lcdui.*;
public final class DirectUtils {
 public static Image createImage(int w,int h,int argb){int[]p=new int[w*h];for(int i=0;i<p.length;i++)p[i]=argb;return Image.createRGBImage(p,w,h,true);}
 public static Image createImage(byte[]data,int offset,int length){return Image.createImage(data,offset,length);}
 public static DirectGraphics getDirectGraphics(Graphics g){if(g==null)throw new NullPointerException();return new Impl(g);}
 private static final class Impl implements DirectGraphics {
  private final Graphics g;private int alpha=255;Impl(Graphics g){this.g=g;}
  public void setARGBColor(int c){alpha=c>>>24;g.setColor(c);}public int getAlphaComponent(){return alpha;}public int getNativePixelFormat(){return TYPE_INT_8888_ARGB;}
  private int transform(int m){int rotation=m&4095;int t=rotation==0?0:rotation==90?6:rotation==180?3:rotation==270?5:-1;if(t<0||(m&~(4095|FLIP_HORIZONTAL|FLIP_VERTICAL))!=0)throw new IllegalArgumentException();if((m&FLIP_HORIZONTAL)!=0)t^=2;if((m&FLIP_VERTICAL)!=0)t^=1;return t;}
  public void drawImage(Image im,int x,int y,int anchor,int manipulation){g.drawRegion(im,0,0,im.getWidth(),im.getHeight(),transform(manipulation),x,y,anchor);}
  public void drawPixels(int[]p,boolean a,int o,int s,int x,int y,int w,int h,int m,int f){if(f!=888&&f!=8888)throw new IllegalArgumentException();if(w<=0||h<=0)return;int[]b=new int[w*h];for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++){int c=p[o+yy*s+xx];b[yy*w+xx]=a&&f==8888?c:c|0xff000000;}drawImage(Image.createRGBImage(b,w,h,true),x,y,20,m);}
  private int expand(int c,int f){switch(f){case 444:return 0xff000000|((c>>8&15)*17<<16)|((c>>4&15)*17<<8)|(c&15)*17;case 4444:return ((c>>12&15)*17<<24)|((c>>8&15)*17<<16)|((c>>4&15)*17<<8)|(c&15)*17;case 555:case 1555:return (f==555||(c&32768)!=0?0xff000000:0)|((c>>10&31)*255/31<<16)|((c>>5&31)*255/31<<8)|(c&31)*255/31;case 565:return 0xff000000|((c>>11&31)*255/31<<16)|((c>>5&63)*255/63<<8)|(c&31)*255/31;default:throw new IllegalArgumentException();}}
  private int pack(int c,int f){switch(f){case 444:case 4444:return (f==4444?(c>>>28)<<12:0)|((c>>20&15)<<8)|((c>>12&15)<<4)|(c>>4&15);case 555:case 1555:return (f==1555&&(c&0x80000000)!=0?32768:0)|((c>>19&31)<<10)|((c>>11&31)<<5)|(c>>3&31);case 565:return ((c>>19&31)<<11)|((c>>10&63)<<5)|(c>>3&31);default:throw new IllegalArgumentException();}}
  public void drawPixels(short[]p,boolean a,int o,int s,int x,int y,int w,int h,int m,int f){if(w<=0||h<=0)return;int[]b=new int[w*h];for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++){int c=expand(p[o+yy*s+xx]&65535,f);b[yy*w+xx]=a?c:c|0xff000000;}drawImage(Image.createRGBImage(b,w,h,true),x,y,20,m);}
  public void getPixels(int[]p,int o,int s,int x,int y,int w,int h,int f){if(f!=888&&f!=8888)throw new IllegalArgumentException();g.readPixels(p,o,s,x,y,w,h);if(f==888)for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++)p[o+yy*s+xx]&=0xffffff;}
  public void getPixels(short[]p,int o,int s,int x,int y,int w,int h,int f){int[]b=new int[w*h];g.readPixels(b,0,w,x,y,w,h);for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++)p[o+yy*s+xx]=(short)pack(b[yy*w+xx],f);}
 }
}
