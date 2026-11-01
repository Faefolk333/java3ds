package javax.microedition.lcdui;
public class RenderCompatibility {
 public static void main(String[]args)throws Exception {
  Image atlas=Image.createImage("/wide-atlas.png");if(atlas.getWidth()!=4096)throw new Exception("wide sprite decode");Image crop=Image.createImage(4,2);crop.getGraphics().drawRegion(atlas,4092,0,4,2,0,0,0,20);int[] rgba=new int[8];crop.getRGB(rgba,0,4,0,0,4,2);if(rgba[7]!=0xff12ab34)throw new Exception("wide sprite region");
  Image image=Image.createImage(32,32);Graphics g=image.getGraphics();int[] pixels={0xff000001,0xff000002,0xff000003,0xff000004,0xff000005,0xff000006};g.drawRGB(pixels,4,-2,1,1,2,3,true);int[] result=new int[6];image.getRGB(result,0,2,1,1,2,3);if(result[0]!=pixels[4]||result[5]!=pixels[1])throw new Exception("negative stride");g.setClip(4,4,8,8);g.setColor(0x123456);g.drawString("ABC",1,1,20);int[] actual=new int[1024];image.getRGB(actual,0,32,0,0,32,32);for(int y=4;y<9;y++)for(int x=4;x<12;x++){int letter=(x-1)/8,bit=((y-1)*8+(x-1)%8);boolean set=(BitmapFont.DATA['A'+letter]&(1L<<bit))!=0;if(set&&actual[y*32+x]!=0xff123456)throw new Exception("font");}
  g.setClip(0,0,32,32);float[] projection={16,16,1,100,16,16,1};float[] far={-5,-5,10,0,0,5,-5,10,0,0,0,5,10,0,0};float[] near={-1,-1,2,0,0,1,-1,2,0,0,0,1,2,0,0};g.microBegin();g.microTriangle(near,projection,null,0,0,0xff0000ff,0);g.microTriangle(far,projection,null,0,0,0xffff0000,0);int[] one=new int[1];image.getRGB(one,0,1,16,16,1,1);if(one[0]!=0xff0000ff)throw new Exception("3D depth: "+Integer.toHexString(one[0]));
  boolean caught=false;try{int[] impossible=new int[1073741824];}catch(OutOfMemoryError e){caught=true;}if(!caught)throw new Exception("allocation guard");
  final int[] calls={0};Display d=Display.getDisplay(null);d.callSerially(new Runnable(){public void run(){calls[0]++;d.callSerially(this);}});Display.pump(0);if(calls[0]!=1)throw new Exception("serial callback loop");Display.pump(0);if(calls[0]!=2)throw new Exception("serial callback deferred");
  System.out.println("RENDER_PASS RGB negative_stride native_font depth OOM callback_deferral");
 }
}
