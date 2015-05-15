#include "Trace.h"
#include "CommandInterpreter.h"

extern volatile byte g_byteSliderEndSwitch;

/**
 * Generic CommandInterpreterChannel implementation 
 */
CommandInterpreterChannel::CommandInterpreterChannel(byte pinCW, byte pinCCW, byte pinPWM) : 
  m_motor(pinCW, pinCCW, pinPWM) 
{
  //m_motor.setSpeedRegulation(100);
}

void CommandInterpreterChannel::beginCommands() {
  m_cSpeed = m_cCurrentSpeed = 0;
  m_ulNext = 0;
}

void CommandInterpreterChannel::beginCommand(char cSpeed, unsigned long ulDuration, unsigned long now) {
  DEBUG_PRINT("CommandInterpreterChannel::beginCommand cSpeed=");
  DEBUG_PRINTDEC(cSpeed);
  DEBUG_PRINT(", ulDuration=");
  DEBUG_PRINTDEC(ulDuration);
  DEBUG_PRINTLN("");

  m_ulNext = now + ulDuration;
  m_cSpeed = cSpeed;
  m_cCurrentSpeed = 0;
  doSpeedStep(now);
}

boolean CommandInterpreterChannel::endCommand() {
  DEBUG_PRINTLN("CommandInterpreterChannel::endCommand()");

  // work with hardware here
  m_motor.stop();

  m_cSpeed = m_cCurrentSpeed = 0;
  m_ulNext = m_ulNextSpeedUpdate = 0;
}

/**
 * by default we are concerned about command expiration only, 
 * no end-switches are in the picture
 */
boolean CommandInterpreterChannel::isReadyToEndCommand(unsigned long now) {
  return (m_ulNext != 0) && (now >= m_ulNext);
}

void CommandInterpreterChannel::adjustCommandDuration(char iSecs) {
  DEBUG_PRINT("CommandInterpreterChannel::adjustCommandDuration iSecs=");
  DEBUG_PRINTDEC(iSecs);
  DEBUG_PRINT(", m_ulNext=");
  DEBUG_PRINTDEC(m_ulNext);
  DEBUG_PRINTLN("");

  if(!isBusy())
    return;
  if(iSecs > 0) {
    m_ulNext = m_ulNext + 1000l * (unsigned long)iSecs;
  } else {
    unsigned long now = millis();
    unsigned long ulNext = m_ulNext - 1000l * (unsigned long)(-iSecs);
    m_ulNext = (now >= ulNext) ? now : ulNext;
  }
  DEBUG_PRINT("m_ulNext=");
  DEBUG_PRINTDEC(m_ulNext);
  DEBUG_PRINTLN("");
}

static char sanitizeSpeed(char cSpeed) {
  return constrain(cSpeed, -100, 100);
}

void CommandInterpreterChannel::adjustCommandSpeed(char cSpeedAdjustment) {
  DEBUG_PRINT("CommandInterpreterChannel::adjustCommandSpeed cSpeedAdjustment=");
  DEBUG_PRINTDEC(cSpeedAdjustment);
  DEBUG_PRINTLN("");

  if(!isBusy())
    return;
  unsigned long now = millis();
  char cSpeed = sanitizeSpeed(m_cSpeed + cSpeedAdjustment);
  if(cSpeed == 0) {
    stopCommand(now);
    return;
  }
  if(cSpeed == m_cSpeed)
    return;
  unsigned long ulNext = m_ulNext;
  if(now >= ulNext)
    return;
       
  m_cSpeed = cSpeed;
  doSpeedStep(now);
}


void CommandInterpreterChannel::tick(unsigned long now) {
  if((m_ulNext == 0) || (m_ulNextSpeedUpdate == 0) || (now < m_ulNextSpeedUpdate))
    return;
  doSpeedStep(now);  
}

/** assume uCurrentSpeed > uSpeed */
static unsigned slowDown(unsigned uCurrentSpeed, unsigned uSpeed, unsigned uSpeedUpdateStep) {
  return (uCurrentSpeed > (uSpeedUpdateStep + uSpeed))
   ? (uCurrentSpeed - uSpeedUpdateStep)
   : uSpeed;
}

