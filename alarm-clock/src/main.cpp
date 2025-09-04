#include <Arduino.h>
#include <ESP32_NOW.h>
#include <WiFi.h>
#include <esp_mac.h>
#include <time.h>
#include <env.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "./images/alarmClock.h"
#include "./images/watch.h"
#include "./images/lock.h"
#include "./images/note.h"
#include "./images/play.h"
#include "./images/volume.h"
#include "./images/Zero.h"
#include "./images/One.h"
#include "./images/Two.h"
#include "./images/Three.h"
#include "./images/Four.h"
#include "./images/Five.h"
#include "./images/Six.h"
#include "./images/Seven.h"
#include "./images/Eight.h"
#include "./images/Nine.h"
#include "./images/colon.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define LOGO_HEIGHT   64
#define LOGO_WIDTH    128

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);



const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

#define ESP_NOW_WIFI_CHANNEL 6

class ESP_NOW_NODE_COMM: public ESP_NOW_Peer {
  public:
    ESP_NOW_NODE_COMM(uint8_t *peer_addr, uint8_t channel, wifi_interface_t iface, const uint8_t *lmk) : ESP_NOW_Peer(peer_addr, channel, iface, lmk) {}
    
    ~ESP_NOW_NODE_COMM(){
      remove();
    }

    bool begin(){
      if (!ESP_NOW.begin() || !add()) {
        log_e("Failed to initialize ESP-NOW or register the broadcast peer");
        return false;
      }
      return true;
    }

    bool send_message(const uint8_t *data, size_t len){
      if (!send(data, len)) {
        log_e("Failed to broadcast message");
        return false;
      }
      return true;
    }
};

uint8_t *selected = nullptr;



uint8_t node_mac_address[] = {0xe4,0xb3,0x23,0xb5,0xe3,0xcc};
ESP_NOW_NODE_COMM com(node_mac_address,ESP_NOW_WIFI_CHANNEL, WIFI_IF_STA, nullptr);

void printLocalTime();
const uint8_t *numberFonts[] = {Zero, One, Two, Three, Four, Five, Six, Seven, Eight, Nine};
void printTime(Adafruit_SSD1306 *disp, uint8_t hours, uint8_t minutes){

  uint8_t numWidth = 16;
  uint8_t numHeight = 19;

  uint8_t hoursTensPlaceX = 27;
  uint8_t hoursTensPlaceY = 21;
  uint8_t hoursTensPlaceVal = hours / 10;

  disp->drawBitmap(hoursTensPlaceX,hoursTensPlaceY, numberFonts[hoursTensPlaceVal], numWidth, numHeight, 1, 0);

  uint8_t hoursOnesPlaceX = 44;
  uint8_t hoursOnesPlaceY = 21;
  uint8_t hoursOnesPlaceVal = hours % 10;

  disp->drawBitmap(hoursOnesPlaceX, hoursOnesPlaceY, numberFonts[hoursOnesPlaceVal], numWidth, numHeight, 1, 0);

  uint8_t colonX = 60;
  uint8_t colonY = 28;

  disp->drawBitmap(colonX, colonY, colon, colonWidth, colonHeight, 1, 0);

  uint8_t minutesTensPlaceX = 68;
  uint8_t minutesTensPlaceY = 21;
  uint8_t minutesTensPlaceVal = minutes / 10;

  disp->drawBitmap(minutesTensPlaceX,minutesTensPlaceY, numberFonts[minutesTensPlaceVal], numWidth, numHeight, 1, 0);

  uint8_t minutesOnesPlaceX = 86;
  uint8_t minutesOnesPlaceY = 21;
  uint8_t minutesOnesPlaceVal = minutes % 10;

  disp->drawBitmap(minutesOnesPlaceX, minutesOnesPlaceY, numberFonts[minutesOnesPlaceVal], numWidth, numHeight, 1, 0);

}

uint16_t currTime = 0;
uint8_t loadingHeight = 10;
uint8_t loadingWidth = 32;
uint8_t loadingX = 92;
uint8_t loadingY = 4;
uint8_t loadingBuffer[volumeHeight*volumeWidth];

void setup() {

  
  Serial.begin(115200);
  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);

  // Initialize the Wi-Fi module
  WiFi.mode(WIFI_STA);
  WiFi.setChannel(ESP_NOW_WIFI_CHANNEL);
  while (!WiFi.STA.started()) {
    delay(100);
  }

  Serial.println("ESP-NOW Example - Broadcast Master");
  Serial.println("Wi-Fi parameters:");
  Serial.println("  Mode: STA");
  Serial.println("  MAC Address: " + WiFi.macAddress());
  Serial.printf("  Channel: %d\n", ESP_NOW_WIFI_CHANNEL);

  // Register the broadcast peer
  if (!com.begin()) {
    Serial.println("Failed to initialize broadcast peer");
    Serial.println("Reebooting in 5 seconds...");
    delay(5000);
    ESP.restart();
  }
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();
  
  display.drawBitmap(alarmClockX, alarmClockY, alarmClock, alarmClockWidth, alarmClockHeight, selected == alarmClock ? 0 : 1, selected == alarmClock ? 1 : 0);
  display.drawBitmap(watchX, watchY, watch, watchWidth, watchHeight, selected == watch ? 0 : 1, selected == watch ? 1 : 0);
  display.drawBitmap(noteX, noteY, note, noteWidth, noteHeight, selected == note ? 0 : 1, selected == note ? 1 : 0);
  display.drawBitmap(playX, playY, play, playWidth, playHeight, selected == play ? 0 : 1, selected == play ? 1 : 0);
  display.drawBitmap(volumeX, volumeY, volume, volumeWidth, volumeHeight, selected == volume ? 0 : 1, selected == volume ? 1 : 0);
  display.drawBitmap(lockX, lockY, lock, lockWidth, lockHeight, selected == lock ? 0 : 1, selected == lock ? 1 : 0);
  Serial.println(loadingHeight*loadingWidth);
  generateVolume(loadingBuffer, loadingHeight, loadingWidth, 50);
  display.drawBitmap(loadingX, loadingY, loadingBuffer, loadingWidth, loadingHeight, 1, 0 );
  // Now that the display is all setup, turn on the display
  display.display();

  // printTime(display, 1, 30);
}

void loop() {
  currTime++;
  // Serial.printf("%u, %u\n", currTime/60, currTime%60);
  // currTime/60;
  // currTime%60;
  printTime(&display, currTime/60, currTime%60);
  generateVolume(loadingBuffer, loadingHeight, loadingWidth, 100);
  display.drawBitmap(loadingX, loadingY, loadingBuffer, loadingWidth, loadingHeight, 1, 0 );
  display.display();
  delay(1000);
  // printLocalTime();
  // com.send_message((uint8_t *)"Lit", 3);
  // delay(5000);
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");

  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%A", &timeinfo);
  Serial.println(timeWeekDay);
  Serial.println();
}