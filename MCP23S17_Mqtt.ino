#include <MCP23S17.h>
#include <aJSON.h>
#include <SPI.h>
#include <PubSubClient.h>
#include <Ethernet.h>

/*
 * Relay control over mqtt
 */

#define MQTT_SERVER "mqtt_server"
#define MQTT_pubTopic "arduino/output"
#define MQTT_subTopic "arduino/input"


// MAC Address of Arduino Ethernet Sheild (on sticker on shield)
byte MAC_ADDRESS[] = { 0x90, 0xA2, 0xDA, 0x0D, 0x31, 0xB8 };
EthernetClient ethClient;
PubSubClient client(MQTT_SERVER, 1883, callback, ethClient);
byte ip[]     = { 192, 168, 1, 23 };

// MCP23S17 definition
const uint8_t chipSelect = 7;

// Create an object for each chip
// Bank 0 is address 0
// Bank 1 is address 1.
// Increase the addresses by 2 for each BA value.

MCP23S17 Bank1(&SPI, chipSelect, 0);
MCP23S17 Bank2(&SPI, chipSelect, 1);


// defines and variable for sensor/control mode
#define MODE_OFF    0  // not sensing light, LED off
#define MODE_ON     1  // not sensing light, LED on

int senseMode = 0;
int pinNumber = 0;
unsigned long time;
char message_buff[100];

void setup()
{
  // MCP23S17 initialisation
  Bank1.begin();
  Bank2.begin();
  for (int i = 0; i < 16; i++) {
    Bank1.pinMode(i, OUTPUT);
    Bank2.pinMode(i, OUTPUT);
  }
  
  // init serial link for debugging
  Serial.begin(9600);
  if (Ethernet.begin(MAC_ADDRESS) == 0)
  {
    Serial.println("Failed to configure Ethernet using DHCP");
    Serial.println("Using static IP to connect...");
    Ethernet.begin(MAC_ADDRESS, ip);
    Serial.println("Ethernet connected.");
  }
  else
  {
    Serial.println("Connected to Ethernet using DHCP");
  }
}

void loop()
{
  if (!client.connected())
  {
    // clientID, username, MD5 encoded password
    Serial.println("Connecting to mqtt server...");
    client.connect("arduino-mqtt", "arduino", "pa$5w0rd");
    client.publish(MQTT_pubTopic, "I'm alive!");
    client.subscribe(MQTT_subTopic);
  }

  if (!client.connected())
  {
    Serial.println("Failed to connect to mqtt server.");
  }
  
  switch (senseMode) {
    case MODE_OFF:
      // light should be off
      if (pinNumber < 16) {
        Bank1.digitalWrite(pinNumber, false);
      }
      else {
        Bank1.digitalWrite(pinNumber - 16, false);
      }      
      break;
    case MODE_ON:
      // light should be on
      if (pinNumber < 16) {
        Bank1.digitalWrite(pinNumber, true);
      }
      else {
        Bank1.digitalWrite(pinNumber - 16, true);
      }
      break;
  }

  // MQTT client loop processing
  client.loop();
}

// handles message arrived on subscribed topic(s)
void callback(char* topic, byte* payload, unsigned int length) {

  int i = 0;

  Serial.println("Message arrived:  topic: " + String(topic));
  Serial.println("Length: " + String(length, DEC));

  // create character buffer with ending null terminator (string)
  for (i = 0; i < length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';

  String msgString = String(message_buff);

  Serial.println("Payload: " + msgString);
  aJsonObject* root = aJson.parse(message_buff);

  if (root != NULL) {
    aJsonObject* command = aJson.getObjectItem(root, "command");
    
    if (command != NULL) {
      // Deserialise json
      aJsonObject* lightmode = aJson.getObjectItem(command, "lightmode");
      aJsonObject* pinnumber = aJson.getObjectItem(command, "pinnumber");
    
      String lightMode = lightmode->valuestring;
      pinNumber = pinnumber->valueint;
      Serial.println(lightMode);
      Serial.println(pinNumber);
    
      if (lightMode.equals("OFF")) {
        senseMode = MODE_OFF;
      } else if (lightMode.equals("ON")) {
        senseMode = MODE_ON;
      }
      //"{\"command\":{\"lightmode\": \"ON\", \"pinnumber\": 1}}"
    }
    // Cleanup ready for the next command
    aJson.deleteItem(root);
  }
}
