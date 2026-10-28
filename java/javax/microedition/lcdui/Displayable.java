package javax.microedition.lcdui;
public class Displayable {protected final java.util.Vector commands=new java.util.Vector();protected CommandListener listener;private String title;
 public boolean isShown(){return Display.current==this;}public int getWidth(){return crimson.GameMain.width;}public int getHeight(){return crimson.GameMain.height;}
 public void addCommand(Command c){if(c!=null&&!commands.contains(c))commands.addElement(c);}public void removeCommand(Command c){commands.removeElement(c);}public void setCommandListener(CommandListener c){listener=c;}public void setTitle(String s){title=s;}public String getTitle(){return title;}public boolean isFullScreenMode(){return true;}
 protected boolean command(int key){if(listener==null||commands.isEmpty())return false;Command c=null;for(int i=0;i<commands.size();i++){Command v=(Command)commands.elementAt(i);boolean back=v.getCommandType()==Command.BACK||v.getCommandType()==Command.CANCEL||v.getCommandType()==Command.EXIT;if((key==-7&&back)||(key==-6&&!back)){c=v;break;}}if(c==null&&key==-6)c=(Command)commands.elementAt(0);if(c!=null){listener.commandAction(c,this);return true;}return false;}
 void render(Graphics g){g.setColor(0x132437);g.fillRect(0,0,getWidth(),getHeight());g.setColor(0xffffff);g.drawString(title==null?"Java":title,8,8,Graphics.TOP|Graphics.LEFT);}
}
