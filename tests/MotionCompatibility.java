import javax.microedition.sensor.*;import javax.microedition.io.*;
public class MotionCompatibility {
 public static void main(String[]a)throws Exception {
  SensorInfo[] info=SensorManager.findSensors(null,null);if(info.length!=2)throw new Exception("discovery");SensorConnection acceleration=(SensorConnection)Connector.open("sensor:acceleration");Data[] data=acceleration.getData(2);if(data.length!=3||data[2].getIntValues()[0]!=9807)throw new Exception("acceleration mapping");acceleration.close();SensorConnection gyro=(SensorConnection)Connector.open("sensor:angular_velocity");data=gyro.getData(1);if(data[0].getIntValues()[0]!=1250||data[1].getIntValues()[0]!=-2500)throw new Exception("gyro mapping");gyro.close();System.out.println("MOTION_PASS discovery connector buffers axes close (host fixtures)");
 }
}
