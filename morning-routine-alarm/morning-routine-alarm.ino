// Calibration data is stored in SPIFFS so we need to include it
#include "FS.h"

#include <SPI.h>

#include <TFT_eSPI.h> // Hardware-specific library
#include <WiFi.h>
#include <time.h>

#include "Widget.h"
#include "Page.h"

// This is the file name used to store the touch coordinate
// calibration data. Change the name to start a new calibration.
#define CALIBRATION_FILE "/TouchCalData3"

// Set REPEAT_CAL to true instead of false to run calibration
// again, otherwise it will only be done once.
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL false

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
HomePage homePage(480, 320, &tft);

// WIFI credentials
const char* ssid     = "MAKERSPACE";
const char* password = "12345678";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -5 * 60 * 60;
const int   daylightOffset_sec = 60*60;

time_t currTime = 0;
time_t lastDisplayedTime = 0;//last time that was displayed on the tft screen
time_t lastUpdatedTime = 0; //last time form ntp server

hw_timer_t *My_timer = NULL;
void IRAM_ATTR onTimer(){
  currTime += 1;
}

void setup(){
  Serial.begin(115200);
  pinMode(12, OUTPUT);
  analogWrite(12, 255);
  tft.init();

  // Set the rotation before we calibrate
  tft.setRotation(1);

  // call screen calibration
  touch_calibrate();

  // clear screen
  tft.fillScreen(TFT_BLACK);

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
  getTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  //begin timer interrupt that is triggered every second
  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 1000000, true);
  timerAlarmEnable(My_timer); //Just Enable

  homePage.draw();

}

void loop(){
  uint16_t x, y;
  //if user touches screen x,y are the coordinate of the finger press
  //other we set x and y to be an invalid touch
  if(!tft.getTouch(&x, &y)){
    x = UINT16_MAX;
    y = UINT16_MAX;
  }
  homePage.execute(x, y);

  //update time on the screen every minute
  if(currTime - lastDisplayedTime >= 60){
    lastDisplayedTime = currTime;
    struct tm * temp = localtime(&currTime);
    String hoursleadingSpace = (temp->tm_hour%12<10)?" ":"";
    String hours = ((temp->tm_hour % 12 == 0)? "12": ((String) (temp->tm_hour % 12)));
    String minutesLeadingZero = temp->tm_min<10?"0":"";
    String minutes = (String) temp->tm_min;
    String amOrPm = ((temp->tm_hour >= 12)? "PM": "AM");
    homePage.setTime(hoursleadingSpace + hours + ":" + minutesLeadingZero+ minutes + " " + amOrPm);
  }

  //double check time with ntp server every hour
  if(currTime-lastUpdatedTime >= 3600){
    getTime();
  }
}

void getTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    homePage.setConnected(false);
    Serial.println("Failed to obtain time");
    return;
  }
  homePage.setConnected(true);
  currTime = mktime(&timeinfo);
  lastUpdatedTime = currTime;
}

void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin()) {
    Serial.println("Formating file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}