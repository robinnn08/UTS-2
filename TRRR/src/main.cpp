#include <Arduino.h>
#include <Wire.h>
#include <FirebaseESP32.h>
#include <WiFi.h>
#include <time.h>
#include <NTPClient.h>
#include "HX711.h"
#include "SPI.h"
#include "Adafruit_SSD1306.h"
#include "Adafruit_GFX.h"

#define TRIGGER 26
#define ECHO 25
#define LOADCELL_DOUT_PIN 13
#define LOADCELL_SCK_PIN 18
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define SOUND_SPEED 0.034
#define HEIGHT 100

#define FIREBASE_HOST "https://esp-scale-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH "18Xdv2jQpBwHZaAdqPUVNyzlp1OBlfgFIOsDRxoc"
#define WIFI_SSID "hotspot1"
#define WIFI_PASSWORD "12345678"

FirebaseData firebaseData;
FirebaseJson json;
FirebaseJson rts;
HX711 scale;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
String databasePath;
// Database child nodes
String weightPath = "/weight";
String timePath = "/time";
String statusPath = "/status";
String percentPath = "/fullness";

// Parent Node (to be updated in every loop)
String parentPath;

unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 60000;

int distanceCm;
int percentage;
int weight;

void ultraSonic(){
  long duration;
  
  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(ECHO, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  percentage = (HEIGHT - distanceCm) * 100 / HEIGHT;
}

void displayWeight(int reading)
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display weight
  display.println("Weight:");
  display.display();
  display.setCursor(0, 30);
  display.setTextSize(2);
  display.print(reading);
  display.print(" ");
  display.print("grams");
  display.display();
}

void serialReading(){
  weight = scale.get_units();
  float lastReading;
  if (scale.wait_ready_timeout(2000))
  {
    Serial.print("Weight: ");
    Serial.println(weight);
    Serial.print("Distance: ");
    Serial.println(distanceCm);
    Serial.print("Percentage: ");
    Serial.println(percentage);
    if (weight != lastReading)
    {
      displayWeight(weight);
    }
    lastReading = weight;
  }
  else
  {
    Serial.println("HX711 not found.");
  }
  scale.power_down();
  delay(100);
  scale.power_up();
}

void send24h(){
  String timestamp;
  databasePath = "/LogTest";
  
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    while(!timeClient.update()) {
    timeClient.forceUpdate();
    }
    //Get current timestamp
    timestamp = timeClient.getFormattedDate();
    timestamp.replace("T", " ");
    timestamp.replace("Z", "");
    Serial.print ("time: ");
    Serial.println (timestamp);

    parentPath = databasePath + "/" + String(timestamp);
    
    if (percentage >= 100){
      json.set(statusPath.c_str(), "FULL");
    } else {
      json.set(statusPath.c_str(), "AVAILABLE");
    }
    json.set(weightPath.c_str(), String(weight));
    json.set(percentPath.c_str(), String(percentage));
    json.set(timePath, String(timestamp));
 
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&firebaseData, parentPath.c_str(), &json) ? "ok" : firebaseData.errorReason().c_str());
  }
  
}

void realtimeSend(){
  rts.set("/capacity", percentage);
  Firebase.updateNode(firebaseData, "/Read/Tong1", rts);
  rts.set("/weight1", weight);
  Firebase.updateNode(firebaseData, "/Read/Tong1", rts);
  delay(1000);
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  Firebase.setwriteSizeLimit(firebaseData, "tiny");

  Serial.println("------------------------------------");
  Serial.println("Connected...");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);

  Serial.println("Initializing the scale");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(-371);
  scale.tare();

  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(25200);
  
}

void loop() {
  ultraSonic();
  send24h();
  realtimeSend();
  serialReading();
}