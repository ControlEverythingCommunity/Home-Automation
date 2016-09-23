// Distributed with a free-will license.
// Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
// Home Automation with ESP8266
// This code is designed to work with the TSL45315_I2CS I2C Mini Module available from ControlEverything.com.
// https://www.controleverything.com/content/Light?sku=TSL45315_I2CS#tabs-0-product_tabset-2
// https://www.controleverything.com/content/Relay-Controller?sku=MCP23008_I2CR8G5LE_10A#tabs-0-product_tabset-2

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Wire.h>

// TSL45315 I2C address is 0x29(41)
#define Addr_Sensor 0x29
// MCP23008_I2CR8G5LE I2C address is 0x20(32)
#define Addr_Relay 0x20

const char* ssid = "your ssid network";
const char* password = "your password";
int luminance;

ESP8266WebServer server(80);

void handleroot()
{
  unsigned int data[2];

  // Start I2C Transmission
  Wire.beginTransmission(Addr_Relay);
  // Select IODIR register
  Wire.write(0x00);
  // All pins are configured as output
  Wire.write(0x00);
  // Stop I2C transmission
  Wire.endTransmission();
  delay(500);

  // Start I2C Transmission
  Wire.beginTransmission(Addr_Sensor);
  // Select control register
  Wire.write(0x80);
  // Normal operation
  Wire.write(0x03);
  // Stop I2C transmission
  Wire.endTransmission();

  // Start I2C Transmission
  Wire.beginTransmission(Addr_Sensor);
  // Select configuration register
  Wire.write(0x81);
  // Multiplier 1x, Tint : 400ms
  Wire.write(0x00);
  // Stop I2C transmission
  Wire.endTransmission();
  delay(300);

  // Start I2C Transmission
  Wire.beginTransmission(Addr_Sensor);
  // Select data register
  Wire.write(0x84);
  // Stop I2C transmission
  Wire.endTransmission();

  // Request 2 bytes of data
  Wire.requestFrom(Addr_Sensor, 2);

  // Read 2 bytes of data
  // luminance lsb, luminance msb
  if (Wire.available() == 2)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
  }

  // Convert the data
  luminance = data[1] * 256 + data[0];

  // Output data to Serial Monitor
  Serial.print("Ambient Light Luminance :");
  Serial.print(luminance);
  Serial.println(" lux");
  delay(300);

  // Output data to Web Server
  server.sendContent
  ("<html><head><meta http-equiv='refresh' content='2'</meta>"
   "<h1 style=text-align:center;font-size:300%;color:blue;font-family:britannic bold;>CONTROL EVERYTHING</h1>"
   "<h3 style=text-align:center;font-family:courier new;><a href=http://www.controleverything.com/ target=_blank>www.controleverything.com</a></h3><hr>"
   "<h2 style=text-align:center;font-family:tahoma;><a href=https://www.controleverything.com/content/Light?sku=TSL45315_I2CS#tabs-0-product_tabset-2 \n"
   "target=_blank>TSL45315 Sensor I2C Mini Module</a></h2>");
  server.sendContent
  ("<h3 style=text-align:center;font-family:tahoma;>Ambient Light Luminance : " + String(luminance) + " lux");

  if (luminance < 200)
  {
    // Start I2C Transmission
    Wire.beginTransmission(Addr_Relay);
    // Select GPIO register
    Wire.write(0x09);
    // All pins are set to logic HIGH
    Wire.write(0xFF);
    // Stop I2C transmission
    Wire.endTransmission();

    // Output data to serial monitor
    Serial.print("House Lights : ON");

    // Output data to Web Server
    server.sendContent
    ("<h3 style=text-align:center;font-family:tahoma;> House Lights : ON");
  }
  else
  {
    // Start I2C Transmission
    Wire.beginTransmission(Addr_Relay);
    // Select GPIO register
    Wire.write(0x09);
    // All pins are set to logic HIGH
    Wire.write(0x00);
    // Stop I2C transmission
    Wire.endTransmission();

    // Output data to serial monitor
    Serial.print("House Lights : OFF");

    // Output data to Web Server
    server.sendContent
    ("<h3 style=text-align:center;font-family:tahoma;> House Lights : OFF");
  }
}

void setup()
{
  // Initialise I2C communication as MASTER
  Wire.begin(2, 14);
  // Initialise serial communication, set baud rate = 115200
  Serial.begin(115200);

  // Connect to WiFi network
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);

  // Get the IP address of ESP8266
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Start the server
  server.on("/", handleroot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
  server.handleClient();
}

