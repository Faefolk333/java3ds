import javax.microedition.rms.RecordStore;
public class RmsCompatibility {
 public static void main(String[] a)throws Exception {
  String[] names={"simple_old","Loco Roco Hi","../unsafe/path","Příliš žluťoučký"};
  for(String name:names){RecordStore s=RecordStore.openRecordStore(name,true);int id=s.addRecord(new byte[]{1,2,3},0,3);int empty=s.addRecord(null,0,0);if(s.getRecord(empty).length!=0)throw new Exception("empty record");s.closeRecordStore();s=RecordStore.openRecordStore(name,false);if(s.getRecord(id)[2]!=3)throw new Exception("not persisted");s.closeRecordStore();}
  String[] list=RecordStore.listRecordStores();for(String name:names){boolean found=false;for(String item:list)if(name.equals(item))found=true;if(!found)throw new Exception("missing "+name);RecordStore.deleteRecordStore(name);}
  System.out.println("RMS_PASS names paths unicode persistence list delete");
 }
}
