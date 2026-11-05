package javax.microedition.lcdui;
public class ImageItem extends Item {
 public static final int LAYOUT_DEFAULT=0,LAYOUT_LEFT=1,LAYOUT_RIGHT=2,LAYOUT_CENTER=3,LAYOUT_NEWLINE_BEFORE=256,LAYOUT_NEWLINE_AFTER=512,PLAIN=0,HYPERLINK=1,BUTTON=2;
 private Image image;private String alternative;private int layout,appearance;
 public ImageItem(String label,Image image,int layout,String alt){this(label,image,layout,alt,PLAIN);}public ImageItem(String label,Image image,int layout,String alt,int mode){super(label);this.image=image;this.layout=layout;alternative=alt;appearance=mode;}
 public Image getImage(){return image;}public void setImage(Image value){image=value;}public String getAltText(){return alternative;}public void setAltText(String text){alternative=text;}public int getLayout(){return layout;}public void setLayout(int value){layout=value;}public int getAppearanceMode(){return appearance;}
 void render(Graphics g,int x,int y,int width){super.render(g,x,y,width);if(image!=null)g.drawImage(image,x+((layout&3)==LAYOUT_CENTER?(width-image.getWidth())/2:(layout&3)==LAYOUT_RIGHT?width-image.getWidth():0),y+18,20);else if(alternative!=null)g.drawString(alternative,x,y+18,20);}int height(){return 20+(image==null?18:image.getHeight());}
}
