package javax.microedition.lcdui;
public final class Graphics {
 public static final int HCENTER=1,VCENTER=2,LEFT=4,RIGHT=8,TOP=16,BOTTOM=32,BASELINE=64;
 public void readPixels(int[] dst,int o,int scan,int x,int y,int w,int h){x+=tx;y+=ty;if(x<0||y<0||w<0||h<0||x+w>width||y+h>height)throw new IllegalArgumentException();if(dst==null)throw new NullPointerException();if(w==0||h==0)return;long last=(long)o+(long)(h-1)*scan;if(Math.min(o,last)<0||Math.max(o,last)+w>dst.length)throw new ArrayIndexOutOfBoundsException();crimson.Native.readPixels(target,width,height,dst,o,scan,x,y,w,h);}
 public static int images;
 private int tx,ty;private Font font=Font.getDefaultFont();private final int[] target;private final int width,height,origin;private int cx,cy,cw,ch,color;
 Graphics(int[] target,int width,int height,int origin){this.target=target;this.width=width;this.height=height;this.origin=origin;setClip(0,0,width-origin*2,height);}
 public void setColor(int rgb){color=rgb&0xffffff;}public void setColor(int r,int g,int b){setColor((r<<16)|(g<<8)|b);}
 public void setClip(int x,int y,int w,int h){cx=Math.max(origin,x+origin+tx);cy=Math.max(0,y+ty);cw=Math.max(0,Math.min(width-origin,x+origin+tx+Math.max(0,w))-cx);ch=Math.max(0,Math.min(height,y+ty+Math.max(0,h))-cy);}
 public void fillRect(int x,int y,int w,int h){x+=origin+tx;y+=ty;int x1=Math.max(x,cx),y1=Math.max(y,cy),x2=Math.min(x+w,cx+cw),y2=Math.min(y+h,cy+ch);if(x2>x1&&y2>y1)crimson.Native.fill(target,width,height,x1,y1,x2-x1,y2-y1,0xff000000|color);}
 public void drawRect(int x,int y,int w,int h){fillRect(x,y,w+1,1);fillRect(x,y+h,w+1,1);fillRect(x,y,1,h);fillRect(x+w,y,1,h);}
 public void drawLine(int x,int y,int xx,int yy){int dx=Math.abs(xx-x),dy=-Math.abs(yy-y),sx=x<xx?1:-1,sy=y<yy?1:-1,err=dx+dy;for(;;){fillRect(x,y,1,1);if(x==xx&&y==yy)break;int e=err*2;if(e>=dy){err+=dy;x+=sx;}if(e<=dx){err+=dx;y+=sy;}}}
 public void drawImage(Image im,int x,int y,int anchor){if((anchor&HCENTER)!=0)x-=im.w/2;else if((anchor&RIGHT)!=0)x-=im.w;if((anchor&VCENTER)!=0)y-=im.h/2;else if((anchor&BOTTOM)!=0)y-=im.h;if(target==null&&im.w==432&&im.h==240&&crimson.GameMain.menuBackdrop())crimson.Native.region(target,width,height,im.pixels,im.w,im.h,40,0,320,240,8,0,y+ty,cx,cy,cw,ch);else crimson.Native.blit(target,width,height,im.pixels,im.w,im.h,x+origin+tx,y+ty,cx,cy,cw,ch);images++;}
 // Numeric health overlays use a tiny bitmap font; dialogue text uses original sprites.
 private void drawNumeric(String s,int x,int y,int anchor){if((anchor&HCENTER)!=0)x-=s.length()*4/2;else if((anchor&RIGHT)!=0)x-=s.length()*4;if((anchor&(BOTTOM|BASELINE))!=0)y-=5;String chars="0123456789/- ";int[] glyph={31599,11415,29671,29647,23497,31183,31215,29257,31727,31695,4680,448,0};for(int k=0;k<s.length();k++){int n=chars.indexOf(s.charAt(k));if(n<0)n=12;for(int row=0;row<5;row++)for(int col=0;col<3;col++)if((glyph[n]&(1<<(14-row*3-col)))!=0)fillRect(x+k*4+col,y+row,1,1);}}

