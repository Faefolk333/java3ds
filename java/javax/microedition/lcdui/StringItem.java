package javax.microedition.lcdui;
public class StringItem extends Item {private String text;public StringItem(String label,String text){super(label);this.text=text;}public String getText(){return text;}public void setText(String s){text=s;}void render(Graphics g,int x,int y,int width){super.render(g,x,y,width);if(text!=null)g.drawString(text,x,y+12,20);}int height(){return 32;}}
