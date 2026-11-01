import javax.microedition.lcdui.*;
import javax.microedition.lcdui.game.GameCanvas;
public class GameCanvasFlushCheck {
 static class Game extends GameCanvas {int paints;Game(){super(false);}public void paint(Graphics g){paints++;g.setColor(0);g.fillRect(0,0,320,240);}Graphics buffer(){return getGraphics();}}
 static class Other extends Canvas {int shows,hides;protected void showNotify(){shows++;}protected void hideNotify(){hides++;}protected void paint(Graphics g){g.setColor(0xffffff);g.fillRect(0,0,320,240);}}
 public static void main(String[]args)throws Exception{
  Display display=Display.getDisplay(null);Game game=new Game();display.setCurrent(game);Display.pump(0);int initial=game.paints;Graphics b=game.buffer();b.setColor(0x123456);b.fillRect(0,0,320,240);b.setColor(0x00ff00);b.fillRect(20,20,4,4);game.flushGraphics();Display.pump(0);if(game.paints!=initial)throw new Exception("flush called paint twice");int[] image=crimson.Native.snapshot();if(image[20*320+20]!=0xff00ff00)throw new Exception("flushed menu missing");
  b.setColor(0xff0000);b.fillRect(0,0,40,40);game.flushGraphics(0,0,1,1);image=crimson.Native.snapshot();if(image[0]!=0xffff0000||image[20*320+20]!=0xff00ff00)throw new Exception("partial flush");Other other=new Other();display.setCurrent(other);Display.pump(0);game.repaint();game.serviceRepaints();game.flushGraphics();image=crimson.Native.snapshot();if(image[0]!=0xffffffff||game.paints!=initial)throw new Exception("hidden GameCanvas published");display.setCurrent(other);if(other.shows!=1||other.hides!=0)throw new Exception("same current redisplayed");
  System.out.println("GAMECANVAS_FLUSH_PASS no_double_paint buffer_objects partial hidden_canvas same_current");
 }
}
