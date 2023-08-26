#include <Wire.h>
#include <SPI.h>
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
#define dio0 16

// LoRa configuration
int SyncWord = 241;
int TxPower = 20;
long freq = 923E6;
double interval = 1;
#define NodeName "Node1"
#define timeout 10000

// Function to create a JSON string
String createJsonString(float tempfl, float humifl)
{
  Serial.println("\n----------   Start of createJsonString()   ----------\n");
  StaticJsonDocument<512> doc;

  // Generate a random packet ID
  int randomPacketID = random(99999, 1000000); // Generates a number between 99999 and 999999
  doc["NodeName"] = NodeName;
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

  // Print the duration in minutes to the serial monitor
  Serial.print("Duration: ");
  Serial.print(sec / 60);
  Serial.println(" minutes");

  // Go to sleep now
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

void setup()
{
  Serial.begin(115200);
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

  // Initialize LoRa module
  LoRa.setPins(ss, rst, dio0);
  LoRa.setSyncWord(SyncWord);
  LoRa.setTxPower(TxPower);
  while (!LoRa.begin(freq))
  {
    Serial.println("Waiting for LoRa module...");
    delay(500);
  }
  Serial.println("LoRa Initialized!");

  // Get sensor readings
  sensors_event_t humidity;
  sensors_event_t temp;
  aht_humidity->getEvent(&humidity);
  aht_temp->getEvent(&temp);
  delay(100);

  // Create JSON string from sensor readings
  String jsonOutput = createJsonString(temp.temperature, humidity.relative_humidity);
  Serial.println("Packet send: ");
  Serial.print(jsonOutput);

  // Send JSON data via LoRa
  blinkLED(3, 300);
  LoRa.beginPacket();
  LoRa.print(jsonOutput);
  LoRa.endPacket();
  delay(1000);

  Serial.println("Switching to receiving state...");

  // Enter receiving state
  unsigned long recvstartTime = millis();
  while (millis() - recvstartTime < timeout)
  {
    int packetSize = LoRa.parsePacket();
    if (packetSize)
    {
      Serial.println("Packet received:");
      while (LoRa.available())
      {
        Serial.write(LoRa.read());
      }
      break;
    }
  }

  // Put the ESP into deep sleep for a calculated duration
  unsigned long endTime = millis();
  unsigned long duration = endTime - startTime;
  float durationSeconds = duration / 1000.0;

  // Put the device to sleep for the calculated duration
  sleep(durationSeconds);
}

void loop()
{
  delay(100);
}
