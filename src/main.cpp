#include <ArduinoJson.h>
#include <LoRa.h>
#include <Adafruit_AHTX0.h>
Adafruit_AHTX0 aht;
Adafruit_Sensor *aht_humidity, *aht_temp;
#define LED 26
#define ss 5
#define rst 4
#define dio0 34
#define VBAT_PIN 33
#define BATTV_MAX 4.2
#define BATTV_MIN 3.2
#define BATTV_LOW 3.4
float battpc;
String enddeviceslist[] = {"Node1"};
int waittime = 1000;
const int enddevices_num = sizeof(enddeviceslist) / sizeof(enddeviceslist[0]);
int node = 0;
RTC_DATA_ATTR int SyncWord;
RTC_DATA_ATTR int TxPower;
RTC_DATA_ATTR long freq;
RTC_DATA_ATTR double interval;
RTC_DATA_ATTR int spreadingFactor;
RTC_DATA_ATTR long signalBandwidth;
int defaultSyncWord = 0xF1;
int defaultTxPower = 20;
long defaultfreq = 923E6;                                                                                                                                                                                                                                                                                                                                                                                      
double defaultinterval = 0.1;
int defaultSpreadingFactor = 9;
long defaultSignalBandwidth = 125E3;
int indexs = 0;
#define user "user2"
#define timeout 50000
float readBatteryVoltage()
{
  float totalVoltage = 0.0;
  for (int i = 0; i < 10000; i++)
  {
    totalVoltage += ((float)analogRead(VBAT_PIN) / 4095) * 3.3 * 2 * 1.055;
  }
  float averageVoltage = totalVoltage / 10000.0;
  return averageVoltage;
}
String createJsonString(String Nodename, float tempfl, float humifl, float battpc)
{
  StaticJsonDocument<512> doc;
  int randomPacketID = random(99999, 1000000);
  doc["NodeName"] = Nodename;
  doc["User"] = user;
  doc["PacketID"] = randomPacketID;
  doc["Temperature"] = round(tempfl * 100.00) / 100.00;
  doc["Humidity"] = round(humifl * 100.00) / 100.00;
  doc["BatLvl"] = round(battpc * 100.00) / 100.00;
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}
void sleep(float sec)
{
  double min_d = sec / 60;
  esp_sleep_enable_timer_wakeup((interval - min_d) * 60 * 1000000);
  esp_deep_sleep_start();
}
void blinkLED(int numBlinks, int blinkDuration = 500)
{
  for (int i = 0; i < numBlinks; i++)
  {
    digitalWrite(LED, HIGH);
    delay(blinkDuration);
    digitalWrite(LED, LOW);
    delay(blinkDuration);
  }
}
void processJsonInput(const char *jsonInput)
{
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, jsonInput);
  if (error)
  {
    return;
  }
  if (doc["SyncWord"] != SyncWord)
  {
    SyncWord = doc["SyncWord"];
  }
  if (doc["TxPower"] != TxPower)
  {
    TxPower = doc["TxPower"];
  }
  if (doc["freq"] != freq)
  {
    freq = doc["freq"];
  }
  if (doc["interval"] != interval)
  {
    interval = doc["interval"];
  }
}
void blinkLEDInfinitely()
{
  while (true)
  {
    digitalWrite(LED, HIGH);
    delay(200);
    digitalWrite(LED, LOW);
    delay(200);
  }
}
void setup()
{
  Serial.begin(115200);
  pinMode(VBAT_PIN, INPUT);
  setCpuFrequencyMhz(80);
  pinMode(LED, OUTPUT);
  float battVoltage = readBatteryVoltage();
  if (battVoltage < BATTV_LOW)
  {
  }
  battpc = ((battVoltage - BATTV_MIN) / (BATTV_MAX - BATTV_MIN)) * 100;
  unsigned long startTime = millis();
  if (!aht.begin())
  {
    blinkLEDInfinitely();
  }
  aht_temp = aht.getTemperatureSensor();
  aht_humidity = aht.getHumiditySensor();
  if (SyncWord == 0 || TxPower == 0 || freq == 0 || interval == 0)
  {
    SyncWord = defaultSyncWord;
    TxPower = defaultTxPower;
    freq = defaultfreq;
    interval = defaultinterval;
    spreadingFactor = defaultSpreadingFactor;
    signalBandwidth = defaultSignalBandwidth;
  }
  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(freq))
  {
    blinkLEDInfinitely();
    delay(500);
  }
  delay(waittime);
  LoRa.setTxPower(TxPower);
  LoRa.setSyncWord(SyncWord);
  LoRa.setSpreadingFactor(spreadingFactor);
  LoRa.setSignalBandwidth(signalBandwidth);
  LoRa.enableCrc();
  for (int i = 0; i < enddevices_num; i++)
  {
    delay(1000);
    sensors_event_t humidity, temp;
    aht_humidity->getEvent(&humidity);
    aht_temp->getEvent(&temp);
    delay(100);
    String jsonOutput = createJsonString(enddeviceslist[i], temp.temperature, humidity.relative_humidity, battpc);
    String data = jsonOutput;
    for (int i = 0; i < 2; i++)
    {
      blinkLED(3, 300);
      LoRa.beginPacket();
      LoRa.print(data);
      LoRa.endPacket();
      delay(500);
    }
  }
}
void loop()
{
  LoRa.setSyncWord(0XF2);
  unsigned long recvstartTime = millis();
  bool packetReceived = false;
  while (millis() - recvstartTime < timeout)
  {
    int packetSize = LoRa.parsePacket();
    if (packetSize)
    {
      packetReceived = true;
      char LoRaData[255];
      int dataIndex = 0;
      while (LoRa.available())
      {
        char receivedChar = LoRa.read();
        LoRaData[dataIndex] = receivedChar;
        if (++dataIndex >= sizeof(LoRaData) - 1)
          break;
        if (dataIndex == 1 && receivedChar != '{')
        {
          dataIndex = 0;
          break;
        }
      }
      if (packetSize < 65)
      {
        if (LoRaData[0] == '{')
        {
          LoRaData[dataIndex] = '\0';
          processJsonInput(LoRaData);
          blinkLED(5, 100);
          break;
        }
      }
    }
  }
  if (!packetReceived)
  {
    ESP.restart();
  }
  float durationSeconds = (millis() - recvstartTime) / 1000.0;
  sleep(durationSeconds);
  indexs++;
}
