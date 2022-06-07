
//Define libraries here

#ifdef __cplusplus
extern "C" {
#endif

uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif

#include <Wire.h>
#include <VL53L0X.h>
#include <WiFiClientSecure.h>
#include <Losant.h>
#include <Preferences.h>


//#define HIGH_SPEED
#define HIGH_ACCURACY
#define MQTT_MAX_PACKET_SIZE 256
#define uS_TO_S_FACTOR 1000000ULL    /* Conversion factor for micro seconds to seconds */
//#define TIME_TO_SLEEP  60        /* Time ESP32 will go to sleep (in seconds) */



// WiFi credentials.
const char* Def_WIFI_SSID = "Test_IoT";
const char* Def_WIFI_PASS = "12345678!";

// Losant credentials.
const char* LOSANT_DEVICE_ID = "622f9eeaa4d9b4159b90b15b";
const char* LOSANT_ACCESS_KEY = "6c3dfb58-3058-4afb-8bd8-d710c1ad02bc";
const char* LOSANT_ACCESS_SECRET = "539fdd07d2d18ef78baa992045696521caae27dac99220850a99c8efd47e1416";

// DigiCert Global Root CA  https://www.digicert.com/kb/digicert-root-certificates.htm https://forums.losant.com/t/solved-losant-brokers-mosquitto-dependent-on-losant-losantrootca-crt-are-all-unable-to-connect-to-losant/1801/2
const char* rootCABuff = \
                         "-----BEGIN CERTIFICATE-----\n" \
                         "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
                         "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
                         "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
                         "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
                         "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
                         "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
                         "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
                         "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
                         "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
                         "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
                         "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
                         "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
                         "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
                         "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
                         "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
                         "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
                         "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
                         "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
                         "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
                         "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
                         "-----END CERTIFICATE-----\n";


VL53L0X sensor;

// initiate the the wifi client
WiFiClientSecure wifiClient;
LosantDevice device(LOSANT_DEVICE_ID);

Preferences Conf; //Load config from Memory of ESP32


//Define global variables here

int LaserRead = 0; // in cm

uint8_t TrashBin_Height; // Load values from storage (in cm)
uint8_t Sensor_Height; // Load values from storage (in cm)

uint8_t Error_Code; // Load values from storage (in cm)

uint8_t Auto_height; // Load values from storage
uint8_t Dbug; // Load values from storage
unsigned long TIME_TO_SLEEP;        /* Time ESP32 will go to sleep (in minutes) */

int Trash_percent;
//bool Auto_height = true;
int con_status = 0;
int wait_time = 7;

String WIFI_SSID;
String WIFI_PASS; //WiFi.begin(ssid.c_str(), password.c_str());

float SysTemp;

int watchDog = 0;

void setup()
{
  // put your setup code here, to run once:
  Conf.begin("ESP32_conf", true);  //RO-mode

  TrashBin_Height = Conf.getUChar("BinH", 30); // Load values from storage (in cm)
  Sensor_Height = Conf.getUChar("SensH", 30); // Load values from storage (in cm)
  Auto_height = Conf.getUChar("Auto", 0); // Load values from storage
  Dbug = Conf.getUChar("Dbug", 1); // Load values from storage
  //Dbug = 1;

  WIFI_SSID = Conf.getString("ssid", "");    //.c_str();
  WIFI_PASS = Conf.getString("pass", ""); //WiFi.begin(ssid.c_str(), password.c_str());

  TIME_TO_SLEEP = Conf.getUInt("sleep", 1);        /* Time ESP32 will go to sleep (in min) */

  Conf.end();
  Serial.begin(115200); // Needed for serial print with serial monitor. Data rate is set to 115200 bits per second (baud)
  if (Dbug == 1)
  {
    Serial.println("Fetching values.....");

    Serial.print("Manual Height: ");
    Serial.println(TrashBin_Height);
    Serial.print("Sensor Height: ");
    Serial.println(Sensor_Height);
    Serial.print("Auto Height Mode: ");
    Serial.println(Auto_height);
    Serial.print("Debug mode: ");
    Serial.println(Dbug);
    Serial.print("Node Sleep Time: ");
    Serial.println(TIME_TO_SLEEP);
    Serial.println();

    Serial.print("WIFI_SSID: ");
    Serial.println(WIFI_SSID);

    Serial.print("WIFI_PASS: ");
    Serial.println(WIFI_PASS);
    Serial.println();
  }
  // =================================== Laser SETUP
  Wire.begin();

  sensor.setTimeout(500);

  // =================================== Deep Sleep SETUP

  if (TIME_TO_SLEEP != 0)  //if sleep is not disabled
  {
    //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON); // deep sleep => all RTC Peripherals are powered
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF); // hibernation => all RTC peripherals are deactivated
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR * 60); // set timer here to wake up ESP32
  }

  if (!sensor.init())
  {
    if (Dbug == 1)
    {
      Serial.println("Failed to detect and initialize Laser sensor!");
    }
    while (1) {}
  }

