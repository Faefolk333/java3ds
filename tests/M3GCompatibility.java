package javax.microedition.lcdui;
import javax.microedition.m3g.*;
public class M3GCompatibility {
 public static void main(String[]args)throws Exception {
  Image target=Image.createImage(32,32);Graphics3D g=Graphics3D.getInstance();Camera camera=new Camera();camera.setPerspective(60,1,1,100);World world=new World();world.addChild(camera);world.setActiveCamera(camera);Background bg=new Background();bg.setColor(0xff00ff00);world.setBackground(bg);
  VertexArray positions=new VertexArray(3,3,2);positions.set(0,3,new short[]{-1,-1,-4,1,-1,-4,0,1,-4});VertexBuffer vertices=new VertexBuffer();vertices.setPositions(positions,1,null);vertices.setDefaultColor(0xffff0000);TriangleStripArray indices=new TriangleStripArray(new int[]{0,1,2},new int[]{3});Appearance appearance=new Appearance();PolygonMode pm=new PolygonMode();pm.setCulling(PolygonMode.CULL_NONE);appearance.setPolygonMode(pm);Mesh mesh=new Mesh(vertices,indices,appearance);world.addChild(mesh);
  g.bindTarget(target.getGraphics());g.render(world);g.releaseTarget();int[] pixels=new int[1024];target.getRGB(pixels,0,32,0,0,32,32);int red=0,green=0;for(int c:pixels){if((c&0xffffff)==0xff0000)red++;if((c&0xffffff)==0x00ff00)green++;}if(red<5||green<5)throw new Exception("M3G pixels red="+red+" green="+green);System.out.println("M3G_RENDER_PASS triangle="+red+" background="+green);
 }
}
