#include <LiquidCrystal.h>
#include "Lcd1602KeypadShield.h"
#include "CommandInterpreter.h"

Lcd1602KeypadShield g_lcd;

CommandInterpreter g_ci(
 26, 27, 3, // slider IN1, IN2, PWM
 22, 23, 11, // pan IN1, IN2, PWM
 24, 25, 12); // tilt IN1, IN2, PWM

Command cmds[] = {

//  {cmdRest,  0, 1000},  // rest for 1 sec
  {cmdSlide, -10, 10000}, // go left for 1 sec
  {cmdSlide, -20, 20000}, // go faster left for 2 sec
  {cmdRest,  0, 10000},  // rest for 10 sec

  {cmdPan, 10, 15000}, // pan right for 30 sec
  {cmdSlide, -10, 10000}, // go left for 1 sec
  {cmdPan, -20, 15000}, // pan right for 30 sec
  {cmdWaitForCompletion, 0, 0},
  {cmdTilt, 20, 10000}, // tilt up for 10 sec
  {cmdRest,  0, 10000},  // rest for 10 sec
  
  {cmdTilt, -10, 12000}, // tilt down for 1 sec
  {cmdNone,  0, 0}
};



void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while(!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  g_lcd.begin();
  g_ci.beginRun(cmds);
}

void loop()
{
  unsigned long now = millis();
  if(g_ci.isRunning()) {
    if(g_ci.continueRun(now)){
      ;
    } else {
      g_ci.endRun();
    }
  }
  if(g_lcd.getAndDispatchKey(now))
    ;  
  delay(100);
}

