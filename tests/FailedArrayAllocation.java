public class FailedArrayAllocation {
 public static void main(String[]args)throws Exception{try{int[] a=new int[19200];throw new Exception("fault injection did not run");}catch(OutOfMemoryError expected){System.out.println("FAILED_CALLOC_PASS caught OutOfMemoryError");}}
}
