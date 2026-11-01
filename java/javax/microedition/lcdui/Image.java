package javax.microedition.lcdui;
import java.io.*;
public final class Image {
 final int[] pixels;final int w,h;public static int loaded;
 private Image(int w,int h,int[] data){if(w<1||h<1||w>16384||h>16384||(long)w*h>1048576||data.length!=w*h)throw new IllegalArgumentException();this.w=w;this.h=h;pixels=data;loaded++;}
 public static Image createImage(byte[] b,int offset,int n){
  if(b==null||offset<0||n<0||offset>b.length-n)throw new IllegalArgumentException();
  crimson.Native.stage(10,n);
  int[] size=new int[2];int[] decoded=crimson.Native.decode(b,offset,n,size);if(decoded==null)throw new IllegalArgumentException("Image decode failed");
  crimson.Native.stage(11,loaded+1);
  return new Image(size[0],size[1],decoded);
 }
 public static Image createImage(InputStream in)throws IOException{if(in==null)throw new NullPointerException();ByteArrayOutputStream out=new ByteArrayOutputStream();byte[] buf=new byte[1024];int n;while((n=in.read(buf))!=-1)out.write(buf,0,n);byte[] bytes=out.toByteArray();return createImage(bytes,0,bytes.length);}
 public static Image createImage(String name)throws IOException{
  InputStream in=crimson.Resources.open(Image.class,name);if(in==null)throw new IOException(name);
  ByteArrayOutputStream out=new ByteArrayOutputStream();try{byte[] buf=new byte[1024];int n;while((n=in.read(buf))!=-1)out.write(buf,0,n);}finally{in.close();}
  byte[] b=out.toByteArray();return createImage(b,0,b.length);
 }
 public static Image createImage(int w,int h){if(w<1||h<1||w>16384||h>16384||(long)w*h>1048576)throw new IllegalArgumentException();int[] p=new int[w*h];for(int i=0;i<p.length;i++)p[i]=0xffffffff;return new Image(w,h,p);}
 public static Image createRGBImage(int[] rgb,int w,int h,boolean alpha){if(w<1||h<1||w>16384||h>16384||(long)w*h>1048576||rgb.length<w*h)throw new IllegalArgumentException();int[] p=new int[w*h];for(int i=0;i<p.length;i++)p[i]=alpha?rgb[i]:rgb[i]|0xff000000;return new Image(w,h,p);}
 public int getWidth(){return w;}public int getHeight(){return h;}
 public Graphics getGraphics(){return new Graphics(pixels,w,h,0);}
 public static Image createImage(Image src){int[]p=new int[src.pixels.length];System.arraycopy(src.pixels,0,p,0,p.length);return new Image(src.w,src.h,p);}
 public static Image createImage(Image src,int x,int y,int w,int h,int transform){if(transform<0||transform>7||x<0||y<0||w<1||h<1||x+w>src.w||y+h>src.h)throw new IllegalArgumentException();int dw=transform>=4?h:w,dh=transform>=4?w:h;int[]data=new int[dw*dh];for(int yy=0;yy<dh;yy++)for(int xx=0;xx<dw;xx++){int u=xx,v=yy;switch(transform){case 1:v=h-1-yy;break;case 2:u=w-1-xx;break;case 3:u=w-1-xx;v=h-1-yy;break;case 4:u=yy;v=xx;break;case 5:u=yy;v=h-1-xx;break;case 6:u=w-1-yy;v=xx;break;case 7:u=w-1-yy;v=h-1-xx;break;}data[yy*dw+xx]=src.pixels[(y+v)*src.w+x+u];}return new Image(dw,dh,data);}
 public void getRGB(int[] dst,int offset,int scan,int x,int y,int width,int height){if(x<0||y<0||width<0||height<0||x+width>w||y+height>h)throw new IllegalArgumentException();for(int row=0;row<height;row++)System.arraycopy(pixels,(y+row)*w+x,dst,offset+row*scan,width);}
}
