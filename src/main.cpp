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
#define NodeName "Node1"
#define timeout 10000

// Function to create a JSON string
String createJsonString(float tempfl, float humifl)
{
  StaticJsonDocument<512> doc;

  // Generate a random packet ID
  int randomPacketID = random(99999, 1000000); // Generates a number between 99999 and 999999
  doc["NodeName"] = NodeName;
  doc["PacketID"] = randomPacketID;
  doc["Temperature"] = round(tempfl * 100.00) / 100.00;
  doc["Humidity"] = round(humifl * 100.00) / 100.00;

  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
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

void setup()
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);

  // Initialize AHT sensor
  Serial.println("Adafruit AHT10/AHT20 test!");
  while (!aht.begin())
  {
    Serial.println("Failed to find AHT10/AHT20 chip");
    delay(10);
  }
  Serial.println("AHT10/AHT20 Found!");
  aht_temp = aht.getTemperatureSensor();
  aht_humidity = aht.getHumiditySensor();

  // Initialize LoRa module
  LoRa.setPins(ss, rst, dio0);
  LoRa.setSyncWord(0xF1);
  LoRa.setTxPower(20);
  while (!LoRa.begin(923E6))
  {
    Serial.println("Waiting for LoRa module...");
    delay(500);
  }
  Serial.println("LoRa Initializing OK!");
}

void loop()
{
  // Get sensor readings
  sensors_event_t humidity;
  sensors_event_t temp;
  aht_humidity->getEvent(&humidity);
  aht_temp->getEvent(&temp);
  delay(100);

  // Create JSON string from sensor readings
  String jsonOutput = createJsonString(temp.temperature, humidity.relative_humidity);
  Serial.println("Packet send : ");
  Serial.println(jsonOutput);

  // Send JSON data via LoRa
  blinkLED(3, 300);
  LoRa.beginPacket();
  LoRa.print(jsonOutput);
  LoRa.endPacket();
  delay(1000);

  Serial.println("Switching to receiving state...");

  // Enter receiving state for 10 seconds
  long startTime = millis();
  while (millis() - startTime < 10000)
  {
    int packetSize = LoRa.parsePacket();
    if (packetSize)
    {
      Serial.println("Packet received:");
      while (LoRa.available())
      {
        Serial.write(LoRa.read());
      }
      break; // Exit the loop after printing the received packet
    }
  }

  Serial.println("Sleeping...");
  // Put the ESP into deep sleep for 5 seconds
  ESP.deepSleep(5000000);
}
