package javax.microedition.media;
public interface Player extends Controllable {
 int CLOSED=0,UNREALIZED=100,REALIZED=200,PREFETCHED=300,STARTED=400;long TIME_UNKNOWN=-1;
 void setLoopCount(int n);void realize()throws MediaException;void prefetch()throws MediaException;void start()throws MediaException;void stop()throws MediaException;void deallocate();void close();int getState();long setMediaTime(long t);
 long getMediaTime();long getDuration();String getContentType();void addPlayerListener(PlayerListener listener);void removePlayerListener(PlayerListener listener);
}
