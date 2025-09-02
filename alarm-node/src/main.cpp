#include <Arduino.h>

#include "ESP32_NOW.h"
#include "WiFi.h"

#include <esp_mac.h>

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
void setup() {
  Serial.begin(115200);

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
}

void loop() {

}
