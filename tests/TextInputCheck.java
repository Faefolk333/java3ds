import javax.microedition.lcdui.*;
public class TextInputCheck {
 static class TestForm extends Form {TestForm(){super("Name");}void press(int key){keyPressed(key);}}
 static int confirmed;
 public static void main(String[]args)throws Exception{
  TestForm form=new TestForm();TextField field=new TextField("Name",null,10,TextField.ANY);form.append(new StringItem(null,"Enter your name"));form.append(field);form.addCommand(new Command("OK",Command.OK,0));form.setCommandListener(new CommandListener(){public void commandAction(Command c,Displayable d){confirmed++;}});
  form.press(-5);if(!field.getString().equals("Player")||confirmed!=0)throw new Exception("Empty name did not open text input");form.press(-5);if(confirmed!=1)throw new Exception("Filled name did not confirm");
  TestForm readOnly=new TestForm();readOnly.append(new TextField("Locked","",10,TextField.UNEDITABLE));readOnly.addCommand(new Command("OK",Command.OK,0));readOnly.setCommandListener(new CommandListener(){public void commandAction(Command c,Displayable d){confirmed++;}});readOnly.press(-5);if(confirmed!=2)throw new Exception("Read-only field swallowed confirmation");
  crimson.MenuCompatibility.enabled=true;Command ok=new Command("OK",3,0),cancel=new Command("Cancel",4,0);if(ok.getCommandType()!=Command.OK||cancel.getCommandType()!=Command.CANCEL)throw new Exception("Vir2L legacy command types");
  System.out.println("TEXT_INPUT_PASS empty_name_entry confirm_after_entry legacy_command_types");
 }
}
