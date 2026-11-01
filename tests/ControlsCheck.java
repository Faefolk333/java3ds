import crimson.*;
public class ControlsCheck {
 static void check(boolean v){if(!v)throw new RuntimeException("Controls mismatch");}
 static int bit(int k){for(int i=0;i<Controls.CODES.length;i++)if(Controls.CODES[i]==k)return 1<<i;throw new RuntimeException();}
 public static void main(String[]args){
  GameMain.keyProfile=0;check(Controls.keys(16,true)==bit(-5));Controls.bindings[4]=49;check(Controls.keys(16,true)==bit(49));Controls.bindings[4]=0;
  GameMain.popProfile=true;GameMain.keyProfile=2;
  check(Controls.keys(1|4,true)==bit(-3));check(!Controls.jumpHeld());check(Controls.keys(32|4,true)==bit(49));check(Controls.jumpHeld());check(Controls.keys(32|8,true)==bit(51));check(Controls.keys(16,true)==bit(-5));check(!Controls.jumpHeld());
  Controls.bindings[7]=1001;check(Controls.keys(128|8,true)==bit(51));Controls.bindings[7]=1002;check(Controls.keys(128,true)==bit(-5));
  check(Controls.keys(1<<28,false)==bit(-6));check(Controls.keys(1<<29,false)==bit(-7));
  System.out.println("CONTROLS_PASS defaults custom B_direction A_action stick_no_jump");
 }
}
