import javax.microedition.media.*;import javax.microedition.media.control.VolumeControl;import java.io.*;
public class MediaPlayerCheck implements PlayerListener {
 int started,stopped,ended,closed;
 public void playerUpdate(Player p,String event,Object data){if(event.equals(STARTED))started++;if(event.equals(STOPPED))stopped++;if(event.equals(END_OF_MEDIA))ended++;if(event.equals(CLOSED))closed++;}
 public static void main(String[]args)throws Exception{
  byte[] wav=new byte[44+160];byte[] head={'R','I','F','F'};System.arraycopy(head,0,wav,0,4);wav[4]=(byte)(wav.length-8);wav[28]=0x40;wav[29]=0x1f;wav[32]=1;wav[8]='W';wav[9]='A';wav[10]='V';wav[11]='E';wav[12]='f';wav[13]='m';wav[14]='t';wav[15]=' ';wav[16]=16;wav[20]=1;wav[22]=1;wav[24]=0x40;wav[25]=0x1f;wav[34]=8;wav[36]='d';wav[37]='a';wav[38]='t';wav[39]='a';wav[40]=(byte)160;for(int i=44;i<wav.length;i++)wav[i]=(byte)128;
  Player p=Manager.createPlayer(new ByteArrayInputStream(wav),"audio/wav");MediaPlayerCheck listener=new MediaPlayerCheck();p.addPlayerListener(listener);p.prefetch();if(p.getDuration()!=20000||p.getControls().length!=1)throw new Exception("media duration/control");VolumeControl v=(VolumeControl)p.getControl("VolumeControl");v.setLevel(65);v.setMute(true);if(!v.isMuted()||v.getLevel()!=65)throw new Exception("mute lost level");v.setMute(false);p.start();Thread.sleep(100);Manager.pump();if(listener.started!=1||listener.ended!=1||p.getState()!=Player.PREFETCHED)throw new Exception("events started="+listener.started+" ended="+listener.ended);p.setMediaTime(0);p.start();p.stop();if(listener.stopped!=1)throw new Exception("stop event");p.close();if(listener.closed!=1)throw new Exception("close event");System.out.println("MEDIA_PLAYER_PASS listeners duration EOF mute getControls restart");
 }
}
