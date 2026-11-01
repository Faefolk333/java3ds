import javax.microedition.rms.RecordStore;
public class RmsCompatibility {
 public static void main(String[] a)throws Exception {
  String[] names={"simple_old","Loco Roco Hi","../unsafe/path","Příliš žluťoučký"};
  for(String name:names){RecordStore s=RecordStore.openRecordStore(name,true);int id=s.addRecord(new byte[]{1,2,3},0,3);int empty=s.addRecord(null,0,0);if(s.getRecord(empty).length!=0)throw new Exception("empty record");s.closeRecordStore();s=RecordStore.openRecordStore(name,false);if(s.getRecord(id)[2]!=3)throw new Exception("not persisted");s.closeRecordStore();}
  RecordStore s=RecordStore.openRecordStore("enumeration_test",true);javax.microedition.rms.RecordEnumeration en=s.enumerateRecords(null,null,true);int count=en.numRecords();int id=s.addRecord(new byte[]{7},0,1);if(en.numRecords()!=count+1)throw new Exception("updated enumeration add");s.deleteRecord(id);if(en.numRecords()!=count)throw new Exception("updated enumeration delete");en.destroy();s.closeRecordStore();RecordStore.deleteRecordStore("enumeration_test");
  String[] list=RecordStore.listRecordStores();for(String name:names){boolean found=false;for(String item:list)if(name.equals(item))found=true;if(!found)throw new Exception("missing "+name);RecordStore.deleteRecordStore(name);}
  System.out.println("RMS_PASS names paths unicode persistence list delete");
 }
}
