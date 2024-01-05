#ifndef PAGE_H
#define PAGE_H
#include <TFT_eSPI.h> 
#include "Widget.h"

class Page {
  public:
    Page(uint16_t _w, uint16_t _h, TFT_eSPI* _tft);
    virtual void draw(){}; //draws the page on the screen
    virtual void execute(uint16_t x, uint16_t y){}; // carries out the proper function given the x, y coordinate of a press on the screen
  protected:
    uint16_t w,h; //size of page
    TFT_eSPI* tft; //tft screen to write on 
    uint8_t buttonPadding = 5;
};

class HomePage: public Page{
  public:
    HomePage(uint16_t _w, uint16_t _h, TFT_eSPI* _tft);
    ~HomePage();
    void draw();
    void execute(uint16_t x, uint16_t y);
    void setTime(String currTime);
    void setTask(String taskStr);
    void setConnected(bool isConnected);
    void setLocked(bool isLocked);
  private:
    Button* changeAlarm, *dimScreen, *unlockPhone; //buttons to display on screen
    Text* timeDisp, *currTask, *ntpConnection, *phoneLocked; //text to display on screen
    bool refreshTime = false;// true if timeDisp text needs to be refreshed
    bool refreshTask = false; // true if currTask text needs to be refreshed
    bool refreshNtp = false;// true if ntpConnection text needs to be refreshed
    bool refreshLocked = false; //true if phoneLocked text needs to be refreshed
    
};

#endif