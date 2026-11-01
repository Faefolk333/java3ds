package javax.microedition.lcdui;
import javax.microedition.lcdui.game.*;
import javax.microedition.m3g.*;
import crimson.Native;
public class StereoCompatibility {
 static class Rect extends Layer {int color;Rect(int x,int y,int w,int h,int c){super(w,h);setPosition(x,y);color=c;}public void paint(Graphics g){g.setColor(color);g.fillRect(x,y,width,height);}}
 public static void main(String[]args)throws Exception{
  Graphics canvas=new Graphics(null,320,240,0);canvas.setColor(0);canvas.fillRect(0,0,320,240);
  LayerManager layers=new LayerManager();layers.append(new Rect(90,70,120,100,0x70ffd0));layers.append(new Rect(0,0,320,240,0x304080));layers.paint(canvas,0,0);Native.present();
  canvas.setColor(0);canvas.fillRect(0,0,320,240);canvas.microBegin();canvas.microTriangle(new float[]{-1,-1,2,0,0,1,-1,3,0,0,0,1,4,0,0},new float[]{160,120,1,100,100,100,1},null,0,0,0xff55ffaa,0);Native.present();
  Graphics3D g=Graphics3D.getInstance();Camera camera=new Camera();camera.setPerspective(60,320f/240,1,100);VertexArray positions=new VertexArray(3,3,2);positions.set(0,3,new short[]{-1,-1,-2,1,-1,-3,0,1,-4});VertexBuffer vb=new VertexBuffer();vb.setPositions(positions,1,null);vb.setDefaultColor(0xffff8855);TriangleStripArray ix=new TriangleStripArray(new int[]{0,1,2},new int[]{3});Appearance a=new Appearance();PolygonMode pm=new PolygonMode();pm.setCulling(PolygonMode.CULL_NONE);a.setPolygonMode(pm);g.bindTarget(canvas);g.setCamera(camera,null);Background bg=new Background();bg.setColor(0xff102030);g.clear(bg);g.render(vb,ix,a,null);g.releaseTarget();Native.present();
  Image offscreen=Image.createImage(320,240);Graphics og=offscreen.getGraphics();og.microBegin();og.microMesh(new float[]{-1,-1,2,1,-1,3,0,1,4},new float[]{160,120,1,100,100,100,1},new int[]{0,1,2,0,0,0,0,0,0,0xff55ffaa,0,-1,0},null,null,0);canvas.drawImage(offscreen,0,0,0);Native.present();
  Image clone=Image.createImage(offscreen);Image flip=Image.createImage(clone,0,0,320,240,2);canvas.drawImage(flip,0,0,0);Native.present();
  int[] readback=new int[320*240];offscreen.getRGB(readback,0,320,0,0,320,240);canvas.drawRGB(readback,0,320,0,0,320,240,false);Native.present();
  com.nokia.mid.ui.DirectGraphics dg=com.nokia.mid.ui.DirectUtils.getDirectGraphics(canvas);int[] nokia=new int[320*240];dg.getPixels(nokia,0,320,0,0,320,240,8888);dg.drawPixels(nokia,true,0,320,0,0,320,240,0,8888);Native.present();
  short[] packed=new short[320*240];dg.getPixels(packed,0,320,0,0,320,240,565);dg.drawPixels(packed,false,0,320,0,0,320,240,0,565);Native.present();
  Graphics.flushImage(offscreen,0,0,320,240);
  System.out.println("STEREO_INTEGRATION_PASS LayerManager Micro3D M3G offscreen_image clone transform fast_flush");
 }
}
