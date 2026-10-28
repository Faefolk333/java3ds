import javax.microedition.midlet.*;
import javax.microedition.lcdui.*;
import javax.microedition.rms.*;
import javax.microedition.media.*;
import java.io.*;
public final class LauncherCheck extends MIDlet implements Runnable,CommandListener {
 private final Command quit=new Command("Exit",Command.EXIT,1);
 private final Command music=new Command("Music",Command.OK,0);
 private Screen view;private boolean run;private int visits,key,frame;private Player player;
 protected void startApp()throws MIDletStateChangeException{
  try{
   InputStream in=String.class.getResourceAsStream("/check.txt");
   if(in==null||in.read()!='R')throw new IOException("Resource lookup failed");in.close();
   RecordStore r=RecordStore.openRecordStore("visits",true);
   if(r.getNumRecords()>0){byte[] b=r.getRecord(1);visits=b[0]&255;}visits++;
   byte[] b={(byte)visits};if(r.getNumRecords()==0)r.addRecord(b,0,1);else r.setRecord(1,b,0,1);r.closeRecordStore();
   System.out.println("CHECK_RESOURCE_OK RMS_VISIT "+visits);
  }catch(Exception e){e.printStackTrace();notifyDestroyed();return;}
  view=new Screen();view.addCommand(music);view.addCommand(quit);view.setCommandListener(this);
  Display.getDisplay(this).setCurrent(view);run=true;new Thread(this).start();
 }
 protected void pauseApp(){}protected void destroyApp(boolean forced){run=false;if(player!=null)player.close();}
 public void commandAction(Command c,Displayable d){if(c==quit){destroyApp(true);notifyDestroyed();}else play();}
 private void play(){try{if(player!=null)player.close();
  byte[] midi={77,84,104,100,0,0,0,6,0,0,0,1,0,96,77,84,114,107,0,0,0,37,0,(byte)192,0,0,(byte)144,60,100,96,(byte)128,60,0,0,(byte)144,64,100,96,(byte)128,64,0,0,(byte)144,67,100,96,(byte)128,67,0,0,(byte)144,72,100,96,(byte)128,72,0,0,(byte)255,47,0};
  // Track length is calculated from the generated payload.
  midi[21]=(byte)(midi.length-22);
  player=Manager.createPlayer(new ByteArrayInputStream(midi),"audio/midi");player.setLoopCount(1);player.start();System.out.println("CHECK_MIDI_START");
 }catch(Exception e){e.printStackTrace();}}
 public void run(){play();while(run){frame++;view.repaint();try{Thread.sleep(40);}catch(Exception e){}if("yes".equals(getAppProperty("Check-Autorun"))&&frame>=60){System.out.println("CHECK_AUTORUN_PASS");destroyApp(true);notifyDestroyed();}}}
 private final class Screen extends Canvas{
  protected void keyPressed(int k){key=k;if(k==KEY_NUM5||k==-5)play();}
  protected void paint(Graphics g){int w=getWidth(),h=getHeight();g.setColor(0x112334);g.fillRect(0,0,w,h);g.setColor(0x55d8bc);g.drawRect(0,0,w-1,h-1);g.drawString("JAVA POCKET CHECK",w/2,12,Graphics.HCENTER|Graphics.TOP);g.setColor(0xffffff);g.drawString(w+" x "+h,w/2,36,Graphics.HCENTER|Graphics.TOP);g.drawString("MIDI + RMS + resources",w/2,60,Graphics.HCENTER|Graphics.TOP);g.drawString("Visits: "+visits+"  Key: "+key,w/2,84,Graphics.HCENTER|Graphics.TOP);g.drawString("A / touch 5: melody",w/2,h-48,Graphics.HCENTER|Graphics.TOP);g.drawString("START: exit",w/2,h-24,Graphics.HCENTER|Graphics.TOP);g.setColor(0xffcb66);g.fillRect(10+frame%Math.max(1,w-36),h/2,16,16);}
 }
}
