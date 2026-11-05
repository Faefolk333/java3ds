package javax.microedition.lcdui;
public class Spacer extends Item {private int width,height;public Spacer(int w,int h){super(null);setMinimumSize(w,h);}public void setMinimumSize(int w,int h){if(w<0||h<0)throw new IllegalArgumentException();width=w;height=h;}int height(){return height;}}
