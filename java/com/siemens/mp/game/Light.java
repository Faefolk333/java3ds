package com.siemens.mp.game;
/** Phone backlight requests do not override the launcher's touch-screen timer. */
public final class Light {private static boolean enabled=true;public static void setLightOn(){enabled=true;}public static void setLightOff(){enabled=false;}}
