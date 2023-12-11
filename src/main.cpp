#include <ArduinoJson.h>
#include <LoRa.h>
#include <Adafruit_AHTX0.h>

// Create instances of sensors
Adafruit_AHTX0 aht;
Adafruit_Sensor *aht_humidity, *aht_temp;

// Pin definitions
#define LED 26
#define ss 5
#define rst 17
#define dio0 12

// Define End devices
String enddeviceslist[] = {"Node1", "Node2", "Node3", "Node4", "Node5"};
const int enddevices_num = sizeof(enddeviceslist) / sizeof(enddeviceslist[0]);
int node = 0;

// LoRa configuration
RTC_DATA_ATTR int SyncWord;
RTC_DATA_ATTR int TxPower;
RTC_DATA_ATTR long freq;
RTC_DATA_ATTR double interval;
RTC_DATA_ATTR int spreadingFactor;
RTC_DATA_ATTR long signalBandwidth;

// LoRa default configuration
int defaultSyncWord = 0xF1;
int defaultTxPower = 20;
long defaultfreq = 923E6;
double defaultinterval = 0.1;
int defaultSpreadingFactor = 9;
long defaultSignalBandwidth = 125E3;
int indexs = 0;

#define NodeName "Node1"

// LoRa recv configuration
#define timeout 15000

String createJsonString(String Nodename, float tempfl, float humifl)
{
  Serial.println("\n----------   Start of createJsonString()   ----------\n");
  StaticJsonDocument<512> doc;

  // Generate a random packet ID
  int randomPacketID = random(99999, 1000000);
  doc["NodeName"] = Nodename;
  doc["PacketID"] = randomPacketID;
  doc["Temperature"] = round(tempfl * 100.00) / 100.00;
  doc["Humidity"] = round(humifl * 100.00) / 100.00;

  String jsonString;
  serializeJson(doc, jsonString);
  Serial.println("\n----------   End of createJsonString()   ----------\n");
  return jsonString;
}

void sleep(float sec)
{
  Serial.println("\n----------   Start of sleep()   ----------\n");
  double min_d = sec / 60;

  // Set wakeup time
  esp_sleep_enable_timer_wakeup((interval - min_d) * 60 * 1000000);

  Serial.print("Duration: ");
  Serial.print(sec / 60);
  Serial.println(" minutes");

  Serial.print("Going to sleep for ");
  Serial.print((interval - min_d));
  Serial.println(" minutes");
  Serial.println("\n----------   End of sleep()   ----------\n");
  esp_deep_sleep_start();
}

void blinkLED(int numBlinks, int blinkDuration = 500)
{
  Serial.println("\n----------   Start of blinkLED()   ----------\n");
  Serial.println("LED blinking...");
  for (int i = 0; i < numBlinks; i++)
  {
    digitalWrite(LED, HIGH);
    delay(blinkDuration);
    digitalWrite(LED, LOW);
    delay(blinkDuration);
  }
  Serial.println("\n----------   End of blinkLED()   ----------\n");
}

void processJsonInput(const char *jsonInput)
{
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, jsonInput);
  if (error)
  {
    Serial.print("Parsing failed: ");
    Serial.println(error.c_str());
    return;
  }

  if (doc["SyncWord"] != SyncWord)
  {
    SyncWord = doc["SyncWord"];
    Serial.print("SyncWord changed");
  }
  if (doc["TxPower"] != TxPower)
  {
    TxPower = doc["TxPower"];
    Serial.print("TxPower changed");
  }
  if (doc["freq"] != freq)
  {
    freq = doc["freq"];
    Serial.print("TxPower changed");
  }
  if (doc["interval"] != interval)
  {
    interval = doc["interval"];
    Serial.print("interval changed");
  }
}

void setup()
{
  Serial.begin(115200);
  setCpuFrequencyMhz(80);
  pinMode(LED, OUTPUT);
  unsigned long startTime = millis();

  // Initialize AHT sensor
  while (!aht.begin())
  {
    Serial.println("Failed to find AHT10/AHT20 chip");
    delay(10);
  }
  Serial.println("AHT10/AHT20 Initialized!");
  delay(200);

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
    Serial.println("No value use default");
  }

  LoRa.setPins(ss, 4, 34);
  while (!LoRa.begin(freq))
  {
    Serial.println("Waiting for LoRa module...");
    delay(500);
  }

  LoRa.setTxPower(TxPower);
  LoRa.setSyncWord(SyncWord);
  LoRa.setSpreadingFactor(spreadingFactor);
  LoRa.setSignalBandwidth(signalBandwidth);
  LoRa.enableCrc();

  // Initialize LoRa module
  Serial.print("SyncWord: ");
  Serial.println(SyncWord, HEX);
  Serial.print("TxPower: ");
  Serial.println(TxPower);
  Serial.print("Frequency: ");
  Serial.println(freq);
  Serial.print("Interval: ");
  Serial.println(interval);
  Serial.print("SpreadingFactor: ");
  Serial.println(spreadingFactor);
  Serial.print("SignalBandwidth: ");
  Serial.println(signalBandwidth);

  Serial.println("LoRa Initialized!");

  for (int i = 0; i < enddevices_num; i++)
  {
    delay(1000);

    // Get sensor readings
    sensors_event_t humidity;
    sensors_event_t temp;
    aht_humidity->getEvent(&humidity);
    aht_temp->getEvent(&temp);
    delay(100);

    // Create JSON string from sensor readings
    String jsonOutput = createJsonString(enddeviceslist[i], temp.temperature, humidity.relative_humidity);
    Serial.print("Packet send: ");
    String data = jsonOutput;
    Serial.println(data);

    // Send JSON data via LoRa
    blinkLED(3, 300);
    LoRa.beginPacket();
    LoRa.print(data);
    LoRa.endPacket();
    Serial.print(millis());
  }
}

void loop()
{

  unsigned long startTime = millis();
  Serial.println("Switching to receiving state...");
  LoRa.setSyncWord(0XF2);

  // Enter receiving state
  unsigned long recvstartTime = millis();
  while (millis() - recvstartTime < timeout)
  {

    int packetSize = LoRa.parsePacket();
    if (packetSize)
    {
      Serial.print("Received packet '");

      char LoRaData[255];
      int dataIndex = 0;

      while (LoRa.available())
      {
        char receivedChar = LoRa.read();
        Serial.print(receivedChar);

        LoRaData[dataIndex] = receivedChar;
        dataIndex++;

        if (dataIndex >= sizeof(LoRaData) - 1)
        {
          LoRaData[dataIndex] = '\0';
          break;
        }

        if (dataIndex == 1 && receivedChar != '{')
        {
          dataIndex = 0;
          break;
        }
      }

      Serial.print("' with RSSI ");
      Serial.println(LoRa.packetRssi());
      blinkLED(5, 100);

      if (dataIndex > 0)
      {
        LoRaData[dataIndex] = '\0';
        processJsonInput(LoRaData);
        delay(3000);
      }
    }
  }

  unsigned long endTime = millis();
  unsigned long duration = endTime - startTime;
  float durationSeconds = duration / 1000.0;

  sleep(durationSeconds);
  indexs = indexs + 1;
}