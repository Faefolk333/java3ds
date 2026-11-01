package javax.microedition.media;
import java.io.*;
import crimson.Native;
import javax.microedition.media.control.VolumeControl;
public final class Manager {
 private static final java.util.Vector players=new java.util.Vector();
 public static void pump(){int count;synchronized(players){count=players.size();}for(int i=0;i<count;i++){NativePlayer p;synchronized(players){if(i>=players.size())break;p=(NativePlayer)players.elementAt(i);}if(p.state==400)p.getState();}}

 public static String[] getSupportedContentTypes(String protocol){return new String[]{"audio/midi","audio/x-midi","audio/wav","audio/x-wav","audio/amr","audio/x-amr"};}public static String[] getSupportedProtocols(String type){return new String[]{"resource"};}
 public static Player createPlayer(String url)throws IOException,MediaException{if(url==null)throw new NullPointerException();if(!url.startsWith("resource:"))throw new MediaException("Unsupported media protocol");InputStream in=crimson.Resources.open(Manager.class,url.substring(9));try{return createPlayer(in,null);}finally{if(in!=null)in.close();}}
 public static Player createPlayer(InputStream in,String mime)throws IOException,MediaException{
  if(in==null)throw new IOException("Missing media");
  ByteArrayOutputStream out=new ByteArrayOutputStream();byte[] buffer=new byte[1024];int n;
  while((n=in.read(buffer))!=-1){if(n==0)continue;if(out.size()+n>1024*1024)throw new IOException("Media too large");out.write(buffer,0,n);}
  return new NativePlayer(out.toByteArray(),mime);
 }
 private static final class NativePlayer implements Player,VolumeControl {
  private byte[] data;private int handle=-1,state=100,loops=1,volume=100;private long position;private boolean muted;private final String mime;private final java.util.Vector listeners=new java.util.Vector();
  NativePlayer(byte[] bytes,String type){data=bytes;mime=type!=null?type:bytes.length>=4&&bytes[0]=='M'&&bytes[1]=='T'?"audio/midi":bytes.length>=5&&bytes[0]=='#'?"audio/amr":"audio/wav";}
  private void event(String name,Object data){PlayerListener[] copy;synchronized(listeners){if(listeners.size()==0)return;copy=new PlayerListener[listeners.size()];for(int i=0;i<copy.length;i++)copy[i]=(PlayerListener)listeners.elementAt(i);}for(int i=0;i<copy.length;i++)try{copy[i].playerUpdate(this,name,data);}catch(Throwable e){System.err.println("Player listener: "+e);}}
  public void addPlayerListener(PlayerListener value){if(value==null)throw new IllegalArgumentException();synchronized(listeners){if(!listeners.contains(value))listeners.addElement(value);}}
  public void removePlayerListener(PlayerListener value){synchronized(listeners){listeners.removeElement(value);}}
  public Control[] getControls(){return new Control[]{this};}
  public String getContentType(){return mime;}
  public synchronized long getDuration(){return handle>=0?Native.audioControl(handle,9,0)*1000L:-1;}
  public synchronized long getMediaTime(){return handle>=0?Native.audioControl(handle,7,0)*1000L:position;}
  public synchronized boolean isMuted(){return muted;}
  public synchronized int getLevel(){return volume;}
  public synchronized void setMute(boolean value){muted=value;if(handle>=0)Native.audioControl(handle,5,value?0:volume);event(PlayerListener.VOLUME_CHANGED,this);}

  public synchronized Control getControl(String name){return name!=null&&name.endsWith("VolumeControl")?this:null;}
  public synchronized void setLoopCount(int n){if(n==0||n < -1)throw new IllegalArgumentException("loop count");loops=n;}
  public synchronized void realize()throws MediaException{
   if(state==0)throw new IllegalStateException("closed");if(state>=200)return;
   handle=Native.audioLoad(data);if(handle<0){String header="";for(int i=0;i<data.length&&i<8;i++)header+=Integer.toHexString(data[i]&255)+" ";throw new MediaException("Unsupported media "+mime+" header="+header);}data=null;state=200;
   Native.audioControl(handle,5,muted?0:volume);synchronized(players){players.addElement(this);}event(PlayerListener.DURATION_UPDATED,Long.valueOf(getDuration()));
  }
  public synchronized void prefetch()throws MediaException{realize();if(state<300)state=300;}
  public synchronized void start()throws MediaException{
   prefetch();if(getState()==400)return;
   Native.audioControl(handle,4,(int)(position/1000));
   if(Native.audioControl(handle,1,loops)<0)throw new MediaException("Cannot start media");state=400;event(PlayerListener.STARTED,Long.valueOf(position));
  }
  public synchronized void stop(){if(handle>=0){position=Native.audioControl(handle,7,0)*1000L;Native.audioControl(handle,2,0);}if(state>=300){state=300;event(PlayerListener.STOPPED,Long.valueOf(position));}}
  public synchronized void deallocate(){stop();if(state>=200)state=200;}
  public synchronized void close(){if(state==0)return;if(handle>=0)Native.audioControl(handle,3,0);handle=-1;data=null;state=0;synchronized(players){players.removeElement(this);}event(PlayerListener.CLOSED,null);}
  public synchronized int getState(){if(state==400&&Native.audioControl(handle,6,0)!=400){state=300;position=0;event(PlayerListener.END_OF_MEDIA,Long.valueOf(getMediaTime()));}return state;}
  public synchronized long setMediaTime(long microseconds){position=Math.max(0,Math.min(2147483647000L,microseconds));if(handle>=0)position=Native.audioControl(handle,4,(int)(position/1000))*1000L;return position;}
  public synchronized int setLevel(int level){volume=Math.max(0,Math.min(100,level));if(handle>=0)Native.audioControl(handle,5,muted?0:volume);event(PlayerListener.VOLUME_CHANGED,this);return volume;}
 }
}
