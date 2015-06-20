#ifndef Keypad_h
#define Keypad_h

/********************************************************************************************/
/* Library for the common LCD & keypad Arduino shield                                       */
/* http://www.dfrobot.com/wiki/index.php?title=Arduino_LCD_KeyPad_Shield_%28SKU:_DFR0009%29 */
/* Uses digital pins 4-9,10                                                                 */
/* Provides for kb input, drawing primitives, etc                                           */
/*                                                                                          */
/* Alex Sokolsky, May 2015                                                                  */
/*                                                                                          */
/********************************************************************************************/

#include <Arduino.h>

/** define some values for button (key) scan codes */
const char VK_NONE = 0;
const char VK_RIGHT = 1;
const char VK_UP = 2;
const char VK_DOWN = 3;
const char VK_LEFT = 4;
const char VK_SEL = 5;

class Keypad {
  /** delay in ms before the long key is fired */
  static const int s_iLongKeyDelay = 1000;

public:
  /** keypad is on this analog pin */
  Keypad(byte bPin) : m_bPin(bPin)
  {    
  }

  boolean getAndDispatchKey(unsigned long now);

private:
  byte m_bPin;
  /** when to fire long key */
  unsigned long m_ulToFireLongKey;
  /** get one of VK_xxx */
  byte getKey();

  byte m_cOldKey = VK_NONE;
};

extern Keypad g_kpad;


#endif
