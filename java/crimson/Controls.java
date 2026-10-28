package crimson;
/** Raw 3DS buttons to phone keys. Kept separate so movement never needs a circle gesture. */
public final class Controls {
 private static int raw;public static final int[]bindings=new int[14];
 public static boolean jumpHeld(){return GameMain.keyProfile!=2||(raw&32)!=0;}
 private static boolean climbing(int state){return state==8||state==36||state==37||state==42||state==13;}
 /** Filter the game's decoded player commands, not its menus or scripted actors. */
 public static int playerFlags(int flags,int state){
  if(!jumpHeld()){
   flags&=~(64|4|0x800000);
   boolean combat=state>=56&&state<=81&&state!=77;
   if(!climbing(state)&&!combat)flags&=~(0x400000|0x2000000|512);
  }
  return flags;
 }
 public static final int[] CODES={-1,-2,-3,-4,-5,-7,-6,48,35,49,50,51,55,57,42,52,53,54,56};
 private static int key(int mask,int code){for(int i=0;i<CODES.length;i++)if(CODES[i]==code)return mask|(1<<i);return mask;}
 private static int generic(int held){int result=0;int[] codes=GameMain.keyProfile==1?new int[]{50,56,52,54,53,48,-6,42,35,51,49,55,57,-7}:new int[]{-1,-2,-3,-4,-5,48,-6,42,35,51,49,55,57,-7};int[]bits={1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192};for(int i=0;i<bits.length;i++)if((held&bits[i])!=0)result=key(result,codes[i]);int[] touch={49,50,51,52,53,54,55,56,57,42,48,35};for(int i=0;i<12;i++)if((held&(1<<(16+i)))!=0)result=key(result,touch[i]);return result;}
 public static int keys(int held,boolean gameplay){
  int explicit=0,remapped=held&0x7fff0000;for(int i=0;i<14;i++)if((held&(1<<i))!=0){int b=bindings[i];if(b==0)remapped|=1<<i;else if(b==1001)remapped|=32;else if(b==1002)remapped|=16;else explicit=key(explicit,b);}held=remapped;raw=held;if((held&0x70000)!=0)raw|=32;
  if(!GameMain.popProfile||GameMain.keyProfile!=2)return generic(held)|explicit;
  int keys=held&15;
  if(gameplay&&(keys&12)!=0)keys&=~1; // Diagonal stick input is running, never a jump.
  if((held&8192)!=0||(!gameplay&&(held&32)!=0))keys|=1<<5;
  if((held&64)!=0)keys|=1<<6;
  if((held&128)!=0)keys|=1<<7;
  if((held&256)!=0)keys|=1<<8;
  if((held&1024)!=0)keys|=1<<14;
  if((held&2048)!=0)keys|=1<<12;
  if((held&4096)!=0)keys|=1<<13;
  if((held&(16|512))!=0)keys|=1<<4;
  if(gameplay&&(held&32)!=0){
   keys&=~15; // B supplies one jump command, without a second direction command.
   keys|=1<<(((held&4)!=0&&(held&8)==0)?9:((held&8)!=0&&(held&4)==0)?11:10);
  }
  int[]touch={49,50,51,52,53,54,55,56,57,42,48,35};for(int i=0;i<12;i++)if((held&(1<<(16+i)))!=0)keys=key(keys,touch[i]);
  return keys|explicit;
 }
}
