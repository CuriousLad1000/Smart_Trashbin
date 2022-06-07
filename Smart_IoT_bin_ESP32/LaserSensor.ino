//Board: Firebeetle ESP32
//SDA：I2C’s data-wire, connect to IO21 of ESP32.
//SCL：Clock pin of I2C, connect to IO22 of ESP32.
//GND to GND
//VIN to 3V3

int Laser_sensorValue = 0;

int laserSensor()
{
  if (sensor.timeoutOccurred())
  {
    Serial.println(" TIMEOUT");
  }
  Laser_sensorValue = int(sensor.readRangeSingleMillimeters() / 10);
  //Serial.print("Distance [cm]: ");
  //Serial.print(Laser_sensorValue);

  //Serial.println();

  if (Laser_sensorValue > 200) //if it is beyond 2m or less than 5cm then it is out of range handle them
  {
    Laser_sensorValue = 200;
  }
  else if (Laser_sensorValue < 5)
  {
    Laser_sensorValue = 0;
  }
  return Laser_sensorValue;
}
