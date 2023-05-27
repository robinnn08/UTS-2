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
String datePath = "/date";
String timePath = "/time";
String statusPath = "/status";
String percentPath = "/fullness";

// Parent Node (to be updated in every loop)
String parentPath;

unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 30000;

int distanceCm;
int percentage;
int weight;
