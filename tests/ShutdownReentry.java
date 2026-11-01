import javax.microedition.midlet.MIDlet;
public class ShutdownReentry {
 static volatile boolean returned;
 public static void main(String[]args)throws Exception {
  crimson.GameMain.midlet=new MIDlet(){protected void startApp(){}protected void pauseApp(){}protected void destroyApp(boolean force){Thread t=new Thread(new Runnable(){public void run(){crimson.GameMain.requestStop();returned=true;}});t.start();try{t.join(1000);}catch(Exception e){}if(!returned)throw new RuntimeException("shutdown callback blocked by launcher monitor");}};
  crimson.GameMain.requestStop();if(!returned)throw new Exception("reentry failed");System.out.println("SHUTDOWN_REENTRY_PASS");
 }
}
