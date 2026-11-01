import javax.microedition.lcdui.*;
public class GraphicsHotPath {
 public static void main(String[]args){Image image=Image.createImage(64,64);Graphics g=image.getGraphics();int[] rgb=new int[4096];for(int i=0;i<rgb.length;i++)rgb[i]=0xff000000|i*1729;long start=System.currentTimeMillis();for(int i=0;i<400;i++){g.drawRGB(rgb,0,64,0,0,64,64,false);g.setColor(0x22bbff);g.drawString("JAVA3DS",0,0,20);}long elapsed=System.currentTimeMillis()-start;image.getRGB(rgb,0,64,0,0,64,64);int hash=1;for(int c:rgb)hash=hash*31+c;System.out.println("GRAPHICS_BENCH ms="+elapsed+" hash="+hash);}
}
