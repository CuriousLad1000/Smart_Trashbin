

// function that connects to Losant platform


int connection_ctr = 0;
int connect_server()
{
  // Connect to Wifi.
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  wifiClient.setCACert(rootCABuff);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Connect to Losant.
  Serial.println();
  Serial.print("Connecting to Losant...");

  device.connectSecure(wifiClient, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);

  while (!device.connected())
  {
    delay(500);
    Serial.print(".");
    connection_ctr++;
    if (connection_ctr > wait_time)
    {
      Serial.println("NOT Connected!");
      return 0;
    }
  }
  Serial.println("Connected!");
  return 1;
}


void sendDataToCloud(int Laser_sensorValue)
{
  //Serial.println("Sending data!");

  // Losant uses a JSON protocol. Construct the simple state object.
  StaticJsonDocument<200> jsonBuffer;
  JsonObject root = jsonBuffer.to<JsonObject>();
  // send random values to Losant
  root["laserDist"] = Laser_sensorValue;

  //Serial.println("JsonObject to be sent: " );
  serializeJson(root, Serial); // prints json to serial
  Serial.println("");

  // Send the state to Losant.
  device.sendState(root);
}
