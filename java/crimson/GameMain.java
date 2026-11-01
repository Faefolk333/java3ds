package crimson;
import java.io.*;import java.util.*;import javax.microedition.midlet.MIDlet;import javax.microedition.lcdui.Display;
public final class GameMain {
 public static boolean original,popProfile;public static int width=320,height=240,keyProfile;public static volatile boolean stopped;public static MIDlet midlet;
 public static final Hashtable properties=new Hashtable();private static volatile int state=-1;private static int soundScene=-1;
 public static int state(){return state;}public static boolean gameplay(){return !popProfile||state==10;}
 public static boolean menuBackdrop(){if(!popProfile)return false;switch(state){case 1:case 2:case 4:case 5:case 7:case 8:case 9:case 16:case 18:case 19:case 21:case 22:case 23:return true;default:return false;}}
 public static void updateState(int s){state=s;Native.stage(60,s);}
 public static void updateSound(int s,int level,boolean enabled){int v=(s<<8)|(level&255)|(enabled?65536:0);if(v!=soundScene){soundScene=v;Native.stage(82,v);}}
 private static boolean stopping;public static void requestStop(){synchronized(GameMain.class){if(stopped||stopping)return;stopping=true;}try{if(midlet!=null)midlet.shutdown();}catch(Throwable e){e.printStackTrace();}stopped=true;Native.stage(80,0);}
 public static String property(String key,String fallback){String value=(String)properties.get(key);return value==null?fallback:value;}
 public static void main(String[]args)throws Exception{
  String file=System.getProperty("launcher.config");if(file==null)throw new IOException("Missing launch configuration");
  BufferedReader r=new BufferedReader(new InputStreamReader(new FileInputStream(file),"UTF-8"));try{String line;while((line=r.readLine())!=null){int i=line.indexOf('=');if(i>0)properties.put(line.substring(0,i),line.substring(i+1));}}finally{r.close();}
  width=Integer.parseInt(property("launcher.width","320"));height=Integer.parseInt(property("launcher.height","240"));keyProfile=Integer.parseInt(property("launcher.keys","0"));popProfile=property("launcher.profile","").equals("pop-classic");original=width==320;
  if(width<64||height<64||width>480||height>640||width*height>512*512)throw new IOException("Invalid viewport");
  System.setProperty("pop.saves",property("launcher.saves","/3ds/java-3ds/saves/default"));
  System.setProperty("microedition.platform","Nokia E71");System.setProperty("microedition.configuration","CLDC-1.1");System.setProperty("microedition.profiles","MIDP-2.0");System.setProperty("microedition.locale","en-US");System.setProperty("microedition.encoding","UTF-8");
  System.out.println("Java3DS: "+property("MIDlet-Name","MIDlet")+" "+width+"x"+height);
  String map=property("launcher.bindings","");java.util.StringTokenizer mt=new java.util.StringTokenizer(map,",");for(int i=0;i<14&&mt.hasMoreTokens();i++)Controls.bindings[i]=Integer.parseInt(mt.nextToken());
  midlet=(MIDlet)Class.forName(property("launcher.main","")).newInstance();
  Thread start=new Thread(new Runnable(){public void run(){try{midlet.launch();}catch(Throwable e){e.printStackTrace();Native.stage(90,1);requestStop();}}},"MIDlet start");start.start();
  while(!stopped){int buttons=Native.buttons();if((buttons&0x40000000)!=0){requestStop();break;}Display.pump(buttons);Thread.sleep(10);}
 }
}
