/********************************************************************************************/
/* Library for the common LCD 1602 & keypad Arduino shield                                  */
/* http://www.dfrobot.com/wiki/index.php?title=Arduino_LCD_KeyPad_Shield_%28SKU:_DFR0009%29 */
/* Uses digital pins 4-9,10                                                                 */
/* Provides for kb input including autorepeat, drawing primitives, etc                      */
/*                                                                                          */
/* Alex Sokolsky, April 2015                                                                */
/*                                                                                          */
/********************************************************************************************/

#ifndef Lcd1602KeypadShield_h
#define Lcd1602KeypadShield_h

#include <Arduino.h>
#include "LiquidCrystal.h"

/** define some values for button (key) scan codes */
const char VK_NONE = 0;
const char VK_RIGHT= 1;
const char VK_UP   = 2;
const char VK_DOWN = 3;
const char VK_LEFT = 4;
const char VK_ENTER= 5;

class Lcd1602KeypadShield : public LiquidCrystal 
{
public:
   /** LCD geometry */
  static const char s_iRows = 2;
  static const char s_iCols = 16;
  
  /** delay in ms before the first auto-repeat if fired */ 
  static const int s_iAutoRepeatDelay = 1000; // ms
  /** delay in ms between auto-repeats */ 
  static const int s_iAutoRepeatPeriod = 500; // ms

  Lcd1602KeypadShield();
  
  void begin() {
    LiquidCrystal::begin(s_iCols, s_iRows);
  }
  /**
   * draw an array of lines, iLines is the number of lines in the array
   * display iFirstLine on top
   */
  //void draw(const char *lines[], int iLines, int iFirstRow, int iFirstCol=0);
  /** 
   * API used to draw the CommandInterpreter
   */  
  void draw(const char *pLabel, char cSlideSpeed, char cPanSpeed, char cTiltSpeed, word wRemainingSecs);
  
  boolean getAndDispatchKey(unsigned long now);

  /** Derived class will overwrite these.  Do nothing by default */
  virtual void onKeyDown(int vk);
  virtual void onKeyUp(int vk);

  /** */
  void setAutoRepeat(bool bAutoRepeat) {
    m_bAutoRepeat = bAutoRepeat;
  }

  /** public API */
  char getSelectedChannel() {
    return m_cSelectedChannel;
  }
private:
  void onLeftRightKeyDown(char iSpeed);
  void onUpDownKeyDown(char iSecs);
  
  /** when to fire autorepeat */
  unsigned long m_ulToFireAutoRepeat; 
  /** autorepeat? */
  bool m_bAutoRepeat = false;
  /** get one of VK_xxx */
  char getKey();

  char m_cOldKey;
  /** 
   * Channel selected in the GUI.  
   * Should be one of cmdSlide, cmdPan or cmdTilt or cmdNone
   */
  char m_cSelectedChannel;
};

#endif
