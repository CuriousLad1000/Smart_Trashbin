

// function that connects to Losant platform


int connect_server()
{
  int connection_ctr = 0;
  // Connect to Wifi.

  if (WIFI_SSID == "" || WIFI_PASS == "")
  {
    Serial.println("No values saved for ssid or password, Switching to default SSID, Pass");
    Serial.print("Connecting to ");
    Serial.println(Def_WIFI_SSID);

    WiFi.begin(Def_WIFI_SSID, Def_WIFI_PASS);
    wifiClient.setCACert(rootCABuff);
  }
  else
  {
    if (Dbug == 1)
    {
      Serial.print("Connecting to ");
      Serial.println(WIFI_SSID);
    }

    WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
    wifiClient.setCACert(rootCABuff);
  }

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    if (Dbug == 1)
    {
      Serial.print(".");
    }

    connection_ctr++;
    if (WiFi.status() == WL_CONNECTED)
    {
      break;
    }
    else if (connection_ctr > 5)
    {
      Serial.println("Switching to Default SSID and Pass....");
      WiFi.begin(Def_WIFI_SSID, Def_WIFI_PASS);
      wifiClient.setCACert(rootCABuff);
      connection_ctr = 0;
    }
  }
  if (Dbug == 1)
  {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // ==================================================Connect to Losant.
    Serial.println();
    Serial.print("Connecting to Losant...");
  }

  device.connectSecure(wifiClient, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);

  while (!device.connected())
  {
    delay(1000);
    if (Dbug == 1)
    {
      Serial.print(".");
    }
    connection_ctr++;
    if (connection_ctr > wait_time)
    {
      if (Dbug == 1)
      {
        Serial.println("NOT Connected!");
      }
      return 0;
    }
  }
  if (Dbug == 1)
  {
    Serial.println("Connected!");
  }
  return 1;
}

void sendDataToCloud(int Laser_sensorValue, int Trashpercent, float ESP_Temp, uint8_t TrashBin_Height, uint8_t Sensor_Height, uint8_t Error_Code, uint8_t TIME_TO_SLEEP)
{
  //Serial.println("Sending data!");

  // Losant uses a JSON protocol. Construct the simple state object.
  StaticJsonDocument<200> jsonBuffer;
  JsonObject root = jsonBuffer.to<JsonObject>();
  // send random values to Losant
  root["laserDist"] = Laser_sensorValue;
  root["TrashPercent"] = Trashpercent;
  root["espTemp"] = ESP_Temp;
  root["TrashBinHeight"] = TrashBin_Height;
  root["SensorHeight"] = Sensor_Height;
  root["ErrorCode"] = Error_Code;
  root["SleepTime"] = TIME_TO_SLEEP;
  if (Dbug == 1)
  {
    //Serial.println("JsonObject to be sent: " );
    serializeJson(root, Serial); // prints json to serial
    Serial.println("");
  }
  // Send the state to Losant.
  device.sendState(root);
}

void handleCommand(LosantCommand *command)
{
  JsonObject& payload = *command->payload;
  if (Dbug == 1)
  {
    serializeJson(payload, Serial);
    Serial.println();
  }

  uint8_t sleep_val = payload["sleep"];
  uint8_t trash_height_val = payload["trash_height"];

  const char* Char_Auto_Calibrate_val = payload["Auto_Calibrate"];
  const char* ssid_val = payload["ssid"];
  const char* pass_val = payload["pass"];
  uint8_t Auto_Calibrate_val;
  const char* Char_Dbug_val = payload["Dbug"];
  uint8_t Dbug_val;

  if (strcmp(Char_Auto_Calibrate_val, "false") == 0)
  {
    Auto_Calibrate_val = 0;
  }
  else
  {
    Auto_Calibrate_val = 1;
  }
  if (strcmp(Char_Dbug_val, "false") == 0)
  {
    Dbug_val = 0;
  }
  else
  {
    Dbug_val = 1;
  }

  Conf.begin("ESP32_conf", false); //R-W mode

  Conf.putUChar("BinH", trash_height_val); //update Trash bin heigh in memory
  Conf.putUChar("Auto", Auto_Calibrate_val); //update bin heigh in memory
  Conf.putUChar("Dbug", Dbug_val); //update bin heigh in memory
  Conf.putUInt("sleep", sleep_val); //update bin heigh in memory

  Conf.putString("ssid", ssid_val);
  Conf.putString("pass", pass_val);

  Conf.end();
  delay(500);
  ESP.restart(); //Reboot the device to load new states
}
