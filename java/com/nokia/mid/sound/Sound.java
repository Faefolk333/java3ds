package com.nokia.mid.sound;
import java.io.ByteArrayInputStream;
import javax.microedition.media.*;
import javax.microedition.media.control.VolumeControl;
/** Nokia audio facade backed by the shared MIDI/WAV player. */
public class Sound {
 public static final int FORMAT_TONE=1,FORMAT_WAV=5,SOUND_PLAYING=0,SOUND_STOPPED=1,SOUND_UNINITIALIZED=3;
 private Player player;private int gain=255;private SoundListener listener;
 public Sound(byte[] data,int type){init(data,type);}
 public Sound(int frequency,long duration){init(frequency,duration);}
 public static int[] getSupportedFormats(){return new int[]{FORMAT_TONE,FORMAT_WAV};}
 public static int getConcurrentSoundCount(int type){return 1;}
 public synchronized void init(byte[] data,int type){if(data==null)throw new NullPointerException();if(type!=1&&type!=5)throw new IllegalArgumentException();release();try{player=Manager.createPlayer(new ByteArrayInputStream(data),type==5?"audio/wav":"audio/midi");setGain(gain);}catch(Exception e){System.err.println("Nokia audio: "+e);}}
 public synchronized void init(int frequency,long duration){if(frequency<0||frequency>20000||duration<=0||duration>60000)throw new IllegalArgumentException();int samples=(int)(duration*8000/1000);byte[] wav=new byte[44+samples];wav[0]=82;wav[1]=73;wav[2]=70;wav[3]=70;put(wav,4,36+samples,4);wav[8]=87;wav[9]=65;wav[10]=86;wav[11]=69;wav[12]=102;wav[13]=109;wav[14]=116;wav[15]=32;put(wav,16,16,4);put(wav,20,1,2);put(wav,22,1,2);put(wav,24,8000,4);put(wav,28,8000,4);put(wav,32,1,2);put(wav,34,8,2);wav[36]=100;wav[37]=97;wav[38]=116;wav[39]=97;put(wav,40,samples,4);for(int i=0;i<samples;i++)wav[44+i]=(byte)(128+(frequency==0?0:32*Math.sin(2*Math.PI*frequency*i/8000)));init(wav,FORMAT_WAV);}
 private static void put(byte[] b,int o,int v,int n){for(int i=0;i<n;i++)b[o+i]=(byte)(v>>>(8*i));}
 public synchronized void play(int loops){if(loops<0)throw new IllegalArgumentException();try{if(player!=null){player.stop();player.setMediaTime(0);player.setLoopCount(loops==0?-1:loops);player.start();event(SOUND_PLAYING);}}catch(Exception e){System.err.println("Nokia play: "+e);}}
 public synchronized void stop(){try{if(player!=null)player.stop();}catch(Exception e){}event(SOUND_STOPPED);}
 public synchronized void resume(){try{if(player!=null)player.start();}catch(Exception e){}}
 public synchronized void release(){if(player!=null)player.close();player=null;}
 public synchronized int getState(){return player==null?SOUND_UNINITIALIZED:player.getState()==400?SOUND_PLAYING:SOUND_STOPPED;}
 public synchronized int getGain(){return gain;}
 public synchronized void setGain(int v){gain=Math.max(0,Math.min(255,v));if(player!=null){VolumeControl c=(VolumeControl)player.getControl("VolumeControl");if(c!=null)c.setLevel(gain*100/255);}}
 public synchronized void setSoundListener(SoundListener value){listener=value;}
 private void event(int state){if(listener!=null)listener.soundStateChanged(this,state);}
}
