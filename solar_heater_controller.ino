#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DallasTemperature.h>
#include "LIFOQueue.h"
#include <ESP8266WiFi.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define VERSION_NUMBER "2023-05-13"

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const byte optocoupler = D5;
const byte oneWireBus = D6;
String yellowLineString = "";
LiFoQueue queue(20);
int triggerTemperature = 25;
// setup temperature sensor communication
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);
  Serial.println("Begin execution ...");
  WiFi.forceSleepBegin();
  initScreen();
  Serial.println("After init screen ...");
  pinMode(optocoupler, OUTPUT);
  digitalWrite(optocoupler, LOW);

  sensors.begin();
  //set the resolution to the max
  sensors.setResolution(12);
  Serial.println("Setup ends...");
}

void initScreen(){  
  Serial.println("Init screen...");
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();
  display.setTextColor(WHITE);
}

void pushToScreen(String message){
  if (yellowLineString.length() > 9) {
    yellowLineString = yellowLineString.substring(yellowLineString.indexOf(" ")+5, yellowLineString.length()) + message + " ";
  }else {
    yellowLineString = yellowLineString + message + " ";
  } 
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(yellowLineString);
  display.setTextSize(4);
  display.setCursor(12, 25);
  display.println(String(queue.getAvarage(),1));
  display.display();
}

void loop() {
  /*Serial.print("==");
  Serial.println(String(onSwitch));*/
  sensors.requestTemperatures(); // Send the command to get temperature readings
  double value = sensors.getTempCByIndex(0);

  Serial.print("sensor reading: ");
  Serial.println(value);
  //filter out the possible junk values
  if (value > -30 && value < 140) {
    queue.pushValue(value);
    pushToScreen(String(value,1));
  } else {
    Serial.println("junk value...");
  }
  if (queue.getSize()>= 10 && queue.getAvarage() >= triggerTemperature ) {
    Serial.println("Set optocoupler HIGH");
    //digitalWrite(optocoupler, HIGH);
  } else {
    //Serial.println("Set optocoupler LOW");
    digitalWrite(optocoupler, LOW);
  }
  /*Serial.print("Queue size: ");
  Serial.print(queue.getSize());
  Serial.print(" | Queue average: ");
  Serial.println(queue.getAvarage());*/
  delay(1000);
}
