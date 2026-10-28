import javax.microedition.midlet.*;
public class DaemonExit extends MIDlet {
 protected void startApp(){Thread t=new Thread(new Runnable(){public void run(){while(true){byte[]b=new byte[8192];try{Thread.sleep(2);}catch(Exception e){}}}},"Exit regression daemon");t.setDaemon(true);t.start();try{Thread.sleep(150);}catch(Exception e){}System.out.println("DAEMON_EXIT_REQUEST");notifyDestroyed();}
 protected void pauseApp(){}protected void destroyApp(boolean b){}
}