 public int getTargetWidth(){return width;}public int getTargetHeight(){return height;}public int[] getFrameBuffer(){return target==null?crimson.Native.snapshot():target;}public void commitFrameBuffer(int[] frame){if(target==null)crimson.Native.rgb(null,width,height,frame,0,width,0,0,width,height,false,0,0,width,height);}
 public void drawTriangle(int x1,int y1,int x2,int y2,int x3,int y3){drawLine(x1,y1,x2,y2);drawLine(x2,y2,x3,y3);drawLine(x3,y3,x1,y1);}
 public void microBegin(){crimson.Native.microBegin();}
 public void microTriangle(float[] triangle,float[] projection,int[] texture,int tw,int th,int color,int flags){crimson.Native.microTriangle(target,width,height,triangle,projection,texture,tw,th,color,flags,cx,cy,cw,ch);}
 public int getClipX(){return cx-origin-tx;}public int getClipY(){return cy-ty;}public int getClipWidth(){return cw;}public int getClipHeight(){return ch;}public void setFont(Font f){font=f==null?Font.getDefaultFont():f;}public Font getFont(){return font;}public int getColor(){return color;}public int getRedComponent(){return (color>>16)&255;}public int getGreenComponent(){return (color>>8)&255;}public int getBlueComponent(){return color&255;}public int getDisplayColor(int c){return c&0xffffff;}public void setGrayScale(int c){setColor(c,c,c);}public int getGrayScale(){return (getRedComponent()+getGreenComponent()+getBlueComponent())/3;}public void setStrokeStyle(int s){}public int getStrokeStyle(){return 0;}public static final int SOLID=0,DOTTED=1;
 public void drawRoundRect(int x,int y,int w,int h,int aw,int ah){drawRect(x,y,w,h);}
 public void fillRoundRect(int x,int y,int w,int h,int aw,int ah){fillRect(x,y,w,h);}
 public void fillTriangle(int ax,int ay,int bx,int by,int dx,int dy){
  int lo=Math.min(ay,Math.min(by,dy)),hi=Math.max(ay,Math.max(by,dy));
  for(int y=lo;y<=hi;y++){int min=2147483647,max=-2147483647;int[] xs={ax,bx,dx},ys={ay,by,dy};for(int i=0;i<3;i++){int j=(i+1)%3;if(ys[i]==ys[j])continue;if(y>=Math.min(ys[i],ys[j])&&y<=Math.max(ys[i],ys[j])){int x=xs[i]+(int)((long)(y-ys[i])*(xs[j]-xs[i])/(ys[j]-ys[i]));min=Math.min(min,x);max=Math.max(max,x);}}if(max>=min)fillRect(min,y,max-min+1,1);}
 }
 public void drawRGB(int[] rgb,int offset,int scan,int x,int y,int w,int h,boolean alpha){if(rgb==null)throw new NullPointerException();if(w<0||h<0)throw new IllegalArgumentException();if(w==0||h==0)return;long last=(long)offset+(long)(h-1)*scan;long min=Math.min(offset,last),max=Math.max(offset,last)+w;if(min<0||max>rgb.length)throw new ArrayIndexOutOfBoundsException();crimson.Native.rgb(target,width,height,rgb,offset,scan,x+origin+tx,y+ty,w,h,alpha,cx,cy,cw,ch);}
 public void drawRegion(Image im,int sx,int sy,int w,int h,int t,int x,int y,int anchor){
  int dw=t>=4?h:w,dh=t>=4?w:h;if((anchor&HCENTER)!=0)x-=dw/2;else if((anchor&RIGHT)!=0)x-=dw;if((anchor&VCENTER)!=0)y-=dh/2;else if((anchor&BOTTOM)!=0)y-=dh;
  crimson.Native.region(target,width,height,im.pixels,im.w,im.h,sx,sy,w,h,t,x+origin+tx,y+ty,cx,cy,cw,ch);
 }
 public void translate(int x,int y){tx+=x;ty+=y;}public int getTranslateX(){return tx;}public int getTranslateY(){return ty;}
 public void clipRect(int x,int y,int w,int h){int nx=Math.max(cx,x+origin+tx),ny=Math.max(cy,y+ty);cw=Math.max(0,Math.min(cx+cw,x+origin+tx+Math.max(0,w))-nx);ch=Math.max(0,Math.min(cy+ch,y+ty+Math.max(0,h))-ny);cx=nx;cy=ny;}
 public void drawString(String s,int x,int y,int anchor){if(crimson.GameMain.popProfile){drawNumeric(s,x,y,anchor);return;}if(s==null)throw new NullPointerException();if(s.length()==0)return;int scale=font.scale,w=font.stringWidth(s),h=font.getHeight();if(w>8192)throw new IllegalArgumentException();if((anchor&HCENTER)!=0)x-=w/2;else if((anchor&RIGHT)!=0)x-=w;if((anchor&VCENTER)!=0)y-=h/2;else if((anchor&BOTTOM)!=0)y-=h;else if((anchor&BASELINE)!=0)y-=font.getBaselinePosition();crimson.Native.text(target,width,height,BitmapFont.DATA,s,x+origin+tx,y+ty,scale,color,cx,cy,cw,ch);}

 public void drawSubstring(String s,int o,int n,int x,int y,int a){drawString(s.substring(o,o+n),x,y,a);}public void drawChar(char c,int x,int y,int a){drawString(String.valueOf(c),x,y,a);}public void drawChars(char[]c,int o,int n,int x,int y,int a){drawString(new String(c,o,n),x,y,a);}
 public void drawArc(int x,int y,int w,int h,int start,int arc){for(int angle=start;angle<start+arc;angle++){double a=angle*Math.PI/180,b=(angle+1)*Math.PI/180;drawLine(x+w/2+(int)(Math.cos(a)*w/2),y+h/2-(int)(Math.sin(a)*h/2),x+w/2+(int)(Math.cos(b)*w/2),y+h/2-(int)(Math.sin(b)*h/2));}}
 public void fillArc(int x,int y,int w,int h,int start,int arc){if(w<=0||h<=0)return;for(int yy=0;yy<h;yy++){double d=(2.0*yy/h-1);int half=(int)(w/2.0*Math.sqrt(Math.max(0,1-d*d)));fillRect(x+w/2-half,y+yy,half*2+1,1);}}
}
