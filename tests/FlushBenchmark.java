import javax.microedition.lcdui.*;
import crimson.Native;
public class FlushBenchmark {
 public static void main(String[] args)throws Exception{
  Image b=Image.createImage(320,240);Graphics g=b.getGraphics();g.setColor(0x173b28);g.fillRect(0,0,320,240);Image sprite=Image.createRGBImage(new int[]{0xffff0080,0xff102030,0xff00a0b0,0xffcccccc},2,2,true);g.drawImage(sprite,180,130,20);
  long start=System.currentTimeMillis();for(int i=0;i<1000;i++)Graphics.flushImage(b,0,0,320,240);long elapsed=System.currentTimeMillis()-start;
  int[] screen=Native.snapshot();if(screen[130*320+180]!=0xffff0080||screen[50]!=0xff173b28)throw new Exception("flush pixels changed");
  System.out.println("FLUSH_BENCH_PASS frames=1000 ms="+elapsed);
 }
}
