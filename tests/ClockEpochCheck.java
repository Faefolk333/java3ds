public class ClockEpochCheck {
 public static void main(String[]args)throws Exception{
  // The host checks these seconds against its independent Windows clock. The
  // emulated 3DS clock starts in 1900; Java and C timed waits must agree on 1970.
  crimson.Native.stage(97,(int)(System.currentTimeMillis()/1000));
  Object lock=new Object();long before=System.currentTimeMillis();
  synchronized(lock){lock.wait(40);}
  long elapsed=System.currentTimeMillis()-before;
  if(elapsed<20||elapsed>1500)throw new Exception("Clock/timed-wait disagreement: "+elapsed);
  System.out.println("CLOCK_WAIT_PASS Unix_epoch bounded_absolute_condition_deadline");
 }
}
