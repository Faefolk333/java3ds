package ru.woesss.j2me.micro3d;
import java.io.*;import java.nio.*;
public class TextureImpl {
 public final int width,height;public int[] pixels;
 public TextureImpl(String name)throws IOException{this(Utils.resource(name));}
 public TextureImpl(byte[] data){try{TextureData t=Loader.loadBmpData(data,0,data.length);width=t.width;height=t.height;ByteBuffer b=t.getRaster();pixels=new int[width*height];for(int i=0;i<pixels.length;i++){int r=b.get()&255,g=b.get()&255,blue=b.get()&255,a=b.get()&255;pixels[i]=a<<24|r<<16|g<<8|blue;}}catch(IOException e){throw new IllegalArgumentException(e.toString());}}
 public void dispose(){pixels=null;}
}
