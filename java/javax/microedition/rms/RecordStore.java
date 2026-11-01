package javax.microedition.rms;
import java.io.*;
import java.util.TreeMap;
import java.util.Map;
public final class RecordStore {
 private final File file;private final TreeMap<Integer,byte[]> records=new TreeMap<Integer,byte[]>();private int next=1,changes;
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
  }finally{in.close();}}else if(!create)throw new RecordStoreNotFoundException("Missing RMS");
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
 public RecordEnumeration enumerateRecords(RecordFilter filter,RecordComparator comparator,boolean keepUpdated){return new Enumeration(this,filter,comparator,keepUpdated);}
 private static final class Enumeration implements RecordEnumeration {
  private final RecordStore store;private final RecordFilter filter;private final RecordComparator comparator;private boolean keep;private int[] ids;private int cursor,version;private boolean destroyed;
  Enumeration(RecordStore s,RecordFilter f,RecordComparator c,boolean k){store=s;filter=f;comparator=c;keep=k;rebuild();}
  private void update(){if(keep&&!destroyed&&version!=store.changes){int old=cursor,nextId=ids!=null&&cursor<ids.length?ids[cursor]:-1;rebuild();cursor=Math.min(old,ids.length);if(nextId!=-1)for(int i=0;i<ids.length;i++)if(ids[i]==nextId){cursor=i;break;}}}
  public void rebuild(){if(destroyed)return;java.util.ArrayList<Integer> list=new java.util.ArrayList<Integer>();for(Integer id:store.records.keySet())if(filter==null||filter.matches(store.records.get(id).clone()))list.add(id);ids=new int[list.size()];for(int i=0;i<ids.length;i++)ids[i]=list.get(i);if(comparator!=null)for(int i=1;i<ids.length;i++)for(int j=i;j>0&&comparator.compare(store.records.get(ids[j-1]),store.records.get(ids[j]))>0;j--){int t=ids[j];ids[j]=ids[j-1];ids[j-1]=t;}cursor=0;version=store.changes;}
  public int numRecords(){update();return ids.length;}public boolean hasNextElement(){update();return cursor<ids.length;}public boolean hasPreviousElement(){update();return cursor>0;}public int nextRecordId()throws InvalidRecordIDException{if(!hasNextElement())throw new InvalidRecordIDException();return ids[cursor++];}public int previousRecordId()throws InvalidRecordIDException{if(!hasPreviousElement())throw new InvalidRecordIDException();return ids[--cursor];}public byte[] nextRecord()throws java.io.IOException{return store.getRecord(nextRecordId());}public byte[] previousRecord()throws java.io.IOException{return store.getRecord(previousRecordId());}public void reset(){cursor=0;}public void destroy(){destroyed=true;keep=false;ids=new int[0];cursor=0;}public void keepUpdated(boolean b){keep=b;if(b)rebuild();}public boolean isKeptUpdated(){return keep;}
 }
 public int getRecordSize(int id)throws IOException{return getRecord(id).length;}
 public int getRecord(int id,byte[] b,int offset)throws IOException{byte[] a=getRecord(id);System.arraycopy(a,0,b,offset,a.length);return a.length;}
 public void setRecord(int id,byte[] b,int o,int n)throws IOException{if(!records.containsKey(id))throw new InvalidRecordIDException("Missing record");byte[] a=new byte[n];if(n!=0)System.arraycopy(b,o,a,0,n);records.put(id,a);changes++;}
 public int getNumRecords(){return records.size();}public int getNextRecordID(){return next;}
 public byte[] getRecord(int id)throws IOException{byte[] b=records.get(id);if(b==null)throw new InvalidRecordIDException("Missing record");return b.clone();}
 public void deleteRecord(int id){if(records.remove(id)!=null)changes++;}
 public int addRecord(byte[] b,int o,int n){byte[] copy=new byte[n];if(n!=0)System.arraycopy(b,o,copy,0,n);int id=next++;records.put(id,copy);changes++;return id;}
 public void closeRecordStore()throws IOException{
  File temp=new File(file.getPath()+".tmp"),backup=new File(file.getPath()+".bak");
  DataOutputStream out=new DataOutputStream(new FileOutputStream(temp));try{out.writeInt(0x524d5331);out.writeInt(next);out.writeInt(records.size());for(Map.Entry<Integer,byte[]> e:records.entrySet()){out.writeInt(e.getKey());out.writeInt(e.getValue().length);out.write(e.getValue());}out.flush();}finally{out.close();}
  if(backup.exists()&&!backup.delete())throw new IOException("Cannot rotate RMS backup");
  if(file.exists()&&!file.renameTo(backup))throw new IOException("Cannot back up RMS");
  if(!temp.renameTo(file)){if(backup.exists())backup.renameTo(file);throw new IOException("Cannot commit RMS");}
 }
}
