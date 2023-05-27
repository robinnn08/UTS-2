#include "header.h"

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

void displayWeight()
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
  display.print(weight);
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

void send24h() {
  String timestamp;
  databasePath = "/LogTest";
  String countPath = "/Counter/count";
  
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    while (!timeClient.update()) {
      timeClient.forceUpdate();
    }
    // Get current timestamp
    timestamp = timeClient.getFormattedDate();
    timestamp.replace("T", " ");
    timestamp.replace("Z", "");
    char timeStr[9];
    char dateStr[11];
    sscanf(timestamp.c_str(), "%10s %8s", dateStr, timeStr);
    Serial.print("time: ");
    Serial.println(timestamp);

    // Read the last count from Firebase
    Firebase.getInt(firebaseData, countPath);
    int lastCount = firebaseData.intData();

    // Increment the count
    int currentCount = lastCount + 1;
    int parentNode = currentCount -1;
    
    Serial.print("Current counter value: ");
    Serial.println(currentCount);

    parentPath = databasePath + "/" + String(parentNode);

    if (percentage >= 100) {
      json.set(statusPath.c_str(), "FULL");
    } else {
      json.set(statusPath.c_str(), "AVAILABLE");
    }
    json.set(weightPath, weight);
    json.set(percentPath, percentage);
    json.set(datePath, String(dateStr));
    json.set(timePath, String(timeStr));

    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&firebaseData, parentPath.c_str(), &json) ? "ok" : firebaseData.errorReason().c_str());

    // Update the count on the separate count path
    Firebase.setInt(firebaseData, countPath, currentCount);
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
  displayWeight();
}