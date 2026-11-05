package com.siemens.mp.lcdui;
public class Command extends javax.microedition.lcdui.Command {
 private char icon;
 public Command(String label,int type,int priority){super(label,type,priority);}public Command(String shortLabel,String label,int type,int priority){super(shortLabel,label,type,priority);}public Command(String shortLabel,String label,int type,int priority,char icon){this(shortLabel,label,type,priority);this.icon=icon;}public Command(String label,int type,int priority,char icon){this(label,type,priority);this.icon=icon;}
}
