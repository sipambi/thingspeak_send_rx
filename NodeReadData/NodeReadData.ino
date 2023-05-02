//-----------Electronics-project-hub-------------//
#include "ThingSpeak.h"
#include <ESP8266WiFi.h>
const char ssid[] = "xxxxxxxxx";  // your network SSID (name)
const char pass[] = "xxxxxxxxx";   // your network password
int statusCode = 0;
const int relay = 4;
WiFiClient  client;

//---------Channel Details---------//
unsigned long counterChannelNumber = 2113747;            // Channel ID
const char * myCounterReadAPIKey = "0FUOQPBPCXS62LFS"; // Read API Key
const int FieldNumber1 = 8;
//-------------------------------//

void setup()
{
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
  Serial.begin(9600);
  delay(1000);
}

void loop()
{
  //----------------- Network -----------------//
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connecting....");
    delay(1000);
    while (WiFi.status() != WL_CONNECTED)
    {
      WiFi.begin(ssid, pass);
      delay(5000);
    }
    Serial.println("Connected Successfully");
    delay(1000);
  }
  //--------- End of Network connection--------//

  //---------------- Channel 1 ----------------//
  long temp = ThingSpeak.readLongField(counterChannelNumber, FieldNumber1, myCounterReadAPIKey);
  statusCode = ThingSpeak.getLastReadStatus();
  
  if (statusCode == 200)
  {
    Serial.print("Recieved Data is ");
    Serial.println(temp);

    if(temp == 1)
    {
      digitalWrite(relay, 1);            
    }    
    else
    {
      digitalWrite(relay, 0);
    }
  }
  else
  {
    Serial.println("Error in Network");
  }
  delay(100);
}
  //-------------- End of Channel 1 -------------/
