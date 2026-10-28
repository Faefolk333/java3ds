package crimson;import java.io.InputStream;
public final class Resources {
 public static InputStream open(Class owner,String name){if(name==null)return null;if(name.startsWith("/"))name=name.substring(1);else if(owner!=null){String c=owner.getName();int dot=c.lastIndexOf('.');if(dot>=0)name=c.substring(0,dot).replace('.','/')+"/"+name;}return Resources.class.getClassLoader().getResourceAsStream(name);}
}
