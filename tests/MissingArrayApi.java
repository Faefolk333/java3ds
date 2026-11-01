public class MissingArrayApi {
 public static void main(String[] args){try{Object[] a=new MissingDeviceApi[1];throw new RuntimeException("expected missing class");}catch(Throwable expected){System.out.println("MISSING_ARRAY_PASS "+expected.getClass().getName());}}
}
class MissingDeviceApi {}
