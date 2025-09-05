#include <Arduino.h>

#include "ESP32_NOW.h"
#include "WiFi.h"

#include <esp_mac.h>

#include <Adafruit_SSD1306.h>
#include "startScreen.h"
#include "congrats.h"

#define ESP_NOW_WIFI_CHANNEL 6

class ESP_NOW_NODE_RECV : public ESP_NOW_Peer {
public:
  // Constructor of the class
  ESP_NOW_NODE_RECV(const uint8_t *mac_addr, uint8_t channel, wifi_interface_t iface, const uint8_t *lmk) : ESP_NOW_Peer(mac_addr, channel, iface, lmk) {}

  // Destructor of the class
  ~ESP_NOW_NODE_RECV() {}

  // Function to register the master peer
  bool add_peer() {
    if (!add()) {
      log_e("Failed to register the broadcast peer");
      return false;
    }
    return true;
  }

  // Function to print the received messages from the master
  void onReceive(const uint8_t *data, size_t len, bool broadcast) {
    Serial.printf("Received a message from master " MACSTR " (%s)\n", MAC2STR(addr()), broadcast ? "broadcast" : "unicast");
    Serial.printf("  Message: %s\n", (char *)data);
  }
};

uint8_t alarm_mac_address[] = {0xe4,0xb3,0x23,0xb5,0xf7,0x5c};
ESP_NOW_NODE_RECV comm(alarm_mac_address, ESP_NOW_WIFI_CHANNEL, WIFI_IF_STA, nullptr);


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int numTasks = 6;

const char *tasks[] = {"Shower", "Floss", "Brush\nTeeth", "Mosturize", "Sunscreen", "Deoderant"};
const int taskTime[] = {10, 2, 2, 3, 3, 2 };

int currTaskIndex = -1;

unsigned long prevTime = 0;

const int buttonPin = D0;

hw_timer_t *timer = nullptr;

volatile bool updateTime = true;
volatile unsigned long taskTimeLeft = 0;


void handleTimerInterrupt(){
  if (taskTimeLeft > 0){
    taskTimeLeft--;
  }
  updateTime = true;
}

String secondsToStr(unsigned long seconds){
  return (String) ((seconds) / 60) +  (String) ":" + ((seconds % 60 < 10)? (String)"0" : (String)"") + (String) ((seconds) % 60);
}

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT);
  WiFi.mode(WIFI_STA);
  WiFi.setChannel(ESP_NOW_WIFI_CHANNEL);
  while(!WiFi.STA.started()){
    delay(100);
  }

  if(!ESP_NOW.begin()){
    Serial.println("Failed to initialized ESP_NOW");
    Serial.println("Restarting the ESP32");
    delay(3000);
    ESP.restart();
  }

  if(!comm.add_peer()){
    Serial.println("Failed to add alarm clock as peer");
  }

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  timer = timerBegin(1000000);
  timerAttachInterrupt(timer, handleTimerInterrupt);
  timerAlarm(timer, 1000000, true, -1);
  // Clear the buffer
  display.clearDisplay();
  display.drawBitmap(0,0, startScreen, startScreenWidth, startScreenHeight, 1);
  display.display();

}

void loop() {
  if (digitalRead(buttonPin) == LOW && (currTaskIndex == -1 || taskTimeLeft == 0 ) && currTaskIndex < numTasks - 1){
    currTaskIndex++;
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, strlen(tasks[currTaskIndex]) > 10 ? 0 : 20);
    display.setTextSize(2);
    display.println(tasks[currTaskIndex]);
    taskTimeLeft = taskTime[currTaskIndex];
    display.display();
    prevTime = millis();
  }

  if(updateTime && currTaskIndex != -1 && currTaskIndex < numTasks ) {
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display.setCursor(10, 40);
    display.setTextSize(1);
    display.println(secondsToStr(taskTimeLeft) + "   ");
    display.display();
    updateTime = false;
  }

  if (digitalRead(buttonPin) == LOW && currTaskIndex == numTasks - 1 && taskTimeLeft == 0){
    display.clearDisplay();
    display.drawBitmap(0,0, congrats, congratsWidth, congratsHeight, 1);
    display.display();
    currTaskIndex++;
  }




}
