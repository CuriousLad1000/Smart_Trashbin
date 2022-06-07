//Board: Firebeetle ESP32
//SDA：I2C’s data-wire, connect to IO21 of ESP32.
//SCL：Clock pin of I2C, connect to IO22 of ESP32.
//GND to GND
//VIN to 3V3

int laserSensor()
{
  int Laser_sensorValue = 0;
  if (sensor.timeoutOccurred())
  {
    if (Dbug == 1)
    {
      Serial.println(" TIMEOUT");
    }
  }
  Laser_sensorValue = int(sensor.readRangeSingleMillimeters() / 10);
  //Serial.print("Distance [cm]: ");
  //Serial.print(Laser_sensorValue);

  //Serial.println();

  if (Laser_sensorValue > 200) //if it is beyond 2m or less than 5cm then it is out of range
  {
    Laser_sensorValue = 200;
  }
  else if (Laser_sensorValue < 5)
  {
    Laser_sensorValue = 0;
  }
  return Laser_sensorValue;
}

float getTemp()
{
  float tmp = (temprature_sens_read() - 32) / 1.8; // Convert raw temperature in F to Celsius degrees
  if (Dbug == 1)
  {
    Serial.print("Temperature: ");
    Serial.print(tmp);
    Serial.println(" C");
  }
  return tmp;
}
