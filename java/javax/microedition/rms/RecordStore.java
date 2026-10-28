package javax.microedition.rms;
import java.io.*;
import java.util.TreeMap;
import java.util.Map;
public final class RecordStore {
 private final File file;private final TreeMap<Integer,byte[]> records=new TreeMap<Integer,byte[]>();private int next=1;
 private RecordStore(String name,boolean create)throws IOException {
  crimson.Native.stage(20,0);
  if(name.length()<1||name.length()>64)throw new IOException("Invalid RMS name");
  for(int i=0;i<name.length();i++)if("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-".indexOf(name.charAt(i))<0)throw new IOException("Invalid RMS name");
  String dir=System.getProperty("pop.saves");if(dir==null)dir="/3ds/pop-classic/saves";
  File root=new File(dir);if(!root.isDirectory()&&!root.mkdirs())throw new IOException("Cannot create save directory");
  crimson.Native.stage(21,0);
  file=new File(root,name+".rms");File backup=new File(file.getPath()+".bak");
  if(!file.exists()&&backup.exists()&&!backup.renameTo(file))throw new IOException("Cannot recover RMS backup");
  if(file.exists()){DataInputStream in=new DataInputStream(new FileInputStream(file));try{
   if(in.readInt()!=0x524d5331)throw new IOException("Invalid RMS header");next=in.readInt();int count=in.readInt();
   if(next<1||count<0||count>10000)throw new IOException("Invalid RMS count");
   for(int i=0;i<count;i++){int id=in.readInt(),n=in.readInt();if(id<1||id>=next||n<0||n>1048576||records.containsKey(id))throw new IOException("Invalid RMS record");byte[] b=new byte[n];in.readFully(b);records.put(id,b);}
  }finally{in.close();}}else if(!create)throw new IOException("Missing RMS");
  crimson.Native.stage(22,records.size());
 }
 public static RecordStore openRecordStore(String name,boolean create)throws IOException{return new RecordStore(name,create);}
 public static void deleteRecordStore(String name)throws IOException {String dir=System.getProperty("pop.saves");if(dir==null)dir="/3ds/pop-classic/saves";if(!name.matches("[a-zA-Z0-9_-]{1,64}"))throw new IOException("Invalid RMS name");new File(dir,name+".rms").delete();new File(dir,name+".rms.bak").delete();}
 public int getRecordSize(int id)throws IOException{return getRecord(id).length;}
 public int getRecord(int id,byte[] b,int offset)throws IOException{byte[] a=getRecord(id);System.arraycopy(a,0,b,offset,a.length);return a.length;}
 public void setRecord(int id,byte[] b,int o,int n)throws IOException{if(!records.containsKey(id))throw new IOException("Missing record");byte[] a=new byte[n];System.arraycopy(b,o,a,0,n);records.put(id,a);}
 public int getNumRecords(){return records.size();}public int getNextRecordID(){return next;}
 public byte[] getRecord(int id)throws IOException{byte[] b=records.get(id);if(b==null)throw new IOException("Missing record");return b.clone();}
 public void deleteRecord(int id){records.remove(id);}
 public int addRecord(byte[] b,int o,int n){byte[] copy=new byte[n];System.arraycopy(b,o,copy,0,n);int id=next++;records.put(id,copy);return id;}
 public void closeRecordStore()throws IOException{
  File temp=new File(file.getPath()+".tmp"),backup=new File(file.getPath()+".bak");
  DataOutputStream out=new DataOutputStream(new FileOutputStream(temp));try{out.writeInt(0x524d5331);out.writeInt(next);out.writeInt(records.size());for(Map.Entry<Integer,byte[]> e:records.entrySet()){out.writeInt(e.getKey());out.writeInt(e.getValue().length);out.write(e.getValue());}out.flush();}finally{out.close();}
  if(backup.exists()&&!backup.delete())throw new IOException("Cannot rotate RMS backup");
  if(file.exists()&&!file.renameTo(backup))throw new IOException("Cannot back up RMS");
  if(!temp.renameTo(file)){if(backup.exists())backup.renameTo(file);throw new IOException("Cannot commit RMS");}
 }
}
