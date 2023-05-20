// Nama: CHristian Robin Kieswanto NIM:2540128474
// 474, X=4, Y=7, Z=4
// Temperatur setiap 4 detik
// Kelembaban setiap 7 detik
// Intensitas cahaya setiap 4 detik
#include <Wire.h>
#include <BH1750.h>
#include <DHTesp.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// define pin data dari DHT11
#define DHTPIN 4 

DHTesp dht;
BH1750 lightMeter;

// fungsi untuk menunjukkan hitungan detik
void showTimer(void *pvParameters) {
  while(1) { //looping task
    Serial.print("Timer: ");
    Serial.println(millis()/1000);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// fungsi untuk membaca suhu setiap 4 detik
void readTemperature(void *pvParameters) {
  while(1) { //looping task
    float temperature = dht.getTemperature();
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println("C");
    vTaskDelay(pdMS_TO_TICKS(4000)); // delay 4 detik
  }
}

// fungsi untuk membaca kelembaban setiap 7 detik
void readHumidity(void *pvParameters) {
  while(1) { //looping task
    float humidity = dht.getHumidity();
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
    vTaskDelay(pdMS_TO_TICKS(7000)); // delay 7 detik
  }
}

// fungsi untuk membaca intensitas cahaya setiap 4 detik
void readLux(void *pvParameters) {
  while(1) { //looping task
    float lux = lightMeter.readLightLevel();
    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lx");
    vTaskDelay(pdMS_TO_TICKS(4000));  // delay 4 detik
  }
}

void setup() {
  Serial.begin(115200);
  // set up sensor DHT11
  dht.setup(DHTPIN, DHTesp::DHT11);
  // set up sensor BH1750
  Wire.begin();
  lightMeter.begin();
  // membuat task RTOS untuk menunjukkan timer, membaca suhu, kelembaban, dan intensitas cahaya
  xTaskCreate(showTimer, "Time", configMINIMAL_STACK_SIZE+1024, NULL, 5 , NULL);
  xTaskCreate(readTemperature, "Temp", configMINIMAL_STACK_SIZE+1024, NULL, 5 , NULL);
  xTaskCreate(readHumidity, "Humid", configMINIMAL_STACK_SIZE+1024, NULL, 5, NULL);
  xTaskCreate(readLux, "Lux", configMINIMAL_STACK_SIZE+1024, NULL, 5, NULL);
}

void loop() {
  // tidak ada yang perlu dilakukan di loop karena semua sudah dihandle oleh task RTOS
}