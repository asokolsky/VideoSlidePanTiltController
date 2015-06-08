/********************************************************************************************/
/* Library for the common LCD & keypad Arduino shield                                       */
/* http://www.dfrobot.com/wiki/index.php?title=Arduino_LCD_KeyPad_Shield_%28SKU:_DFR0009%29 */
/* Uses digital pins 4-9,10                                                                 */
/* Provides for kb input, drawing primitives, etc                                           */
/*                                                                                          */
/* Alex Sokolsky, May 2015                                                                  */
/*                                                                                          */
/********************************************************************************************/

#ifndef LcdKeypadShield_h
#define LcdKeypadShield_h

#include <Arduino.h>
#include <LiquidCrystal.h>

/** define some values for button (key) scan codes */
const char VK_NONE = 0;
const char VK_RIGHT= 1;
const char VK_UP   = 2;
const char VK_DOWN = 3;
const char VK_LEFT = 4;
const char VK_SEL  = 5;

class LcdKeypadShield : public LiquidCrystal 
{
public:
   /** LCD geometry */
  static const char s_iRows = 2;
  static const char s_iCols = 16;
  
  /** delay in ms before the long key is fired */ 
  static const int s_iLongKeyDelay = 1000;

  LcdKeypadShield() : LiquidCrystal(8, 9, 4, 5, 6, 7) {    
  }
  
  void begin() {
    LiquidCrystal::begin(s_iCols, s_iRows);
  }

  /** 
   * API used to draw the CommandInterpreter
   */  
  void draw(const char *pLabel, char cSlideSpeed, char cPanSpeed, char cTiltSpeed, word wRemainingSecs);
  
  boolean getAndDispatchKey(unsigned long now);

private:
  
  /** when to fire long key */
  unsigned long m_ulToFireLongKey; 
  /** get one of VK_xxx */
  byte getKey();

  byte m_cOldKey = VK_NONE;
};

extern LcdKeypadShield g_lcd;


#endif
