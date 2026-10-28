public class StopWait {
 public static void main(String[] args) throws Exception {
  Object monitor=new Object();synchronized(monitor){monitor.wait();}
 }
}
