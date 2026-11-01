import javax.microedition.lcdui.*;
import com.nokia.mid.ui.*;
public class DirectGraphicsCheck {
 static void check(boolean b){if(!b)throw new RuntimeException("DirectGraphics mismatch");}
 public static void main(String[]args){
  Image im=DirectUtils.createImage(8,8,0xff000000);DirectGraphics d=DirectUtils.getDirectGraphics(im.getGraphics());
  short[] p={(short)0xf800,(short)0x07e0,(short)0x001f,(short)0xffff};d.drawPixels(p,false,0,2,1,2,2,2,0,565);short[] q=new short[4];d.getPixels(q,0,2,1,2,2,2,565);for(int i=0;i<4;i++)check(p[i]==q[i]);
  int[] a={0xffff0000,0xff00ff00,0xff0000ff,0xffffffff};d.drawPixels(a,true,0,2,0,0,2,2,90,8888);int[] b=new int[4];d.getPixels(b,0,2,0,0,2,2,8888);check(b[0]==a[1]&&b[1]==a[3]&&b[2]==a[0]&&b[3]==a[2]);
  d.drawPixels(a,true,0,2,0,0,2,2,8192,8888);d.getPixels(b,0,2,0,0,2,2,8888);check(b[0]==a[1]&&b[1]==a[0]&&b[2]==a[3]&&b[3]==a[2]);
  Image fast=DirectUtils.createImage(3,3,0xff336699),reference=DirectUtils.createImage(3,3,0xff336699);int[] translucent={0x00000000,0x80ff8040,0xffffffff,0xff20cc60};
  DirectUtils.getDirectGraphics(fast.getGraphics()).drawPixels(translucent,true,2,-2,-1,0,2,2,0,8888);
  int[] reverse={translucent[2],translucent[3],translucent[0],translucent[1]};reference.getGraphics().drawRegion(Image.createRGBImage(reverse,2,2,true),0,0,2,2,0,-1,0,20);int[] expected=new int[9],actual=new int[9];reference.getRGB(expected,0,3,0,0,3,3);fast.getRGB(actual,0,3,0,0,3,3);for(int i=0;i<9;i++)for(int shift=0;shift<32;shift+=8)check(Math.abs((actual[i]>>>shift&255)-(expected[i]>>>shift&255))<=1);
  System.out.println("DIRECT_GRAPHICS_PASS RGB565 ARGB8888 ROT90 FLIP_H");
 }
}
