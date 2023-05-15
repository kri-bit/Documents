/*************************************************************

  This is a simple demo of sending and receiving some data.
  Be sure to check out other examples!
 *************************************************************/

/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           "TMPLJkCMlJCK"
#define BLYNK_TEMPLATE_NAME         "Quickstart Template"
#define BLYNK_AUTH_TOKEN            "FWuD2-aqrz5TKNHJGAWSzj6a_7qh4kYO"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial


#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2

// /* header file for ASC712 current sendor */
// #include "ACS712.h"

// /* assigning current sensor type and its analog pin */
// ACS712 sensor(ACS712_20A, A0);
// //ACS712_20A for 20 Amp type

//Anlog pin configures ACS712 sensor
const int sensorIn = A0;
const int relayIn = 14;
int mVperAmp = 100; // use 185 for 5A, 100 for 20A Module and 66 for 30A Module

double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;
float watts;

int max_cutoff_watt = 1000;
int max_cutoff_kva = 300;

// unit calculation variables
float kwh;
unsigned long totalET;
float dailykwh;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Gokul 🏡";
char pass[] = "AllowMe6";

BlynkTimer timer;

// This function is called every time the Virtual Pin 0 state changes
BLYNK_WRITE(V0)
{
  // Set incoming value from pin V0 to a variable
  int value = param.asInt();

  // Update state
  Blynk.virtualWrite(V1, value);
  if(value==1){
    digitalWrite(relayIn, LOW);
  }
  else{
    digitalWrite(relayIn, HIGH);
  }
}

BLYNK_WRITE(V6)
{
  // Set incoming value from pin V6 to a variable
  max_cutoff_watt = param.asInt();

  Serial.println("Max cuttoff watt (In side Blynk write function) :");
  Serial.print(max_cutoff_watt);
}

// max_cutoff_kva
BLYNK_WRITE(V7)
{
  // Set incoming value from pin V6 to a variable
  max_cutoff_kva = param.asInt();

  Serial.print("Max cuttoff unit in Killo watt hour:");
  Serial.println(max_cutoff_kva);
}

// dailykwh
BLYNK_WRITE(V8)
{
  // Set incoming value from pin V6 to a variable
  dailykwh = param.asDouble();

  Serial.print("dailykwh in Killo watt hour:");
  Serial.println(dailykwh);
}

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{
  // Change Web Link Button message to "Congratulations!"
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

// This function will get the PWM analog signal and convert it into sampled digital value
void readACS712(){
  Voltage = getVPP();
  VRMS = (Voltage/2.0) *0.707; // sq root
  AmpsRMS = (VRMS * 1000)/mVperAmp;
  Serial.print(AmpsRMS);
  Serial.print(" Amps RMS -- new RMS ");
  if (AmpsRMS < 0.09) {
    AmpsRMS = 0;
  }
  Serial.println(AmpsRMS);
  Blynk.virtualWrite(V4, AmpsRMS);
  watts = (240*AmpsRMS); //Observed 18-20 Watt when no load was connected, so substracting offset value to get real consumption.
  Serial.print(watts); 
  Serial.println(" Watt ");
  Blynk.virtualWrite(V5, watts);
  pinMode(sensorIn, INPUT);
}

float getVPP()
{
  float result;
  
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here
  
   uint32_t start_time = millis();

   while((millis()-start_time) < 1000) //sample for 1 Sec
   {
       readValue = analogRead(sensorIn);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           /*record the maximum sensor value*/
           minValue = readValue;
       }
    }
   
   // Subtract min from max
   result = ((maxValue - minValue) * 5)/1024.0;
      
   return result;
 }

// This function sends Arduino's uptime every second to Virtual Pin 2.
void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V2, millis() / 1000);
  // float I = sensor.getCurrentAC();
  // Serial.print("Current: ");
  // Serial.print(I);
  // Serial.println(" A");
  // Blynk.virtualWrite(V4, I);
  // Serial.print("Watt: ");
  // Serial.print(I*260);
  // Serial.println(" W");
  // Blynk.virtualWrite(V5, I*260);
}

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

void setup()
{
  // Debug console
  Serial.begin(115200);

  // relay default turn off
  digitalWrite(relayIn, HIGH);

  // lcd initiate
  lcd.init();
  lcd.backlight();

  // sensor.calibrate();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to Wifi: ");
  lcd.print(ssid);
  for (int positionCounter = 0; positionCounter < 16; positionCounter++) {
    // scroll one position left:
    lcd.scrollDisplayLeft();
    // wait a bit:
    delay(150);
  }


  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  // You can also specify server:
  //Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
  //Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, IPAddress(192,168,1,100), 8080);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" $$ Connected $$ ");

  // Setup a function to be called every second
  timer.setInterval(1000L, myTimerEvent);
  timer.setInterval(1000L, readACS712);
  Blynk.syncVirtual(V6);
  Serial.println("Max cuttoff watt (setup):");
  Serial.print(max_cutoff_watt);
  Blynk.syncVirtual(V7);
  Serial.println("Max cuttoff kva (setup):");
  Serial.print(max_cutoff_kva);
  Blynk.syncVirtual(V8);
  Serial.println("Max cuttoff kva (setup):");
  Serial.print(dailykwh);
  pinMode(relayIn, OUTPUT);
}

void loop()
{
  uint32_t ts1 = millis();
  Blynk.run();
  timer.run();
  Serial.print("Max cuttoff watt(loop):");
  Serial.println(max_cutoff_watt);
  Serial.print("Max cuttoff kva (loop):");
  Serial.println(max_cutoff_kva);
  uint32_t ts2 = millis();
  uint32_t ts3 = (ts2-ts1)/1000;
  kwh = watts * ts3 / 3600000;
  totalET = totalET + ts3;
  Serial.print("Seconds: ");
  Serial.println(totalET);
  Serial.print("Current KWh: "); 
  Serial.println(kwh, 4);
  dailykwh = dailykwh + kwh;
  Serial.print("Daily KWh: "); 
  Serial.println(dailykwh, 4);
  Blynk.virtualWrite(V8, dailykwh);

  // LCD display connections
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("C:");
  lcd.print(AmpsRMS);
  lcd.print("A||");

  lcd.print("P:");
  lcd.print(watts);
  lcd.print("W");

  lcd.setCursor(0, 1);
  lcd.print("U:");
  lcd.print(dailykwh);
  lcd.print("kw");

  lcd.print("||");
  lcd.print(max_cutoff_kva);
  lcd.print("kwh");

  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Blynk. Remember
  // to avoid delay() function!
}

