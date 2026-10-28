package javax.microedition.midlet;
public abstract class MIDlet {
 protected abstract void startApp()throws MIDletStateChangeException;protected abstract void pauseApp();protected abstract void destroyApp(boolean unconditional)throws MIDletStateChangeException;
 public final void launch()throws MIDletStateChangeException{startApp();}public final void shutdown()throws MIDletStateChangeException{destroyApp(true);}
 public String getAppProperty(String key){return (String)crimson.GameMain.properties.get(key);}
 public boolean platformRequest(String url){return false;}public void notifyDestroyed(){crimson.GameMain.stopped=true;crimson.Native.stage(80,0);}public void notifyPaused(){}public void resumeRequest(){}
}