#if defined LONG_RANGE
  // lower the return signal rate limit (default is 0.25 MCPS)
  sensor.setSignalRateLimit(0.1);
  // increase laser pulse periods (defaults are 14 and 10 PCLKs)
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
#endif

#if defined HIGH_SPEED
  // reduce timing budget to 20 ms (default is about 33 ms)
  sensor.setMeasurementTimingBudget(20000);
#elif defined HIGH_ACCURACY
  // increase timing budget to 200 ms
  sensor.setMeasurementTimingBudget(200000);
#endif
  connect_server();  //connect to Server

}

void loop()
{
  // put your main code here, to run repeatedly:
  bool toReconnect = false;
  Error_Code = 0;
  if (Auto_height == 1)    //Re-Calibrate the Sensor height
  {
    Auto_height = 0;
    Serial.println("Calibrating Sensor Height...");
    delay(5000);
    Sensor_Height = laserSensor();
    //TrashBin_Height = laserSensor();
    Serial.print("Sensor height set to: ");
    Serial.println(Sensor_Height);

    Conf.begin("ESP32_conf", false); //R-W mode
    Conf.putUChar("SensH", Sensor_Height); //update Sensor heigh in memory
    //Conf.putUChar("BinH", TrashBin_Height); //update bin heigh in memory
    Conf.putUChar("Auto", Auto_height); //update bin heigh in memory
    Conf.end();
  }
  LaserRead = Sensor_Height - laserSensor(); // Fetch info from sensor actual heigh of trash
  if (LaserRead < 0)
  {
    LaserRead = 0;
    Error_Code = 1;
    if (Dbug == 1)
    {
      Serial.println("Sensor height error!! Recalibrate the Sensor height.");

    }
  }

  Trash_percent = ((float)LaserRead / (float)TrashBin_Height) * 100; //calculate percentage of trash in can by dividint it with trashbin height
  SysTemp = getTemp();
  if (WiFi.status() != WL_CONNECTED)      // Check the status of Wi-Fi
  {
    if (Dbug == 1)
    {
      Serial.println("Disconnected from WiFi");
    }
    toReconnect = true;
  }

  if (!device.connected()) // Check the status of server connection
  {
    if (Dbug == 1)
    {
      Serial.println("Disconnected from Losant");
    }
    toReconnect = true;
  }

  if (toReconnect)      //Re-connect if disconnected from server or WIFI
  {
    con_status = 0;
    watchDog = 0;
    while (con_status != 1)
    {
      delay(5000);                    //wait 10 sec before reconnecting

      con_status = connect_server();
      if (con_status == 1)
      {
        break;
      }
      else if (watchDog == 1)
      {
        ESP.restart(); //Reboot in case it freezes or is unable to connect to the network
      }
      watchDog++;
    }
  }

  //device.loop(); // Keep Losant and WiFi connection alive
  sendDataToCloud(LaserRead, Trash_percent, SysTemp, TrashBin_Height, Sensor_Height, Error_Code, TIME_TO_SLEEP); // push data to server
  if (Dbug == 1)
  {
    Serial.println("Send Now!!");
  }
  delay(1000);
  device.onCommand(&handleCommand);
  device.loop(); // Keep Losant and WiFi connection alive


  if (TIME_TO_SLEEP != 0)  //if sleep is not disabled
  {
    if (Dbug == 1)
    {
      Serial.println("Going to sleep...");
    }
    esp_deep_sleep_start();
  }
}
