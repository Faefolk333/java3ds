package javax.microedition.lcdui;
public class Item {
 public static final int LAYOUT_DEFAULT=0,LAYOUT_LEFT=1,LAYOUT_RIGHT=2,LAYOUT_CENTER=3,LAYOUT_TOP=16,LAYOUT_BOTTOM=32,LAYOUT_VCENTER=48,LAYOUT_NEWLINE_BEFORE=256,LAYOUT_NEWLINE_AFTER=512,LAYOUT_SHRINK=1024,LAYOUT_EXPAND=2048,LAYOUT_VSHRINK=4096,LAYOUT_VEXPAND=8192,LAYOUT_2=16384,PLAIN=0,HYPERLINK=1,BUTTON=2;
 private String label;private int layout,preferredWidth=-1,preferredHeight=-1;
 protected Item(String label){this.label=label;}public String getLabel(){return label;}public void setLabel(String s){label=s;}
 public int getLayout(){return layout;}public void setLayout(int value){layout=value;}
 public void setPreferredSize(int w,int h){if(w< -1||h< -1)throw new IllegalArgumentException();preferredWidth=w;preferredHeight=h;}
 public int getMinimumWidth(){return 1;}public int getMinimumHeight(){return height();}public int getPreferredWidth(){return preferredWidth<0?80:preferredWidth;}public int getPreferredHeight(){return preferredHeight<0?height():preferredHeight;}
 void render(Graphics g,int x,int y,int width){if(label!=null)g.drawString(label,x,y,Graphics.TOP|Graphics.LEFT);}int height(){return 20;}
}
