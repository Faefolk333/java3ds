package ru.woesss.j2me.micro3d;
import java.io.*;
public class FigureImpl {
 Model model;float[] transformed,triangle=new float[15];private int pattern;
 public FigureImpl(byte[] data){try{model=Loader.loadMbacData(data,0,data.length);pose(null);}catch(IOException e){throw new IllegalArgumentException(e.toString());}}
 public FigureImpl(String name)throws IOException{this(Utils.resource(name));}
 public void dispose(){model=null;}public int getNumPattern(){return model.numPatterns;}public void setPattern(int p){pattern=p;}public void setPosture(ActTableImpl table,int action,int frame){if(action<0||action>=table.actions.length)throw new IllegalArgumentException();Action a=table.actions[action];for(Action.Bone b:a.boneActions)b.setFrame(Math.max(0,frame));pattern=table.getPattern(action,frame,pattern);pose(a.matrices);}
 private void pose(float[] a){Utils.transform(model.originalVertices,model.vertices,model.originalNormals,model.normals,model.bones,a);}public int getPattern(){return pattern;}
}
