package javax.microedition.lcdui;
public class Item {
 private String label;protected Item(String label){this.label=label;}public String getLabel(){return label;}public void setLabel(String s){label=s;}void render(Graphics g,int x,int y,int width){if(label!=null)g.drawString(label,x,y,Graphics.TOP|Graphics.LEFT);}int height(){return 20;}
}
