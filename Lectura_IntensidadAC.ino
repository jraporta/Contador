#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <WiFi101.h>
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;     // your network SSID (name)
char pass[] = SECRET_PASS;     // your network password

int status = WL_IDLE_STATUS;
WiFiSSLClient net;

Adafruit_ADS1115 ads;

const float MULTIPLIER = 0.00269; //obtenido por comparacion con multimetro

const float VOLTAJE = 230;

void setup()
{
delay(2000);
Serial.begin(9600);

Serial.println("firmware: amperímetro_inteligente 0.0.2");

ads.setGain(GAIN_TWO);        // ±2.048V  1 bit = 0.0625mV
ads.begin();

connectWifi();
}

void loop()
{

float currentRMS = getCorriente();
float power = VOLTAJE * currentRMS;

status = WiFi.status();
if ( status != WL_CONNECTED) connectWifi();

printMeasure("Irms: ", currentRMS, "A ,");
printMeasure("Potencia: ", power, "W");
delay(1000);
}

float getCorriente()
{
float corriente;
float sum = 0;
long tiempo = millis();
int counter = 0;

while (millis() - tiempo < 1000)
{
corriente = ads.readADC_Differential_0_1() * MULTIPLIER;
sum += sq(corriente);
counter = counter + 1;
}

corriente = sqrt(sum / counter);
return(corriente);
}

void printMeasure(String prefix, float value, String postfix)
{
Serial.print(prefix);
Serial.print(value, 3);
Serial.println(postfix);
}

void connectWifi() {
Serial.println("checking wifi...");
status = WiFi.status();
if ( status != WL_CONNECTED) {
Serial.print("Connecting to SSID: ");
Serial.print(ssid);
Serial.print("...");
status = WiFi.begin(ssid, pass);      // Connect to WPA/WPA2 network
delay(1000);
}
if ( status != WL_CONNECTED) {
Serial.println("error");
} else {
Serial.println("done!!!");
printWiFiStatus();
}
}

void printWiFiStatus() {
Serial.print("SSID: ");
Serial.println(WiFi.SSID());

IPAddress ip = WiFi.localIP();
Serial.print("IP: ");
Serial.println(ip);

Serial.print("signal strength (RSSI):");
Serial.print(WiFi.RSSI());
Serial.println(" dBm");
}