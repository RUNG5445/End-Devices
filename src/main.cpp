#include <lorawan.h>
#include <Adafruit_AHTX0.h>
#include <CayenneLPP.h>

Adafruit_AHTX0 aht;
Adafruit_Sensor *aht_humidity, *aht_temp;
CayenneLPP lpp(51);

// ABP Credentials
const char *devAddr = "00f89395";
const char *nwkSKey = "40256ba8fb26b3c30ae68f41830f4c50";
const char *appSKey = "b5cde784a212640ba8ee271c59d298e5";

const unsigned long interval = 20000; // 10 s interval to send message
unsigned long previousMillis = 0;     // will store last time message sent
unsigned int counter = 0;             // message counter

char myStr[50];
char outStr[255];
byte recvStatus = 0;

const sRFM_pins RFM_pins = {
    .CS = 5,
    .RST = 4,
    .DIO0 = 34,
    .DIO1 = 35,
    .DIO2 = -1,
    .DIO5 = -1,
};

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

void setup()
{
  // Setup loraid access
  Serial.begin(115200);
  delay(5000);
  Serial.println("Start..");
  if (!lora.init())
  {
    Serial.println("RFM95 not detected");
    delay(5000);
    return;
  }

  // Set LoRaWAN Class change CLASS_A or CLASS_C
  lora.setDeviceClass(CLASS_C);

  // Set Data Rate
  lora.setDataRate(SF7BW125);

  // set channel to random
  lora.setChannel(1);

  // Put ABP Key and DevAddress here
  lora.setNwkSKey(nwkSKey);
  lora.setAppSKey(appSKey);
  lora.setDevAddr(devAddr);
}

void loop()
{

  // Check interval overflow
  if (millis() - previousMillis > interval)
  {
    previousMillis = millis();

    while (!aht.begin())
    {
      Serial.println("Failed to find AHT10/AHT20 chip");
      delay(10);
    }
    Serial.println("AHT10/AHT20 Initialized!");
    delay(200);

    aht_temp = aht.getTemperatureSensor();
    aht_humidity = aht.getHumiditySensor();
    // Get sensor readings
    sensors_event_t humidity;
    sensors_event_t temp;
    aht_humidity->getEvent(&humidity);
    aht_temp->getEvent(&temp);
    lpp.reset();
    lpp.addTemperature(2, temp.temperature);
    lpp.addRelativeHumidity(3, humidity.relative_humidity);
    String jsonOutput = createJsonString("Node", temp.temperature, humidity.relative_humidity);
    // Get the length of the String
    int length = jsonOutput.length();

    // Create a char array with enough space for the String and a null terminator
    char charArray[length + 1];

    // Copy the contents of the String to the char array
    jsonOutput.toCharArray(charArray, length + 1);
    Serial.print("Sending: ");
    Serial.println(jsonOutput);

    lora.sendUplink(charArray, length, 0, 1);
    counter++;
  }

  recvStatus = lora.readData(outStr);
  if (recvStatus)
  {
    Serial.print("====>> ");
    Serial.println(outStr);
  }

  // Check Lora RX
  lora.update();
}