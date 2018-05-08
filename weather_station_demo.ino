/*
 * DIY weather station using Arduino 
 * built by alessio.torraco@esa.int
 * with the contribution of joakim.adrup@esa.int 
 *                          diana.mastracci@esa.int
 * Code written by alessio.torraco@esa.int
 * Demo version proposed at GEOSS/HACK 2018 organized by diana.mastracci@esa.int
 * https://www.earthobservations.org/me_201805_dpw.php?t=hackathon
 * 
 * MIT License 2018 Alessio Torraco
 */

//  Include libraries
#include <SoftwareSerial.h>
#include "TinyGPS++.h"
#include <SPI.h>
#include <DHT.h> //https://github.com/adafruit/DHT-sensor-library
#include <DHT_U.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h> //https://github.com/adafruit/Adafruit_BMP280_Library

//
#define DHTPIN 2    
#define DHTTYPE DHT22   
DHT dht(DHTPIN, DHTTYPE);   
Adafruit_BMP280 bmp;    
#define smokePIN A1
const int chipSelect = 4;
int RED = 5;
int GREEN = 7;
int GPSrx = 9;
int GPStx = 8;
long latitude;
long longitude;
TinyGPSPlus gps;
SoftwareSerial serial_connection(GPSrx, GPStx);
float temp;
float humidity; 
float temperature;
float pressure;
float heatic;
float altitude;
int smoke;
char dataString[10];
int k = 0;
char filename[] = "datalog.csv";
#define load_Res 10    
#define air_factor 9.83
float Res = 0;
float res = 0;
float result;
float SmokeCurve[3] ={4.50,0.70,-0.40};
float ratio;

//smoke sensor calibration
float resistance(int samples, int interval)
{
   int i;
   float res=0; 
   for (i=0;i<samples;i++) 
   {
      int adc_value=analogRead(smokePIN);
      res+=((float)load_Res*(1023-adc_value)/adc_value);
      delay(interval);
   }
   res/=samples;
   return res;
}

float SensorCalibration()
{
  int i;
  float val=0;    
  val=resistance(50,500);                
  val = val/air_factor;  
  return val; 
}

void setup(void) 
{ 
 //
 Serial.begin(9600);  //9600 baud rate
 serial_connection.begin(9600);
 Serial.println(F("GPS Start"));
 dht.begin();
 if(!bmp.begin()) {
  Serial.println(F("BMP280 not found!"));
 }
 pinMode(smokePIN, INPUT);
 pinMode(RED, OUTPUT);
 pinMode(GREEN,OUTPUT);
 Serial.println(F("Smoke sensor calibration..."));
 Res = SensorCalibration();
 Serial.print(F("Res= "));
 Serial.print(Res);
 Serial.println(F(" kOhm\n"));
} 

void loop(void) 
{  
  delay(2000);
  /* TO DO: must add weighted average */
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  heatic = dht.computeHeatIndex(temperature, humidity, false);
  pressure = bmp.readPressure()/100.0F;
  res = resistance(5,50);
  res /= Res;
  result=pow(10,(((log(res)-SmokeCurve[1])/SmokeCurve[2]) + SmokeCurve[0]));
  /* TO DO: data validation */ 
  if (result < 1000) {
    digitalWrite(GREEN, HIGH); //testing GREEN/RED LEDS
    digitalWrite(RED, LOW);
  } else {
    digitalWrite(RED, HIGH);
    digitalWrite(GREEN, LOW);
  }
  while (serial_connection.available() > 0)
    if (gps.encode(serial_connection.read()))
        delay(10);

     /* GEOSS/HACK demo to display collected data on Screen via serial cable */
     Serial.print(F("DHT22 Temperature: "));
     Serial.print(temperature);
     Serial.print(F(" C, Humidity: "));
     Serial.print(humidity);
     Serial.print(F("%, Heat perceived in C: "));
     Serial.println(heatic);
     Serial.print(F("GYBMP280 Pressure: "));
     Serial.print(pressure); // 100 Pa = 1 millibar
     Serial.println(" hPa");
     Serial.print(F("MQ-2: "));
     Serial.print(result);
     Serial.println(F(" ppm"));
     Serial.print(F("GPS: "));
     Serial.print(gps.location.lat(), 6);
     Serial.print(F(", "));
     Serial.println(gps.location.lng(), 6);
} 
