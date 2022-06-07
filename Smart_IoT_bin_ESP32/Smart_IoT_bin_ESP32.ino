

//Define libraries here

#include <Wire.h>
#include <VL53L0X.h>
#include <WiFiClientSecure.h>
#include <Losant.h>

//#define HIGH_SPEED
#define HIGH_ACCURACY
#define MQTT_MAX_PACKET_SIZE 256
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  10        /* Time ESP32 will go to sleep (in seconds) */



// WiFi credentials.
const char* WIFI_SSID = "Test_IoT";    //your SSID
const char* WIFI_PASS = "12345678!"; //your pass

// Losant credentials.
const char* LOSANT_DEVICE_ID = "622f9eeaa4d9b4159b90b15b";  //your device ID
const char* LOSANT_ACCESS_KEY = "6c3dfb58-3058-4afb-8bd8-d710c1ad02bc"; //your access key
const char* LOSANT_ACCESS_SECRET = "539fdd07d2d18ef78baa992045696521caae27dac99220850a99c8efd47e1416"; //your access secret

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




//Define global variables here

int LaserRead = 0; // in cm
int TrashBin_Height = 30; //in cm
int var = 0;
int con_status = 0;
int wait_time = 10;

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(115200); // Needed for serial print with serial monitor. Data rate is set to 115200 bits per second (baud)
  delay(10); // delay of 10 microseconds
  Serial.println("");

  // =================================== Laser SETUP
  Wire.begin();

  sensor.setTimeout(500);  //Serial.setTimeout(2000);

  //esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON); // deep sleep => all RTC Peripherals are powered
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF); // hibernation => all RTC peripherals are deactivated
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); // set timer here to wake up ESP32

  if (!sensor.init())
  {
    Serial.println("Failed to detect and initialize Laser sensor!");
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

  Serial.print("Trashbin Height set to: ");
  Serial.println(TrashBin_Height);
}

void loop()
{
  // put your main code here, to run repeatedly:
  bool toReconnect = false;

  LaserRead = TrashBin_Height - laserSensor(); // Fetch info from sensor
  if (LaserRead < 0) //countering some height errors
  {
    LaserRead = 0;
    Serial.print("Bin height error!! Re-configure the Trashbin height.");
  }

  if (WiFi.status() != WL_CONNECTED)      // Check the status of Wi-Fi
  {
    Serial.println("Disconnected from WiFi");
    toReconnect = true;
  }

  if (!device.connected()) // Check the status of server connection
  {
    Serial.println("Disconnected from Losant");
    toReconnect = true;
  }

  if (toReconnect)      //Re-connect if disconnected from server or WIFI
  {
    con_status = 0;
    while (con_status != 1)
    {
      con_status = connect_server();
    }
  }
  device.loop(); // Keep Losant and WiFi connection alive

  sendDataToCloud(LaserRead); // push data to server

  // Deep sleep mode for 10 seconds, the ESP8266 wakes up by itself when GPIO 16 (D0 in NodeMCU board) is connected to the RESET pin
  Serial.println("Going to sleep...");
  esp_deep_sleep_start();
  //delay(1000);
}
