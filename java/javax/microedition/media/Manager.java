package javax.microedition.media;
import java.io.*;
import crimson.Native;
import javax.microedition.media.control.VolumeControl;
public final class Manager {
 public static String[] getSupportedContentTypes(String protocol){return new String[]{"audio/midi","audio/x-midi","audio/wav","audio/x-wav"};}public static String[] getSupportedProtocols(String type){return new String[]{"resource"};}
 public static Player createPlayer(String url)throws IOException,MediaException{if(url==null)throw new NullPointerException();if(!url.startsWith("resource:"))throw new MediaException("Unsupported media protocol");InputStream in=crimson.Resources.open(Manager.class,url.substring(9));try{return createPlayer(in,null);}finally{if(in!=null)in.close();}}
 public static Player createPlayer(InputStream in,String mime)throws IOException,MediaException{
  if(in==null)throw new IOException("Missing media");
  ByteArrayOutputStream out=new ByteArrayOutputStream();byte[] buffer=new byte[1024];int n;
  while((n=in.read(buffer))!=-1){if(n==0)continue;if(out.size()+n>1024*1024)throw new IOException("Media too large");out.write(buffer,0,n);}
  return new NativePlayer(out.toByteArray());
 }
 private static final class NativePlayer implements Player,VolumeControl {
  private byte[] data;private int handle=-1,state=100,loops=1,volume=100;private long position;
  NativePlayer(byte[] bytes){data=bytes;}
  public synchronized Control getControl(String name){return name!=null&&name.endsWith("VolumeControl")?this:null;}
  public synchronized void setLoopCount(int n){if(n==0||n < -1)throw new IllegalArgumentException("loop count");loops=n;}
  public synchronized void realize()throws MediaException{
   if(state==0)throw new IllegalStateException("closed");if(state>=200)return;
   handle=Native.audioLoad(data);if(handle<0)throw new MediaException("MIDI/WAV output unavailable");data=null;state=200;
   Native.audioControl(handle,5,volume);
  }
  public synchronized void prefetch()throws MediaException{realize();if(state<300)state=300;}
  public synchronized void start()throws MediaException{
   prefetch();if(getState()==400)return;
   Native.audioControl(handle,4,(int)(position/1000));
   if(Native.audioControl(handle,1,loops)<0)throw new MediaException("Cannot start media");state=400;
  }
  public synchronized void stop(){if(handle>=0){position=Native.audioControl(handle,7,0)*1000L;Native.audioControl(handle,2,0);}if(state>=300)state=300;}
  public synchronized void deallocate(){stop();if(state>=200)state=200;}
  public synchronized void close(){if(handle>=0)Native.audioControl(handle,3,0);handle=-1;data=null;state=0;}
  public synchronized int getState(){if(state==400&&Native.audioControl(handle,6,0)!=400){state=300;position=0;}return state;}
  public synchronized long setMediaTime(long microseconds){position=Math.max(0,Math.min(2147483647000L,microseconds));if(handle>=0)position=Native.audioControl(handle,4,(int)(position/1000))*1000L;return position;}
  public synchronized int setLevel(int level){volume=Math.max(0,Math.min(100,level));if(handle>=0)Native.audioControl(handle,5,volume);return volume;}
 }
}
