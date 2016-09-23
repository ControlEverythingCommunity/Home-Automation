// Distributed with a free-will license.
// Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
// Cloth Washer/Dryer Monitoring with ESP8266
// This code is designed to work with the H3LIS331DL_I2CS I2C Mini Module available from ControlEverything.com.
// https://www.controleverything.com/content/Accelorometer?sku=H3LIS331DL_I2CS#tabs-0-product_tabset-2

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Wire.h>

// H3LIS331DL I2C address is 0x18(24)
#define Addr 0x18

const char* ssid = "your ssid network";
const char* password = "your password";

ESP8266WebServer server(80);

void handleroot()
{
  unsigned int data[6];

  for (int i = 0; i < 6; i++)
  {
    // Start I2C Transmission
    Wire.beginTransmission(Addr);
    // Select data register
    Wire.write((40 + i));
    // Stop I2C Transmission
    Wire.endTransmission();

    // Request 1 byte of data
    Wire.requestFrom(Addr, 1);
    // Read 6 bytes of data
    // xAccl lsb, xAccl msb, yAccl lsb, yAccl msb, zAccl lsb, zAccl msb
    if (Wire.available() == 1)
    {
      data[i] = Wire.read();
    }
  }
  delay(300);

  // Convert the data
  int xAccl = ((data[1] * 256) + data[0]);
  if (xAccl > 32767)
  {
    xAccl -= 65536;
  }
  int xAcc = ((100 * 9.8) / 32768) * xAccl;

  int yAccl = ((data[3] * 256) + data[2]);
  if (yAccl > 32767)
  {
    yAccl -= 65536;
  }
  int yAcc = ((100 * 9.8) / 32768) * yAccl;

  int zAccl = ((data[5] * 256) + data[4]);
  if (zAccl > 32767)
  {
    zAccl -= 65536;
  }
  int zAcc = ((100 * 9.8) / 32768) * zAccl;

  // Output data to serial monitor
  Serial.print("Acceleration in X-Axis : ");
  Serial.print(xAcc);
  Serial.println(" m/s");
  Serial.print("Acceleration in Y-Axis : ");
  Serial.print(yAcc);
  Serial.println(" m/s");
  Serial.print("Acceleration in Z-Axis : ");
  Serial.print(zAcc);
  Serial.println(" m/s");
  delay(300);

  // Output data to Web Server
  server.sendContent
  ("<html><head><meta http-equiv='refresh' content='10'</meta>"
   "<h1 style=text-align:center;font-size:300%;color:blue;font-family:britannic bold;>CONTROL EVERYTHING</h1>"
   "<h3 style=text-align:center;font-family:courier new;><a href=http://www.controleverything.com/ target=_blank>www.controleverything.com</a></h3><hr>"
   "<h2 style=text-align:center;font-family:tahoma;><a href=https://www.controleverything.com/content/Accelorometer?sku=H3LIS331DL_I2CS#tabs-0-product_tabset-2 \n"
   "target=_blank>H3LIS331DL Sensor I2C Mini Module</a></h2>");
  server.sendContent
  ("<h3 style=text-align:center;font-family:tahoma;>Acceleration in X-Axis = " + String(xAcc) + " m/s/s");
  server.sendContent
  ("<h3 style=text-align:center;font-family:tahoma;>Acceleration in Y-Axis = " + String(yAcc) + " m/s/s");
  server.sendContent
  ("<h3 style=text-align:center;font-family:tahoma;>Acceleration in Z-Axis = " + String(zAcc) + " m/s/s");

  if (xAcc > 2)
  {
    // Output data to serial monitor
    Serial.println("Cloths Washer/Dryer : Working");

    // Output data to Web Server
    server.sendContent
    ("<h3 style=text-align:center;font-family:tahoma;> Cloths Washer/Dryer : Working");
  }
  else
  {
    // Output data to serial monitor
    Serial.println("Cloths Washer/Dryer : Completed");

    // Output data to Web Server
    server.sendContent
    ("<h3 style=text-align:center;font-family:tahoma;> Cloths Washer/Dryer : Completed");
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

  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select control register 1
  Wire.write(0x20);
  // Enable X, Y, Z axis, power on mode, data output rate 50Hz
  Wire.write(0x27);
  // Stop I2C Transmission
  Wire.endTransmission();

  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select control register 4
  Wire.write(0x23);
  // Set full scale, +/- 100g, continuous update
  Wire.write(0x00);
  // Stop I2C Transmission
  Wire.endTransmission();
  delay(300);
}

void loop()
{
  server.handleClient();
}

