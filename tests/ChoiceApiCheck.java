import javax.microedition.lcdui.*;
public class ChoiceApiCheck {
 static class TestForm extends Form {TestForm(){super("commands");}void press(int key){keyPressed(key);}}
 static int actions;
 static void require(boolean b)throws Exception{if(!b)throw new Exception("choice contract");}
 public static void main(String[] args)throws Exception{
  ChoiceGroup c=new ChoiceGroup("Options",Choice.MULTIPLE,new String[]{"one","two","three"},null);
  c.setSelectedIndex(0,true);c.setSelectedIndex(2,true);boolean[] f=new boolean[5];require(c.getSelectedIndex()==-1&&c.getSelectedFlags(f)==2&&f[0]&&!f[1]&&f[2]&&!f[3]);
  c.insert(0,"zero",null);require(c.isSelected(1)&&c.isSelected(3));c.delete(1);require(c.isSelected(2));c.setSelectedFlags(new boolean[]{true,true,false});require(c.getSelectedFlags(f)==2);c.deleteAll();require(c.size()==0);
  List l=new List("Single",Choice.EXCLUSIVE,new String[]{"a","b"},null);l.setSelectedIndex(1,true);l.insert(0,"z",null);require(l.getSelectedIndex()==2);l.delete(0);require(l.getSelectedIndex()==1);l.setSelectedFlags(new boolean[]{false,false});require(l.getSelectedIndex()==0);
  boolean bad=false;try{l.getSelectedFlags(new boolean[1]);}catch(IllegalArgumentException e){bad=true;}require(bad);
  TestForm form=new TestForm();form.addCommand(new Command("OK",Command.OK,0));form.addCommand(new Command("Back",Command.BACK,1));form.setCommandListener(new CommandListener(){public void commandAction(Command c,Displayable d){actions+=c.getCommandType()==Command.BACK?10:1;}});form.press(-6);form.press(-7);form.press(-5);require(actions==12);
  TextField t=new TextField("Name","test",8,TextField.ANY);t.insert("!",4);require(t.getString().equals("test!"));t.delete(0,1);require(t.size()==4);
  com.siemens.mp.lcdui.Form vendor=new com.siemens.mp.lcdui.Form("Vendor");vendor.append(t);com.siemens.mp.lcdui.Displayable.setHeadlineIcon(vendor,null);
  System.out.println("CHOICE_API_PASS multiple exclusive edits textfield vendor_form");
 }
}
