# MCP23S17_Mqtt
An arduino sketch for controlling an MCP23S17 GPIO expander using JSON messages with MQTT.

The idea of this sketch is to provide a simple way of controlling many pins on arduino remotely using simple Json commands.

# This use the following libraries: 

Majenko MCP23S17: https://github.com/MajenkoLibraries/MCP23S17
aJson: https://github.com/interactive-matter/aJson
PubSubClient: https://github.com/knolleary/pubsubclient

# Commands

Currently there are only two commands implemented, ON and OFF. You can send the pin and pin state in a JSON message as follows. I am using two MCP23S17s and so to make things simpler I can address the pin 0-31 with 0-15 being on bank1 and 16-31 bank2.

# Turn pin 4 ON

{
   "command": {
        "lightmode":       "ON", 
        "pinnumber":     4 
    }
}

# Turn pin 4 OFF

{
   "command": {
        "lightmode":       "OFF", 
        "pinnumber":     4 
    }
}
