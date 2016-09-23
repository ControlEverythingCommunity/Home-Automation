// Distributed with a free-will license.
// Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
// Control Lights with ESP8266
// This code is designed to work with the MPL115A2_I2CS I2C Mini Module available from ControlEverything.com.
// https://www.controleverything.com/content/Barometer?sku=MPL115A2_I2CS#tabs-0-product_tabset-2
// https://www.controleverything.com/content/Relay-Controller?sku=MCP23008_I2CR8G5LE_10A#tabs-0-product_tabset-2

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Wire.h>

// MPL115A2 I2C address is 0x60(96)
#define Addr_Sensor 0x60
// MCP23008_I2CR8G5LE I2C address is 0x20(32)
#define Addr_Relay 0x20

const char* ssid = "your ssid network";
const char* password = "your password";

unsigned int data[8];
float a1 = 0.0, b1 = 0.0, b2 = 0.0, c12 = 0.0;

ESP8266WebServer server(80);

void handleroot()
{
  // Start I2C Transmission
  Wire.beginTransmission(Addr_Relay);
  // Select IODIR register
  Wire.write(0x00);
  // All pins are configured as output
  Wire.write(0x00);
  // Stop I2C transmission
  Wire.endTransmission();
  delay(500);

  for (int i = 0; i < 8; i++)
  {
    // Start I2C Transmission
    Wire.beginTransmission(Addr_Sensor);
    // Select data register
    Wire.write(4 + i);
    // Stop I2C Transmission
    Wire.endTransmission();

    // Request 1 byte of data
    Wire.requestFrom(Addr_Sensor, 1);

    // Read 1 byte of data
    // A10 msb, A10 lsb, Bb1 msb, Bb1 lsb, B2 msb, B2 lsb, C12 msb, C12 lsb
    if (Wire.available() == 1)
    {
      data[i] = Wire.read();
    }
  }

  // Convert the data to floating points
  a1 = ((data[0] * 256.0) + data[1]) / 8.0;
  b1 = ((data[2] * 256) + data[3]);
  if (b1 > 32767)
  {
    b1 -= 65536;
  }
  b1 = b1 / 8192.0;
  b2 = ((data[4] * 256) + data[5]);
  if (b2 > 32767)
  {
    b2 -= 65536;
  }
  b2 = b2 / 16384.0;
  c12 = ((data[6] * 256.0 + data[7]) / 4.0) / 4194304.0;
  delay(300);

  // Start I2C Transmission
  Wire.beginTransmission(Addr_Sensor);
  // Send Pressure measurement command
  Wire.write(0x12);
  // Start conversion
  Wire.write(0x00);
  // Stop I2C Transmission
  Wire.endTransmission();
  delay(300);

  // Start I2C Transmission
  Wire.beginTransmission(Addr_Sensor);
  // Select data register
  Wire.write(0x00);
  // Stop I2C Transmission
  Wire.endTransmission();

  // Request 4 bytes of data
  Wire.requestFrom(Addr_Sensor, 4);

  // Read 4 bytes of data
  // pres msb, pres lsb, temp msb, temp lsb
  if (Wire.available() == 4)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
    data[2] = Wire.read();
    data[3] = Wire.read();
  }

  // Convert the data to 10-bits
  int pres = (data[0] * 256 + (data[1] & 0xC0)) / 64;
  int temp = (data[2] * 256 + (data[3] & 0xC0)) / 64;

  // Calculate pressure compensation
  float presComp = a1 + (b1 + c12 * temp) * pres + b2 * temp;

  // Convert the data
  float pressure = (65.0 / 1023.0) * presComp + 50.0;
  float cTemp = (temp - 498) / (-5.35) + 25.0;
  float fTemp = cTemp * 1.8 + 32.0;

  // Output data to serial monitor
  Serial.print("Pressure : ");
  Serial.print(pressure);
  Serial.println(" kPa");
  Serial.print("Temperature in Celsius : ");
  Serial.print(cTemp);
  Serial.println(" C");
  Serial.print("Temperature in Fahrenheit : ");
  Serial.print(fTemp);
  Serial.println(" F");
  delay(500);

  // Output data to Web Server
  server.sendContent
  ("<html><head><meta http-equiv='refresh' content='2'</meta>"
   "<h1 style=text-align:center;font-size:300%;color:blue;font-family:britannic bold;>CONTROL EVERYTHING</h1>"
   "<h3 style=text-align:center;font-family:courier new;><a href=http://www.controleverything.com/ target=_blank>www.controleverything.com</a></h3><hr>"
   "<h2 style=text-align:center;font-family:tahoma;><a href=https://www.controleverything.com/content/Barometer?sku=MPL115A2_I2CS#tabs-0-product_tabset-2 \n"
   "target=_blank>MPL115A2 Sensor I2C Mini Module</a></h2>");
  server.sendContent
  ("<h3 style=text-align:center;font-family:tahoma;>Pressure : " + String(pressure) + " kPa");
  server.sendContent
  ("<h3 style=text-align:center;font-family:tahoma;>Temperature in Celsius = " + String(cTemp) + " C");
  server.sendContent
  ("<h3 style=text-align:center;font-family:tahoma;>Temperature in Fahrenheit = " + String(fTemp) + " F");

  if (pressure > 136)
  {
    // Start I2C Transmission
    Wire.beginTransmission(Addr_Relay);
    // Select GPIO register
    Wire.write(0x09);
    // Realy-1 set to logic High
    Wire.write(0x01);
    // Stop I2C transmission
    Wire.endTransmission();

    // Output data to serial monitor
    Serial.println("Side Lamp : ON");

    // Output data to Web Server
    server.sendContent
    ("<h3 style=text-align:center;font-family:tahoma;> Side lamp : ON");
  }
  else
  {
    // Start I2C Transmission
    Wire.beginTransmission(Addr_Relay);
    // Select GPIO register
    Wire.write(0x09);
    // Relay-1 set to logic Low
    Wire.write(0x00);
    // Stop I2C transmission
    Wire.endTransmission();

    // Output data to serial monitor
    Serial.println("Side Lamp : OFF");

    // Output data to Web Server
    server.sendContent
    ("<h3 style=text-align:center;font-family:tahoma;> Side Lamp : OFF");
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

