package javax.microedition.rms;
import java.io.*;
import java.util.TreeMap;
import java.util.Map;
public final class RecordStore {
 private final File file;private final TreeMap<Integer,byte[]> records=new TreeMap<Integer,byte[]>();private int next=1;
 private RecordStore(String name,boolean create)throws IOException {
  crimson.Native.stage(20,0);
  String diskName=encodeName(name);
  String dir=System.getProperty("pop.saves");if(dir==null)dir="/3ds/pop-classic/saves";
  File root=new File(dir);if(!root.isDirectory()&&!root.mkdirs())throw new IOException("Cannot create save directory");
  crimson.Native.stage(21,0);
  file=new File(root,diskName+".rms");File backup=new File(file.getPath()+".bak");
  if(!file.exists()&&backup.exists()&&!backup.renameTo(file))throw new IOException("Cannot recover RMS backup");
  if(file.exists()){DataInputStream in=new DataInputStream(new FileInputStream(file));try{
   if(in.readInt()!=0x524d5331)throw new IOException("Invalid RMS header");next=in.readInt();int count=in.readInt();
   if(next<1||count<0||count>10000)throw new IOException("Invalid RMS count");
   for(int i=0;i<count;i++){int id=in.readInt(),n=in.readInt();if(id<1||id>=next||n<0||n>1048576||records.containsKey(id))throw new IOException("Invalid RMS record");byte[] b=new byte[n];in.readFully(b);records.put(id,b);}
  }finally{in.close();}}else if(!create)throw new IOException("Missing RMS");
  crimson.Native.stage(22,records.size());
 }
 public static RecordStore openRecordStore(String name,boolean create)throws IOException{return new RecordStore(name,create);}
 public static void deleteRecordStore(String name)throws IOException {String dir=System.getProperty("pop.saves");if(dir==null)dir="/3ds/pop-classic/saves";String diskName=encodeName(name);new File(dir,diskName+".rms").delete();new File(dir,diskName+".rms.bak").delete();}
 private static String encodeName(String name)throws IOException {
  if(name==null||name.length()<1||name.length()>64)throw new IOException("Invalid RMS name");
  boolean simple=true;for(int i=0;i<name.length();i++)if("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-".indexOf(name.charAt(i))<0)simple=false;
  if(simple)return name;StringBuffer out=new StringBuffer("~");String hex="0123456789abcdef";
  for(int i=0;i<name.length();i++){int c=name.charAt(i);for(int shift=12;shift>=0;shift-=4)out.append(hex.charAt((c>>shift)&15));}return out.toString();
 }
 public int getSize(){int n=0;for(byte[] b:records.values())n+=b.length;return n;}
 public int getSizeAvailable(){return 4*1024*1024-getSize();}
 public int getVersion(){return next;}
 public static RecordStore openRecordStore(String name,boolean create,int authmode,boolean writable)throws IOException{return openRecordStore(name,create);}
 public static String[] listRecordStores(){
  String dir=System.getProperty("pop.saves","/3ds/java-3ds/saves/default");String[] files=new File(dir).list();if(files==null)return null;
  java.util.Vector names=new java.util.Vector();for(int i=0;i<files.length;i++){String n=files[i];if(!n.endsWith(".rms"))continue;n=n.substring(0,n.length()-4);if(n.startsWith("~")){if((n.length()-1)%4!=0)continue;StringBuffer decoded=new StringBuffer();try{for(int j=1;j<n.length();j+=4)decoded.append((char)Integer.parseInt(n.substring(j,j+4),16));}catch(NumberFormatException e){continue;}n=decoded.toString();}names.addElement(n);}
  if(names.isEmpty())return null;String[] result=new String[names.size()];names.copyInto(result);return result;
 }
 public int getRecordSize(int id)throws IOException{return getRecord(id).length;}
 public int getRecord(int id,byte[] b,int offset)throws IOException{byte[] a=getRecord(id);System.arraycopy(a,0,b,offset,a.length);return a.length;}
 public void setRecord(int id,byte[] b,int o,int n)throws IOException{if(!records.containsKey(id))throw new IOException("Missing record");byte[] a=new byte[n];if(n!=0)System.arraycopy(b,o,a,0,n);records.put(id,a);}
 public int getNumRecords(){return records.size();}public int getNextRecordID(){return next;}
 public byte[] getRecord(int id)throws IOException{byte[] b=records.get(id);if(b==null)throw new IOException("Missing record");return b.clone();}
 public void deleteRecord(int id){records.remove(id);}
 public int addRecord(byte[] b,int o,int n){byte[] copy=new byte[n];if(n!=0)System.arraycopy(b,o,copy,0,n);int id=next++;records.put(id,copy);return id;}
 public void closeRecordStore()throws IOException{
  File temp=new File(file.getPath()+".tmp"),backup=new File(file.getPath()+".bak");
  DataOutputStream out=new DataOutputStream(new FileOutputStream(temp));try{out.writeInt(0x524d5331);out.writeInt(next);out.writeInt(records.size());for(Map.Entry<Integer,byte[]> e:records.entrySet()){out.writeInt(e.getKey());out.writeInt(e.getValue().length);out.write(e.getValue());}out.flush();}finally{out.close();}
  if(backup.exists()&&!backup.delete())throw new IOException("Cannot rotate RMS backup");
  if(file.exists()&&!file.renameTo(backup))throw new IOException("Cannot back up RMS");
  if(!temp.renameTo(file)){if(backup.exists())backup.renameTo(file);throw new IOException("Cannot commit RMS");}
 }
}
