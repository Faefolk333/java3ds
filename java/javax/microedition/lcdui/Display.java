package javax.microedition.lcdui;
public final class Display {
 private static final Display D=new Display();public static volatile Displayable current;private static final java.util.Vector callbacks=new java.util.Vector();
 public static Display getDisplay(javax.microedition.midlet.MIDlet midlet){return D;}
 public void setCurrent(Displayable d){Displayable old=current;if(old instanceof Canvas)((Canvas)old).hideNotify();current=d;if(d instanceof Canvas){((Canvas)d).showNotify();((Canvas)d).repaint();}}
 public Displayable getCurrent(){return current;}public void callSerially(Runnable r){synchronized(callbacks){callbacks.addElement(r);}}
 public static void pump(int held){synchronized(callbacks){while(!callbacks.isEmpty()){Runnable r=(Runnable)callbacks.elementAt(0);callbacks.removeElementAt(0);r.run();}}Displayable d=current;if(d instanceof Canvas){if(!crimson.GameMain.popProfile)((Canvas)d).pump(held);}else if(d!=null){int keys=crimson.Controls.keys(held,false);if((keys&16)!=0)d.command(-6);if((keys&32)!=0)d.command(-7);d.render(new Graphics(null,crimson.GameMain.width,crimson.GameMain.height,0));crimson.Native.present();}}
 public boolean vibrate(int ms){return false;}public boolean flashBacklight(int ms){return false;}public boolean isColor(){return true;}public int numColors(){return 16777216;}public int numAlphaLevels(){return 256;}
}
