#include "thingProperties.h"  // Arduino Cloud için (otomatik eklenir)
#include <ESP32Servo.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#define TRIG_PIN 5
#define ECHO_PIN 18
#define DHT_PIN 15
#define LDR_PIN 34
#define RELAY_PIN 19
#define SERVO_PIN 13

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

DHT dht(DHT_PIN, DHT22);
Servo myServo;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  myServo.attach(SERVO_PIN);
  myServo.write(0);  // Başlangıçta kapalı

  dht.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

  initProperties();  // Arduino Cloud için
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  Serial.println("Setup Complete");
}void loop() {
  ArduinoCloud.update();

  // Röleyi Arduino Cloud'dan kontrol et
  digitalWrite(RELAY_PIN, relayControl ? LOW : HIGH);  // LOW → Aktif

  // Eğer relayControl açık ise servo'yu kontrol edelim
  if (relayControl) {
    float distanceCm = getDistance();
    distance = distanceCm;  // Arduino Cloud değişkeni güncelle

    if (distanceCm < 10.0) {
      myServo.write(90);  // Kapak açılır
    } else {
      myServo.write(0);   // Kapak kapanır
    }
  } else {
    myServo.detach();  // Röle kapalıysa servo kapalı
  }

  // DHT22 verilerini oku ve Cloud'a gönder
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // LDR verisi oku ve Cloud'a gönder
  lightLevel = analogRead(LDR_PIN);

  // OLED ekranda göster
  showOnOLED();

  delay(1000);  // 1 saniyede bir güncelle
}
float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  float distanceCm = duration * 0.034 / 2;

  return distanceCm;
}
void showOnOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("Dist: "); display.print(distance); display.println(" cm");

  display.setCursor(0, 10);
  display.print("Temp: "); display.print(temperature); display.println(" C");

  display.setCursor(0, 20);
  display.print("Hum: "); display.print(humidity); display.println(" %");

  display.setCursor(0, 30);
  display.print("Light: "); display.print(lightLevel);

  display.display();
}