/** assume uCurrentSpeed < uSpeed */
static unsigned speedUp(unsigned uCurrentSpeed, unsigned uSpeed, unsigned uSpeedUpdateStep) {
  return ((uCurrentSpeed + uSpeedUpdateStep) < uSpeed)
   ? (uCurrentSpeed + uSpeedUpdateStep)
   : uSpeed;
}


/**
 * implement a trapezoidal velocity profile
 * communicate with hardware
 *
 * we are now at m_cCurrentSpeed
 * we need to be at m_cSpeed
 * adjust speed by cSpeedUpdateStep
 * given that in m_ulNext-now we need to be at 0
 */
void CommandInterpreterChannel::doSpeedStep(unsigned long now) { 
  // say when next time
  m_ulNextSpeedUpdate = now + ulSpeedUpdateTick;

  unsigned uCurrentSpeed = abs(m_cCurrentSpeed);
  unsigned uSpeed = abs(m_cSpeed);
  if(uSpeed < uCurrentSpeed) 
  {
    // slow down!
    uCurrentSpeed = slowDown(uCurrentSpeed, uSpeed, bSpeedUpdateStep);
  } 
  else if (uCurrentSpeed >= (bSpeedUpdateStep * ((m_ulNext-now)/ulSpeedUpdateTick)))
  {
    // faget about it!
    // start slowing down! the end is near!
    uSpeed = m_cSpeed = 0;
    uCurrentSpeed = slowDown(uCurrentSpeed, uSpeed, bSpeedUpdateStep);
  } 
  else if(uSpeed == uCurrentSpeed)
  {
    return;
  }
  else
  {
    // speed up!
    uCurrentSpeed = speedUp(uCurrentSpeed, uSpeed, bSpeedUpdateStep);
  }
  m_cCurrentSpeed = (m_cCurrentSpeed > 0) ? uCurrentSpeed : -uCurrentSpeed;


  // work with the hardware here!
  // this will start the motor spinning!
  DEBUG_PRINT("m_motor.setSpeed ");
  DEBUG_PRINTDEC(uCurrentSpeed);
  DEBUG_PRINT(" now=");
  DEBUG_PRINTDEC(now);
  DEBUG_PRINTLN("");

  m_motor.setSpeed(m_cCurrentSpeed >= 0, uCurrentSpeed);
}



/**
 * slider implementation
 */
void SlideCommandInterpreterChannel::beginCommand(char cSpeed, unsigned long ulDuration, unsigned long now) {
  DEBUG_PRINT("SlideCommandInterpreterChannel::beginCommand cSpeed=");
  DEBUG_PRINTDEC(cSpeed);
  DEBUG_PRINT(", ulDuration=");
  DEBUG_PRINTDEC(ulDuration);
  DEBUG_PRINTLN("");

  m_bEndSwitchCounter = g_byteSliderEndSwitch;
  CommandInterpreterChannel::beginCommand(cSpeed, ulDuration, now);  
}

boolean SlideCommandInterpreterChannel::endCommand() {
  DEBUG_PRINTLN("SlideCommandInterpreterChannel::endCommand()");
  CommandInterpreterChannel::endCommand();
}

/** 
 * Slider has an end-switch!  Check it!
 */
boolean SlideCommandInterpreterChannel::isReadyToEndCommand(unsigned long now) {
  if(m_ulNext == 0)
    return false;
  // did we hit an endswitch yet?
  if(m_bEndSwitchCounter != g_byteSliderEndSwitch) {
    DEBUG_PRINT("SlideCommandInterpreterChannel::isReadyToEndCommand detected interrupt, g_byteSliderEndSwitch=");
    DEBUG_PRINTDEC((int)g_byteSliderEndSwitch);
    DEBUG_PRINTLN("");
    m_bEndSwitchCounter = g_byteSliderEndSwitch;
    m_ulNext = now;
    return true;
  }
  return CommandInterpreterChannel::isReadyToEndCommand(now);
}


