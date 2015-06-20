#include <SPI.h>
#include <RH_RF69.h>

#include <LiquidCrystal.h>
#include <VideoSlidePanTiltController.h>
#include "RemoteCommandInterpreter.h"
#include "Radio.h"

/**
 * Globals: the LCD screen/keypad  object
 */
LcdKeypadShield g_lcd;
/**
 * Globals: views and a pointer to a currently selected one.
 */
ChannelsView g_viewChannels;
ChannelView g_viewChannel;
char View::s_cSelectedChannel = cmdSlide;
View *g_pView;

/**
 * Globals: the object to communicate with the Controller
 */
static RemoteCommandInterpreter g_ci;
CommandInterpreter *g_pCommandInterpreter = &g_ci;

/**
 * Globals: the object to communicate with the Controller
 */
Radio g_radio;

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  
  g_lcd.begin(20, 4);  
  g_radio.begin();
  g_ci.begin();
}

void loop()
{
  /**
  DEBUG_PRINT("g_byteSliderEndSwitch=");
  DEBUG_PRINTDEC(g_byteSliderEndSwitch);
  DEBUG_PRINTLN("");
  */
  unsigned long now = millis();
  if(g_lcd.getAndDispatchKey(now)) 
  {
    ;
  } 
  if(g_radio.available())
  {
    g_radio.getAndDispatch();
    
  }
/*
  if(g_sci.available()) {
    do {
      g_sci.readAndDispatch();
      g_ci.updateDisplay(now);
    } while(g_sci.available());
  }
*/

  delay(100);
}

