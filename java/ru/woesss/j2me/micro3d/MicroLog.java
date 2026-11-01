package ru.woesss.j2me.micro3d;
class MicroLog {static void e(String tag,String msg){System.err.println(tag+": "+msg);}static void e(String tag,String msg,Throwable e){e(tag,msg);e.printStackTrace();}static void w(String tag,String msg){e(tag,msg);}static void d(String tag,String msg){}}
