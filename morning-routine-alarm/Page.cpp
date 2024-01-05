#include "Page.h"

Page::Page(uint16_t _w, uint16_t _h, TFT_eSPI* _tft){
  this->w = _w;
  this->h = _h;
  this->tft = _tft;
}

HomePage::HomePage(uint16_t _w, uint16_t _h, TFT_eSPI* _tft):Page(_w, _h, _tft){
  this->changeAlarm = new Button(0 + this->buttonPadding, 250+ this->buttonPadding, 200 - 2*this->buttonPadding, _h - 250 - 2*this->buttonPadding, this->tft, "Change Alarm");
  this->dimScreen = new Button(215 + this->buttonPadding, 250+ this->buttonPadding, 155 - 2*this->buttonPadding, _h - 250 - 2*this->buttonPadding, this->tft, "Dim Screen");
  this->unlockPhone = new Button(_w - 100 + this->buttonPadding, 250+ this->buttonPadding, 100 - 2*this->buttonPadding, _h - 250 - 2*this->buttonPadding, this->tft, "Unlock");
  this->timeDisp = new Text(_w/2, _h/2, 12, 4, this->tft, "");
  this->currTask = new Text(10, 10, 2, 0, this->tft, "current task: NS");
  this->ntpConnection = new Text(250, 10, 2, 0, this->tft, "");
  this->phoneLocked = new Text(10, 60, 2, 0, this->tft, "phone locked: n");
  
}

HomePage::~HomePage(){
  delete this->changeAlarm;
  delete this->dimScreen;
  delete this->unlockPhone;
  delete this->timeDisp;
  delete this->currTask;
  delete this->ntpConnection;
  delete this->phoneLocked;
}

void HomePage::draw(){
  this->tft->fillScreen(TFT_BLACK);
  this->changeAlarm->draw();
  this->dimScreen->draw();
  this->unlockPhone->draw();
  this->timeDisp->draw();
  this->currTask->draw();
  this->ntpConnection->draw();
  this->phoneLocked->draw();
}

void HomePage::execute(uint16_t x, uint16_t y){
  if(this->refreshTime){
    this->timeDisp->draw();
    this->refreshTime = false;
  }
  if(this->changeAlarm->pressed(x, y)){
    Serial.println("changeAlarm pressed");
  }
  else if(this->dimScreen->pressed(x, y)){
    Serial.println("dimScreen pressed");
  }
  else if(this->unlockPhone->pressed(x, y)){
    Serial.println("unlockPhone pressed");
  }
}

void HomePage::setTime(String currTime){
  this->timeDisp->setText(currTime);
  this->refreshTime = true;
}

//task can be NS (Not Started), FN (Finished), or a number representing the current checkpoint 
void HomePage::setTask(String taskStr){
  this->currTask->setText("currrent task: " + taskStr);
  this->refreshTask = true;
}

void HomePage::setConnected(bool isConnected){
  this->ntpConnection->setText("ntp connection: " + (String)(isConnected ?"y" : "n"));
  this->refreshNtp = true;
}

void HomePage::setLocked(bool isLocked){
  this->ntpConnection->setText("phone locked: " + (String)(isLocked ?"y" : "n"));
  this->refreshLocked = true;
}