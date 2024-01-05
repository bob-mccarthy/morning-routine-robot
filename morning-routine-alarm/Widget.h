#ifndef WIDGET_H
#define WIDGET_H
#include <TFT_eSPI.h> 

class Widget {
  public:
    Widget(){};
    Widget(uint16_t _x, uint16_t _y, TFT_eSPI* _tft); //initialize widget with x and y coordinate of left corner as well as the width and height of the robot with tft to draw on
    void setPos(uint16_t _x, uint16_t _y); //set x and y position of the widget
    virtual void draw(); //draws the widget on the TFT screen
  protected:
    uint16_t x,y;
    TFT_eSPI* tft = NULL;

};

class Button: public Widget {
  public:
    Button(){};
    Button(uint16_t _x, uint16_t _y, uint16_t _w, uint16_t _h, TFT_eSPI* _tft, String _text); //intialize button with position, bounding box, text, and TFT to draw on
    void draw(); //draw the button
    void setText(String _text); //set the button
    void setBounds(uint16_t _w, uint16_t _h);
    bool pressed(uint16_t touchX, uint16_t touchY); // returns true and the x,y position of the touch is inside of the button
    void setDisabled (bool _disabled);

  private:
    String text;
    uint16_t w,h;
    uint8_t bR = 20; //button radius
    uint16_t bColor= TFT_WHITE; //button color 
    uint8_t textSize = 2; //text size
    uint16_t textColor = TFT_BLACK; //button text color
    bool disabled = false;
};

class Text: public Widget{
  public:
    Text(){};
    Text(uint16_t _x, uint16_t _y, uint8_t _size, uint8_t _datum, TFT_eSPI* _tft, String _text); //intialize text with position, bounding box, text, and TFT to draw on
    void draw();
    void setText(String _text);
    void setDatum(uint8_t mode);
  private:
    String text;
    uint16_t bColor = TFT_BLACK; //background color 
    uint16_t textColor = TFT_WHITE; //text color
    uint8_t textSize; //text size
    uint8_t datum = 0; //defines the point where we place the text (ie 0:top left corner, 4: center, etc)
};

#endif