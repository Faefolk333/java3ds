public class ClassLoadOOMProbe {public static void main(String[]args)throws Exception{crimson.Native.stage(85,1);Class.forName("OOMTarget");throw new Exception("fault injection did not stop class loading");}}
class OOMTarget {public static final String DATA="target class constant pool allocation";}
