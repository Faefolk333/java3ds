package crimson;
import java.lang.reflect.Field;
/** The supplied Vir2L games put Select/OK on the right softkey and ignore
 * FIRE in their custom canvas menus. Preserve their own UI command dispatch. */
public final class MenuCompatibility {
 public static boolean enabled;private static Field screen,commands,select,okay;private static Class gameClass;private static String commandList;private static boolean reported;
 private static Field field(Class owner,String name)throws NoSuchFieldException{Field f=owner.getDeclaredField(name);f.setAccessible(true);return f;}
 public static boolean active(){
  if(!enabled||GameMain.midlet==null)return false;
  try{
   Class game=GameMain.midlet.getClass();
   if(game!=gameClass){gameClass=game;commands=null;reported=false;
    try{screen=field(game,"uic");select=field(game,"selectCommand");okay=field(game,"okCommand");commandList="u";}
    catch(NoSuchFieldException obfuscated){screen=field(game,"ax");Class ui=screen.getType();select=field(ui,"u");okay=field(ui,"I");commandList="q";}
   }
   if(screen==null)return false;Object ui=screen.get(null);if(ui==null)return false;
   if(commands==null){Class owner=ui.getClass();while(owner!=null){try{commands=field(owner,commandList);break;}catch(NoSuchFieldException parent){owner=owner.getSuperclass();}}}
   if(commands==null)return false;java.util.Vector list=(java.util.Vector)commands.get(ui);Object primary=select.get(null),ok=okay.get(null);
   for(int i=0;i<list.size();i++){Object command=list.elementAt(i);if(command==primary||command==ok){if(!reported){reported=true;System.out.println("VIR2L_MENU ready: A=right softkey, B=left softkey");}return true;}}
   return false; // A must not activate a lone Exit command on a loading screen.
  }
  catch(Exception unavailable){screen=null;if(!reported){reported=true;System.out.println("Vir2L menu mapping unavailable: "+unavailable.toString());}return false;}
 }
}
