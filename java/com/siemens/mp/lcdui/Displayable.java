package com.siemens.mp.lcdui;
/** Optional vendor title artwork is retained separately from game pixels. */
public final class Displayable {private static final java.util.Hashtable icons=new java.util.Hashtable();public static void setHeadlineIcon(javax.microedition.lcdui.Displayable screen,javax.microedition.lcdui.Image icon){if(screen==null)throw new NullPointerException();if(icon==null)icons.remove(screen);else icons.put(screen,icon);}}
