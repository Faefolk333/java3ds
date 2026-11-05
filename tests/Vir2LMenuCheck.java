import crimson.*;import javax.microedition.midlet.MIDlet;import javax.microedition.lcdui.Canvas;
public class Vir2LMenuCheck extends MIDlet {
 private static final javax.microedition.lcdui.Command selectCommand=new javax.microedition.lcdui.Command("Select",3,0),okCommand=new javax.microedition.lcdui.Command("OK",3,0);
 static class Menu {private final java.util.Vector u=new java.util.Vector();Menu(){u.addElement(selectCommand);}}
 private static Object uic=new Menu();protected void startApp(){}protected void pauseApp(){}protected void destroyApp(boolean b){}
 static class ObfuscatedMenu {private static final javax.microedition.lcdui.Command u=new javax.microedition.lcdui.Command("Select",3,0),I=new javax.microedition.lcdui.Command("OK",3,0);private final java.util.Vector q=new java.util.Vector();ObfuscatedMenu(){q.addElement(u);}}
 static class ObfuscatedGame extends MIDlet {private static ObfuscatedMenu ax=new ObfuscatedMenu();protected void startApp(){}protected void pauseApp(){}protected void destroyApp(boolean b){}}
 static int bit(int code){for(int i=0;i<Controls.CODES.length;i++)if(Controls.CODES[i]==code)return 1<<i;return 0;}
 static void check(boolean value)throws Exception{if(!value)throw new Exception("Vir2L menu controls");}
 public static void main(String[]args)throws Exception{
  GameMain.midlet=new Vir2LMenuCheck();MenuCompatibility.enabled=true;Controls.numeric=true;
  check(Controls.keys(16,false)==bit(-7));check(Controls.keys(32,false)==bit(-6));check(Controls.keys(1<<20,false)==bit(53));
  Controls.bindings[4]=49;check(Controls.keys(16,false)==bit(49));Controls.bindings[4]=0;
  ((Menu)uic).u.removeAllElements();((Menu)uic).u.addElement(new javax.microedition.lcdui.Command("Exit",7,0));check(Controls.keys(16,false)==bit(-5));
  uic=null;check(Controls.keys(16,true)==bit(-5));check(Controls.phoneCode(-5)==53);check(Controls.phoneCode(-6)==-6);
  Canvas canvas=new Canvas(){protected void paint(javax.microedition.lcdui.Graphics g){}};check(canvas.getKeyCode(Canvas.FIRE)==53&&canvas.getKeyCode(Canvas.UP)==50);
  GameMain.midlet=new ObfuscatedGame();check(Controls.keys(16,false)==bit(-7));check(Controls.keys(32,false)==bit(-6));ObfuscatedGame.ax.q.removeAllElements();check(Controls.keys(16,false)==bit(-5));
  System.out.println("VIR2L_MENU_PASS A_confirm B_back gameplay_5 numeric_getKeyCode explicit_bindings touch_unchanged");
 }
}
