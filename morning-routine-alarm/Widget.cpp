#include "Widget.h"
#include <SPI.h>

#include <TFT_eSPI.h> // Hardware-specific library

Widget::Widget(uint16_t _x, uint16_t _y, TFT_eSPI* _tft){
  this->x = _x;
  this->y = _y;
  this->tft = _tft;
}

void Widget::setPos(uint16_t _x, uint16_t _y){
  this->x = _x;
  this->y = _y;
}

void Widget::draw(){
  return;
}

Button::Button(uint16_t _x, uint16_t _y, uint16_t _w, uint16_t _h, TFT_eSPI* _tft, String _text) : Widget(_x, _y, _tft){
  this->text = _text;
  this->w = _w;
  this->h = _h;
}

void Button::setBounds(uint16_t _w, uint16_t _h){
  this->w = _w;
  this->h = _h;
}

void Button::draw(){
  //draw button background
  this->tft->fillRoundRect(this->x, this->y, this->w, this->h, this->bR, this->bColor);
  //draw button text
  this->tft->setTextColor(this->textColor, this->bColor);
  //Set text datum to be middle center of text
  this->tft->setTextDatum(4);

  this->tft->setTextSize(this->textSize);

  //draw text in center of the button using font 2
  this->tft->drawString(text, this->x + this->w/2, this->y + this->h/2, 2);

  if (this->disabled){
    this->tft->fillRect(this->x, this->y + this->h/2 - this->h/10, this->w, this->h/10, this->textColor);
  }
}

void Button::setText(String _text){
  this->text = _text;
}

//set the whether the button is disabled or not
void Button::setDisabled(bool _disabled){
  this->disabled = _disabled;
}

//returns true, meaning the button has been pressed, if the button has not been disabled and the x and y coordinates fall within the button
bool Button::pressed(uint16_t touchX, uint16_t touchY){
  return (!this->disabled) && (this->x <= touchX) && (touchX <= this->x + this->w) && (this->y <= touchY) && (touchY <= this->y + this->h);
}

Text::Text(uint16_t _x, uint16_t _y, uint8_t _size, uint8_t _datum, TFT_eSPI* _tft, String _text): Widget(_x, _y, _tft){
  this->text = _text;
  this->textSize = _size;
  this->datum = _datum;
}

void Text::draw(){
  this->tft->setTextColor(this->textColor, this->bColor);
  //Set text datum to be top left corner of text
  this->tft->setTextDatum(this->datum);
  this->tft->setTextSize(this->textSize);
  //draw text at position x y using font 2
  this->tft->drawString(text, this->x, this->y, 2);
}

void Text::setText(String _text){
  this->text = _text;
}




