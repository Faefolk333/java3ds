import javax.microedition.lcdui.*;
public class ArcCheck {
 static int[] picture(int start,int angle){Image im=Image.createImage(16,16);Graphics g=im.getGraphics();g.setColor(0);g.fillArc(0,0,16,16,start,angle);int[]p=new int[256];im.getRGB(p,0,16,0,0,16,16);return p;}
 public static void main(String[]args)throws Exception{
  int[] quarter=picture(0,90),negative=picture(90,-90);if(quarter[4*16+12]!=0xff000000||quarter[12*16+12]!=0xffffffff||quarter[4*16+4]!=0xffffffff)throw new Exception("arc quadrant");for(int i=0;i<256;i++)if(quarter[i]!=negative[i])throw new Exception("negative extent");int count=0;for(int p:picture(0,5))if(p==0xff000000)count++;if(count<1||count>12)throw new Exception("five degree golf arc pixels="+count);for(int p:picture(0,0))if(p!=0xffffffff)throw new Exception("zero arc");
  Image clipped=Image.createImage(16,16);Graphics g=clipped.getGraphics();g.setClip(8,0,8,8);g.setColor(0);g.fillArc(0,0,16,16,0,360);int[]p=new int[256];clipped.getRGB(p,0,16,0,0,16,16);if(p[4*16+4]!=0xffffffff||p[12*16+12]!=0xffffffff)throw new Exception("arc clip");System.out.println("ARC_PASS quarters negative_extent five_degree_golf zero clip");
 }
}
