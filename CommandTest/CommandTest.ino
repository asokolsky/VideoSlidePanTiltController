#include <LiquidCrystal.h>

#include "LcdKeypadShield.h"
#include "CommandInterpreter.h"
#include "SerialCommand.h"
#include "Views.h"


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
 * Globals: slider end switch counter
 */
volatile byte g_byteSliderEndSwitch = 0;

/**
 * Globals: Main command interpreter
 */
CommandInterpreter g_ci(
 26, 27, 3, // slider IN1, IN2, PWM
 //26, 27, 2, // slider IN1, IN2, PWM
 22, 23, 11, // pan IN1, IN2, PWM
 24, 25, 12); // tilt IN1, IN2, PWM

/**
 * Globals: commands to run at startup
 */
static Command cmds[] = {

//  {cmdRest,  0, 1000},  // rest for 1 sec

  {cmdBeginLoop, 0, 0},
    {cmdSlide, -91, 10000}, // go left for 40 sec or endswitch
    {cmdWaitForCompletion,  0, 0},  // 
    {cmdRest,  0, 5000},  // Rest for 5 sec - end of wait will interrupt slide!
    {cmdSlide, 89, 10000}, // go right for 40 sec or endswitch
    {cmdWaitForCompletion,  0, 0},  // 
    {cmdRest,  0, 5000},  // rest for 5 sec
    /*  
    {cmdSlide, -20, 20000}, // go faster left for 2 sec
  
    {cmdPan, 10, 15000}, // pan right for 30 sec
    {cmdSlide, -10, 10000}, // go left for 1 sec
    {cmdPan, -20, 15000}, // pan right for 30 sec
    {cmdWaitForCompletion, 0, 0},
    {cmdTilt, 20, 10000}, // tilt up for 10 sec
    {cmdRest,  0, 10000},  // rest for 10 sec
    
    {cmdTilt, -10, 12000}, // tilt down for 1 sec
    */  
  {cmdEndLoop, 0, 0},
  {cmdNone,  0, 0}
};

/**
 * Globals: Serial Port object
 */
MySerialCommand g_sci;

static void onSliderEndSwitch()
{
  g_byteSliderEndSwitch++;
}

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while(!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  g_pView = &g_viewChannels;
  
  // internal pull-up resistor
  //digitalWrite (3, HIGH);
  digitalWrite (2, HIGH);
  // interrupt 1 is on pin 3
  //attachInterrupt(1, onSliderEndSwitch, FALLING);
  // interrupt 0 is on pin 3
  attachInterrupt(0, onSliderEndSwitch, FALLING);

  g_sci.begin();
  g_lcd.begin();
  g_ci.begin();
  g_ci.beginRun(cmds);
}

void loop()
{
  /**
  DEBUG_PRINT("g_byteSliderEndSwitch=");
  DEBUG_PRINTDEC(g_byteSliderEndSwitch);
  DEBUG_PRINTLN("");
  */
  unsigned long now = millis();
  if(g_ci.isRunning()) {
    if(g_ci.continueRun(now)){
      ;
    } else {
      g_ci.endRun();
    }
  }
  if(g_lcd.getAndDispatchKey(now)) {
    ;
  } else if(g_sci.available()) {
    do {
      g_sci.readAndDispatch();
      g_ci.updateDisplay(now);
    } while(g_sci.available());
  }
  delay(50);
}

