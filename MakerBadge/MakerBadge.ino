#define ENABLE_GxEPD2_GFX 0  // we won't need the GFX base class
#include <GxEPD2_BW.h>
#include <WiFi.h>
#include <esp_sleep.h>
#include "bitmap.h"

// ESP32-S2 e-ink pinout
#define BUSY 42
#define RST 39
#define DC 40
#define CS 41

const char* ssid = "PICOPAD";
const char* password = "12345678";
const char* host = "192.168.4.1";
String delimiter = "|";
float temperature, humidity;

// Instantiate the GxEPD2_BW class for our display type
GxEPD2_BW<GxEPD2_213_B74, GxEPD2_213_B74::HEIGHT> display(GxEPD2_213_B74(CS, DC, RST, BUSY));  // GDEM0213B74 128x250, SSD1680

void setup() {
  Serial.begin(115200);

  // Initialize the display
  display.init(115200);

  esp_sleep_enable_timer_wakeup(1 * 60 * 1000000);  // Wake up after 1 minute

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  Serial.println("Sending GET request...");

  WiFiClient client;
  if (client.connect(host, 80)) {
    client.print("GET / HTTP/1.1\r\n");
    client.print("Host: ");
    client.println(host);
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("Connection failed");
    esp_deep_sleep_start();
  }

  delay(500);

  while (client.available()) {
    String line = client.readStringUntil('\r');

    int delimiterIndex = line.indexOf(delimiter);

    if (delimiterIndex != -1) {
      String firstPart = line.substring(0, delimiterIndex);    // First part of the string before the delimiter
      String secondPart = line.substring(delimiterIndex + 1);  // Second part of the string after the delimiter

      // Convert first part to float
      temperature = firstPart.toFloat();

      // Convert second part to float
      humidity = secondPart.toFloat();

      // Display the data on the e-ink display
      display.firstPage();
      do {
        display.setRotation(3);
        display.setFullWindow();
        display.fillScreen(GxEPD_WHITE);
        display.drawBitmap(0, 0, sarka_bmp, 250, 122, GxEPD_BLACK);
        display.setCursor(158, 96);
        display.setTextColor(GxEPD_BLACK);
        display.print("Teplota: ");
        display.print(temperature);
        display.print("C");
        display.setCursor(158, 106);
        display.print("Vlhkost: ");
        display.print(humidity);
        display.print("%");
      } while (display.nextPage());
    }
  }
  esp_deep_sleep_start();
}

void loop() {
  // never runs :-)
}