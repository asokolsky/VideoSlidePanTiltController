#include "Lcd1602KeypadShield.h"
#include "CommandInterpreter.h"

extern Lcd1602KeypadShield g_lcd;

/**
 * Generic CommandInterpreterChannel implementation 
 */
void CommandInterpreterChannel::beginCommands() {
  m_cSpeed = 0;
  m_ulNext = 0;
}
void CommandInterpreterChannel::endCommands() {
}
void CommandInterpreterChannel::saveCommand(char cSpeed, unsigned long ulDuration, unsigned long now) {
  m_cSpeed = cSpeed;
  m_ulNext = now + ulDuration;
}

static char sanitizeSpeed(char cSpeed) {
  return constrain(cSpeed, -100, 100);
}

void CommandInterpreterChannel::adjustCommandSpeed(char cSpeedAdjustment) {
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
  endCommand();
  beginCommand(cSpeed, ulNext-now, now);
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

/**
 * slider implementation
 */
void SlideCommandInterpreterChannel::beginCommand(char cSpeed, unsigned long ulDuration, unsigned long now) {
  DEBUG_PRINT("SlideCommandInterpreterChannel::beginCommand cSpeed=");
  DEBUG_PRINTDEC(cSpeed);
  DEBUG_PRINT(", ulDuration=");
  DEBUG_PRINTDEC(ulDuration);
  DEBUG_PRINTLN("");

  saveCommand(cSpeed, ulDuration, now);

  // work with slider hardware here
  m_motor.setSpeed((cSpeed >= 0) ? cSpeed : -cSpeed);
  m_motor.setDirection(cSpeed >= 0);
  m_motor.go();
}
boolean SlideCommandInterpreterChannel::endCommand() {
  DEBUG_PRINTLN("SlideCommandInterpreterChannel::endCommand()");
  saveCommand(0, 0, 0);

  // work with slider hardware here
  m_motor.stop();
}

/**
 * panner implementation
 */
void PanCommandInterpreterChannel::beginCommand(char cSpeed, unsigned long ulDuration, unsigned long now) {
  DEBUG_PRINT("PanCommandInterpreterChannel::beginCommand cSpeed=");
  DEBUG_PRINTDEC(cSpeed);
  DEBUG_PRINT(", ulDuration=");
  DEBUG_PRINTDEC(ulDuration);
  DEBUG_PRINTLN("");

  saveCommand(cSpeed, ulDuration, now);
  // work with pan hardware here
  m_motor.setSpeed((cSpeed >= 0) ? cSpeed : -cSpeed);
  m_motor.setDirection(cSpeed >= 0);
  m_motor.go();
}
boolean PanCommandInterpreterChannel::endCommand() {
  DEBUG_PRINTLN("PanCommandInterpreterChannel::endCommand()");
  saveCommand(0, 0, 0);
  // work with slider hardware here
  m_motor.stop();
}

/**
 * tilter implementation
 */
void TiltCommandInterpreterChannel::beginCommand(char cSpeed, unsigned long ulDuration, unsigned long now) {
  DEBUG_PRINT("TiltCommandInterpreterChannel::beginCommand cSpeed=");
  DEBUG_PRINTDEC(cSpeed);
  DEBUG_PRINT(", ulDuration=");
  DEBUG_PRINTDEC(ulDuration);
  DEBUG_PRINTLN("");

  saveCommand(cSpeed, ulDuration, now);
  // work with tilt hardware here
  m_motor.setSpeed((cSpeed >= 0) ? cSpeed : -cSpeed);
  m_motor.setDirection(cSpeed >= 0);
  m_motor.go();
}
boolean TiltCommandInterpreterChannel::endCommand() {
  DEBUG_PRINTLN("TiltCommandInterpreterChannel::endCommand()");
  saveCommand(0, 0, 0);
  // work with slider hardware here
  m_motor.stop();
}

/**
 * CommandInterpreter implementation
 */
CommandInterpreter::CommandInterpreter(
  byte pinSlideCW, byte pinSlideCCW, byte pinSlidePWM,
  byte pinPanCW, byte pinPanCCW, byte pinPanPWM,
  byte pinTiltCW, byte pinTiltCCW, byte pinTiltPWM) : 
  m_pCommand(0), m_ulNext(0), m_bWaitingForCompletion(false)
{
  m_channels[cmdSlide] = new SlideCommandInterpreterChannel(pinSlideCW, pinSlideCCW, pinSlidePWM);
  m_channels[cmdPan] = new PanCommandInterpreterChannel(pinPanCW, pinPanCCW, pinPanPWM);
  m_channels[cmdTilt] = new TiltCommandInterpreterChannel(pinTiltCW, pinTiltCCW, pinTiltPWM);
}

//
// buffer for a sequence of one command
//
static Command cmds[] = {
  {cmdNone, 0, 0},
  {cmdNone,  0, 0}
};

/** external API */
void CommandInterpreter::beginRun(char cmd, char cSpeed, unsigned long ulDuration) {
  DEBUG_PRINTLN("CommandInterpreter::beginRun");
  cmds[0].m_command = cmd;
  cmds[0].m_speed = cSpeed;
  cmds[0].m_uDuration = ulDuration;
  beginRun(cmds);
}

void CommandInterpreter::beginRun(Command *p) {
  DEBUG_PRINTLN("CommandInterpreter::beginRun");
  // beginCommands for all channels.....
  for(char i = 0; i < sizeof(m_channels)/sizeof(m_channels[0]); i++)
    if(m_channels[i] != 0)
      m_channels[i]->beginCommands();
  m_bWaitingForCompletion = false;
  beginCommand(p, millis());
}

void CommandInterpreter::endRun() {
  DEBUG_PRINTLN("CommandInterpreter::endRun");
  m_bWaitingForCompletion = false;
  m_pCommand = 0;
  m_ulNext = 0;
  for(char i = 0; i < sizeof(m_channels)/sizeof(m_channels[0]); i++)
    if(m_channels[i] != 0)
      m_channels[i]->endCommands();
  g_lcd.clear();
  g_lcd.setCursor(0, 0);
  g_lcd.print("Done!");
}

/** force stop processing commands */
void CommandInterpreter::stopRun() {
  DEBUG_PRINTLN("CommandInterpreter::stopRun");
  m_bWaitingForCompletion = false;
  m_pCommand = 0;
  m_ulNext = 0;
  
  for(char i = 0; i < sizeof(m_channels)/sizeof(m_channels[0]); i++)
    if((m_channels[i] != 0) && m_channels[i]->isBusy())
      m_channels[i]->endCommand();
  g_lcd.clear();
  g_lcd.setCursor(0, 0);
  g_lcd.print("Stopped!");
}

/** 
 * Times tick, update interpreter status
 * return true to continue running
 * return false to end the run
 */
bool CommandInterpreter::continueRun(unsigned long now) 
{
  /*DEBUG_PRINT("CommandInterpreter::continueRun now=");
  DEBUG_PRINTDEC(now);
  DEBUG_PRINTLN("");
*/
  if(m_pCommand == 0)
    return false;
  // update the display at least once a sec
  bool bUpdateDisplay = ((now - m_ulLastDisplayUpdate) > 900);
  // end commands in any of the channels if it is time to do so
  for(char i = 0; i < sizeof(m_channels)/sizeof(m_channels[0]); i++) {
    if(m_channels[i] == 0)
      continue;
    if(m_channels[i]->isReadyToEndCommand(now)) {
      m_channels[i]->endCommand();
      bUpdateDisplay = true;
    }
  }
  if(bUpdateDisplay)
    updateDisplay(now);
  if(m_pCommand->m_command == cmdNone) {
    // wait till all the commands are completed
    return (getBusyChannels() > 0);
  } else if(isResting()) {
    //DEBUG_PRINTLN("CommandInterpreter::continueRun - isResting!");
    if(!isReadyToEndRest(now))
      return true;
    endRest();
  } else if(isWaitingForCompletion()) {
    //DEBUG_PRINTLN("CommandInterpreter::continueRun - isWaitingForCompletion!");
    if(getBusyChannels() > 0)
      return true;
    endWaitForCompletion();
  } else {
    //DEBUG_PRINTLN("CommandInterpreter::continueRun - not resting!");
    // is the channell needed for the next command already busy?
    Command *p = m_pCommand + 1;
    char cCmd = p->m_command;
    if(isBusy(cCmd))
        return true;
  } 
  // advance!
  m_pCommand++;
  // execute the command pointed to by m_pCommand
  beginCommand(m_pCommand, now);
  return true;
}

boolean CommandInterpreter::isBusy(char cChannel) {
  if(!isRunning())
    return false;
  return (0 <= cChannel) 
    && (cChannel < cmdMax) 
    && (m_channels[cChannel] != 0) 
    && m_channels[cChannel]->isBusy();
}

/** get the # of channels running command */
char CommandInterpreter::getBusyChannels() {
  char iRes = 0;
  for(char i = 0; i < sizeof(m_channels)/sizeof(m_channels[0]); i++)
    if((m_channels[i] != 0) && (m_channels[i]->isBusy()))
      iRes++;
  return iRes;
}


/** 
 * External API 
 */
void CommandInterpreter::beginCommand(char cmd, char cSpeed, unsigned long ulDuration) {
  DEBUG_PRINTLN("CommandInterpreter::beginCommand");
  cmds[0].m_command = cmd;
  cmds[0].m_speed = cSpeed;
  cmds[0].m_uDuration = ulDuration;
  beginCommand(cmds, millis());
}

/** 
 * execute the command pointed to by m_pCommand 
 */
void CommandInterpreter::beginCommand(Command *p, unsigned long now) {
  //DEBUG_PRINTLN("CommandInterpreter::beginCommand");
  m_pCommand = p;
  char iCmd = p->m_command;
  switch(iCmd) {
    case cmdRest:
      beginRest(p->m_uDuration, now);
      break;
    case cmdWaitForCompletion:
      beginWaitForCompletion();
      break;
    case cmdSlide:
    case cmdPan:
    case cmdTilt:
      if(m_channels[iCmd] == 0) {
        // we are not equiped to handle this command!  Ignore it!
        m_pCommand++;
        beginCommand(m_pCommand, now);
        return;
      }
      m_channels[iCmd]->beginCommand(p->m_speed, p->m_uDuration, now);
      break;
    //case cmdNone:
    //case cmdZoom:
    default:
      // melt the core here
      return;
  }
  updateDisplay(now);
}

void CommandInterpreter::adjustCommandSpeed(char iCmd, char iSpeed) {
  DEBUG_PRINTLN("CommandInterpreter::adjustCommandSpeed");
  switch(iCmd) {
    case cmdSlide:
    case cmdPan:
    case cmdTilt:
      if(m_channels[iCmd] != 0)
        m_channels[iCmd]->adjustCommandSpeed(iSpeed);
      break;
  }
}

void CommandInterpreter::adjustCommandDuration(char iCmd, int iSecs) {
  DEBUG_PRINTLN("CommandInterpreter::adjustCommandDuration");
  switch(iCmd) {
    case cmdRest:
    case cmdWaitForCompletion:
      stopRun();
      break;
    case cmdSlide:
    case cmdPan:
    case cmdTilt:
      if(m_channels[iCmd] != 0)
        m_channels[iCmd]->adjustCommandDuration(iSecs);
      break;
  }
}

void CommandInterpreter::stopCommand(char iCmd)
{
  if(iCmd < 0 || iCmd > sizeof(m_channels)/sizeof(m_channels[0]))
    return;
  if(m_channels[iCmd] == 0)
   return;
  m_channels[iCmd]->stopCommand(millis());
}

/** 
 * find out for how long the first busy channel will be busy
 */
word CommandInterpreter::getBusySeconds(unsigned long now) {
  word wSecsRes = 0;
  for(char i = 0; i < sizeof(m_channels)/sizeof(m_channels[0]); i++)
    if((m_channels[i] != 0) && m_channels[i]->isBusy()) {
      unsigned long ulNext = m_channels[i]->m_ulNext;
      if(now < ulNext) {
        word wSecs = (ulNext - now) / 1000L;
        if((wSecsRes == 0) || ((wSecs != 0) && (wSecs < wSecsRes)))
          wSecsRes = wSecs;
      }
    }
  return wSecsRes;
}

void CommandInterpreter::updateDisplay(unsigned long now) {
  const char *pLabel = 0;
  word wSecs = 0;
  char cSelectedChannel = g_lcd.getSelectedChannel();
  if((cmdFirst <= cSelectedChannel) 
    && (cSelectedChannel <= cmdLast) 
    && (m_channels[cSelectedChannel] != 0)) {
    unsigned long ulNext = m_channels[cSelectedChannel]->m_ulNext;
    if(now < ulNext)
      wSecs = (ulNext - now) / 1000L;
  }

  if(isResting()) {
    pLabel = "Rest";
    if((wSecs == 0) && (now < m_ulNext))
      wSecs = (m_ulNext - now) / 1000;
  } else if(isWaitingForCompletion()) {
    pLabel = "Wait";
    if(wSecs == 0)
      wSecs = getBusySeconds(now);
  } else {
    pLabel = "Stop";
    if(wSecs == 0)
      wSecs = getBusySeconds(now);
  }
  g_lcd.draw(pLabel, 
    (m_channels[cmdSlide]!= 0) ? m_channels[cmdSlide]->m_cSpeed : 0, 
    (m_channels[cmdPan]  != 0) ? m_channels[cmdPan]->m_cSpeed : 0, 
    (m_channels[cmdTilt] != 0) ? m_channels[cmdTilt]->m_cSpeed : 0, 
    wSecs);
  m_ulLastDisplayUpdate = now; 
}

