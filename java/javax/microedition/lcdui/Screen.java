package javax.microedition.lcdui;
public abstract class Screen extends Canvas {
 protected void keyPressed(int key){if(key==-6||key==-7)command(key);else if(getGameAction(key)==FIRE)command(-6);}
 final void paintCommands(Graphics g){String left=null,right=null;for(int i=0;i<commands.size();i++){Command c=(Command)commands.elementAt(i);int t=c.getCommandType();boolean back=t==Command.BACK||t==Command.CANCEL||t==Command.EXIT;if(back){if(right==null)right=c.getLabel();}else if(left==null)left=c.getLabel();}if(left==null&&right==null)return;g.setColor(0x182b43);g.fillRect(0,getHeight()-18,getWidth(),18);g.setColor(0xffffff);if(left!=null)g.drawString(left,4,getHeight()-14,Graphics.LEFT|Graphics.TOP);if(right!=null)g.drawString(right,getWidth()-4,getHeight()-14,Graphics.RIGHT|Graphics.TOP);}
}
