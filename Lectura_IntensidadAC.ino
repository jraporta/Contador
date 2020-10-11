#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <WiFi101.h>
#include <ThingSpeak.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi101OTA.h>
#include "arduino_secrets.h"

char ssid[] = SECRET_SSID; // your network SSID (name)
char pass[] = SECRET_PASS; // your network password

unsigned long myChannelNumber = SECRET_CH_ID;
const char *myWriteAPIKey = SECRET_WRITE_APIKEY;

int status = WL_IDLE_STATUS;
WiFiClient net; //WiFiSSLClient no funciona por motivos desconocidos

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

Adafruit_ADS1115 ads;

const float MULTIPLIER = 0.00269; //obtenido por comparacion con multimetro

const float VOLTAJE = 230;
const float costeEnergia = 0.14; // €/kWh
const int pinReset = 10;

float energy = 0;
long tiempo0 = 0, tiempo1 = 0;

void setup()
{
  lcd.begin(16, 2);
  lcd.home();
  lcd.print("Initializing...");
  delay(2000);
  Serial.begin(9600);

  Serial.println("firmware: amperímetro_inteligente 0.0.2");

  connectWifi();

  ads.setGain(GAIN_TWO); // ±2.048V  1 bit = 0.0625mV
  ads.begin();

  ThingSpeak.begin(net); // Initialize ThingSpeak

  pinMode(pinReset, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinReset), ResetEnergy, RISING);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Power:");
  lcd.setCursor(0, 1);
  lcd.print("Cost:");

  WiFiOTA.begin(BOARDS_NAME, PASSWORD, InternalStorage);
}

void loop()
{
  WiFiOTA.poll();

  float currentRMS = getCorriente();
  float power = VOLTAJE * currentRMS;
  long tiempo2 = millis();
  energy = energy + power * (tiempo2 - tiempo1) / 3600E6;
  float coste = energy * costeEnergia;
  tiempo1 = tiempo2;

  printMeasure("Irms: ", currentRMS, "A");
  printMeasure("Potencia: ", power, "W");
  printMeasure("Energía: ", energy, "kWh");
  printMeasure("Consumo acumulado: ", coste, "€");

  lcd.setCursor(7, 0);
  lcd.print(power);
  lcd.print(" W");
  lcd.setCursor(6, 1);
  lcd.print(coste);
  lcd.print(" Eu.");

  status = WiFi.status();
  if (status != WL_CONNECTED)
    connectWifi();

  if (status == WL_CONNECTED && millis() - tiempo0 > 14000)
  {
    // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
    // pieces of information in a channel.  Here, we write to field 1.
    ThingSpeak.setField(1, power);
    ThingSpeak.setField(2, currentRMS);
    ThingSpeak.setField(3, energy);
    ThingSpeak.setField(4, coste);
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (x == 200)
    {
      Serial.println("Channel update successful.");
    }
    else
    {
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    tiempo0 = millis();
  }
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
  return (corriente);
}

void printMeasure(String prefix, float value, String postfix)
{
  Serial.print(prefix);
  Serial.print(value, 3);
  Serial.println(postfix);
}

void connectWifi()
{
  Serial.println("checking wifi...");
  status = WiFi.status();
  if (status != WL_CONNECTED)
  {
    Serial.print("Connecting to SSID: ");
    Serial.print(ssid);
    Serial.print("...");
    status = WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network
    delay(1000);
  }
  if (status != WL_CONNECTED)
  {
    Serial.println("error");
  }
  else
  {
    Serial.println("done!!!");
    printWiFiStatus();
  }
}

void printWiFiStatus()
{
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP: ");
  Serial.println(ip);

  Serial.print("signal strength (RSSI):");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}

void ResetEnergy()
{
  energy = 0;
}
