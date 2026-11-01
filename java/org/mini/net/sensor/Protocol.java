package org.mini.net.sensor;
public class Protocol implements com.sun.cldc.io.ConnectionBaseInterface {public javax.microedition.io.Connection openPrim(String name,int mode,boolean timeouts)throws java.io.IOException{return new javax.microedition.sensor.HidSensorConnection("sensor:"+name);}}
