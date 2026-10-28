package javax.microedition.media;
public interface Player extends Controllable { void setLoopCount(int n);void realize()throws MediaException;void prefetch()throws MediaException;void start()throws MediaException;void stop()throws MediaException;void deallocate();void close();int getState();long setMediaTime(long t); }
