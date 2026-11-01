package javax.microedition.lcdui;
public class Gauge extends Item {
 public static final int INDEFINITE=-1,CONTINUOUS_IDLE=0,INCREMENTAL_IDLE=1,CONTINUOUS_RUNNING=2,INCREMENTAL_UPDATING=3;
 private final boolean interactive;private int max,value;
 public Gauge(String label,boolean interactive,int maxValue,int initialValue){super(label);this.interactive=interactive;setMaxValue(maxValue);setValue(initialValue);}
 public void setValue(int n){if(max==INDEFINITE){if(n<0||n>3)throw new IllegalArgumentException();value=n;}else value=Math.max(0,Math.min(max,n));}public int getValue(){return value;}public boolean isInteractive(){return interactive;}public int getMaxValue(){return max;}public void setMaxValue(int n){if(n==0||n<INDEFINITE||(interactive&&n==INDEFINITE))throw new IllegalArgumentException();max=n;if(max>0)value=Math.min(value,max);}
 void render(Graphics g,int x,int y,int width){super.render(g,x,y,width);g.drawRect(x,y+16,width,8);if(max>0)g.fillRect(x+1,y+17,(width-2)*value/max,6);}int height(){return 32;}
}
