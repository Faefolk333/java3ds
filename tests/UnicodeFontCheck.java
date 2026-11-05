package javax.microedition.lcdui;
public class UnicodeFontCheck {
 public static void main(String[]args)throws Exception{
  String letters="ПриветЁёěčΩω";Font f=Font.getDefaultFont();Image image=Image.createImage(letters.length()*8,16);Graphics g=image.getGraphics();g.setColor(0);g.fillRect(0,0,image.getWidth(),16);g.setColor(0xffffff);g.drawString(letters,0,0,20);int[] pixels=new int[image.getWidth()*16];image.getRGB(pixels,0,image.getWidth(),0,0,image.getWidth(),16);
  for(int i=0;i<letters.length();i++){int code=letters.charAt(i);long glyph=BitmapFont.DATA[code];if(glyph==0||glyph==BitmapFont.DATA[63])throw new Exception("missing Unicode glyph "+code);for(int y=0;y<8;y++)for(int x=0;x<8;x++){boolean lit=(glyph&(1L<<(y*8+x)))!=0;if(((pixels[y*image.getWidth()+i*8+x]&0xffffff)!=0)!=lit)throw new Exception("Unicode raster mismatch "+code);}}
  if(f.stringWidth(letters)!=letters.length()*8)throw new Exception("UTF16 metrics");System.out.println("UNICODE_FONT_PASS Cyrillic Greek Latin_extended actual_native_pixels metrics");
 }
}
