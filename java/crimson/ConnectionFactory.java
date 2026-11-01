package crimson;
import java.io.*;import javax.microedition.io.*;
/** Adapter used for game calls; the bootstrap Connector retains its existing protocols. */
public final class ConnectionFactory {
 public static final int READ=1,WRITE=2,READ_WRITE=3;
 public static Connection open(String url)throws IOException{return open(url,READ_WRITE,false);}public static Connection open(String url,int mode)throws IOException{return open(url,mode,false);}public static Connection open(String url,int mode,boolean timeout)throws IOException{if(url!=null&&url.startsWith("sensor:"))return new javax.microedition.sensor.HidSensorConnection(url);return Connector.open(url,mode,timeout);}
 public static InputStream openInputStream(String url)throws IOException{return ((InputConnection)open(url,READ)).openInputStream();}public static DataInputStream openDataInputStream(String url)throws IOException{return new DataInputStream(openInputStream(url));}public static OutputStream openOutputStream(String url)throws IOException{return ((OutputConnection)open(url,WRITE)).openOutputStream();}public static DataOutputStream openDataOutputStream(String url)throws IOException{return new DataOutputStream(openOutputStream(url));}
}
