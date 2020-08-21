#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads;
  
const float MULTIPLIER = 0.00269; //obtenido por comparacion con multimetro
 
const float VOLTAJE = 230;
 
void setup()
{
  Serial.begin(9600);
 
  ads.setGain(GAIN_TWO);        // ±2.048V  1 bit = 0.0625mV
  ads.begin();
}
 
void loop()
{
 float currentRMS = getCorriente();
 float power = VOLTAJE * currentRMS;
 
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