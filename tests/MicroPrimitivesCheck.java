import javax.microedition.lcdui.*;import com.mascotcapsule.micro3d.v3.*;
public class MicroPrimitivesCheck {
 public static void main(String[]args)throws Exception{
  Image image=Image.createImage(64,64);Graphics graphics=image.getGraphics();Graphics3D renderer=new Graphics3D();renderer.bind(graphics);FigureLayout layout=new FigureLayout(null,4096,4096,32,32);Effect3D effect=new Effect3D();
  renderer.renderPrimitives(null,0,0,layout,effect,Graphics3D.PRIMITVE_QUADS|Graphics3D.PDATA_COLOR_PER_COMMAND,1,new int[]{-10,-10,10,10,-10,10,10,10,10,-10,10,10},new int[0],new int[0],new int[]{0x20c070});renderer.release(graphics);
  int[] pixels=new int[4096];image.getRGB(pixels,0,64,0,0,64,64);if((pixels[33*64+32]&0xffffff)!=0x20c070)throw new Exception("Quad primitives did not rasterize");
  renderer.bind(graphics);renderer.renderPrimitives(null,0,0,layout,effect,Graphics3D.PRIMITVE_TRIANGLES|Graphics3D.PDATA_COLOR_PER_FACE,1,new int[]{-8,-8,5,8,-8,5,0,8,5},new int[0],new int[0],new int[]{0xc03060});renderer.release(graphics);image.getRGB(pixels,0,64,0,0,64,64);if((pixels[32*64+32]&0xffffff)!=0xc03060)throw new Exception("Triangle primitives did not rasterize");
  boolean bounded=false;renderer.bind(graphics);try{renderer.renderPrimitives(null,0,0,layout,effect,Graphics3D.PRIMITVE_QUADS,256,new int[0],new int[0],new int[0],new int[0]);}catch(IllegalArgumentException e){bounded=true;}renderer.release(graphics);if(!bounded)throw new Exception("Unbounded primitives");
  System.out.println("MICRO_PRIMITIVES_PASS quad triangle actual_pixels command_color face_color count_bounds");
 }
}
