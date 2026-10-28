public class StopLoops {
 static final Object monitor=new Object();
 public static void main(String[] args) throws Exception {
  new Thread(new Runnable(){public void run(){synchronized(monitor){for(;;){}}}}).start();
  Thread.sleep(150);
  synchronized(monitor){System.out.println("UNREACHABLE");}
 }
}
