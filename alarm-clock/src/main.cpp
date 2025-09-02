#include <Arduino.h>
#include <ESP32_NOW.h>
#include <WiFi.h>
#include <esp_mac.h>
#include <time.h>
#include <env.h>




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



uint8_t node_mac_address[] = {0xe4,0xb3,0x23,0xb5,0xe3,0xcc};
ESP_NOW_NODE_COMM com(node_mac_address,ESP_NOW_WIFI_CHANNEL, WIFI_IF_STA, nullptr);

void printLocalTime();

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

  

}

void loop() {
  printLocalTime();
  com.send_message((uint8_t *)"Lit", 3);
  delay(5000);
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