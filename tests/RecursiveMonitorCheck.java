public class RecursiveMonitorCheck {
 static final Object lock=new Object();static volatile boolean notified;
 public static void main(String[]args)throws Exception{
  Thread notifier;
  synchronized(lock){synchronized(lock){synchronized(lock){
   notifier=new Thread(new Runnable(){public void run(){synchronized(lock){notified=true;lock.notifyAll();}}});notifier.start();
   long deadline=System.currentTimeMillis()+2000;while(!notified&&System.currentTimeMillis()<deadline)lock.wait(500);
   if(!notified)throw new Exception("Object.wait retained recursive monitor ownership");
  }}}
  notifier.join(1000);if(notifier.isAlive())throw new Exception("notify thread did not exit");
  synchronized(lock){synchronized(lock){long start=System.currentTimeMillis();lock.wait(30);if(System.currentTimeMillis()-start<15)throw new Exception("recursive timed wait returned immediately");}}
  System.out.println("RECURSIVE_MONITOR_PASS release_all restore_depth notify timed_wait");
 }
}
