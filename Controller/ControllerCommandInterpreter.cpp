#include <LiquidCrystal.h>
#include <VideoSlidePanTiltController.h>
#include "ControllerCommandInterpreter.h"

extern LiquidCrystal g_lcd;


/**
 * CommandInterpreter implementation
 */
ControllerCommandInterpreter::ControllerCommandInterpreter(
  byte pinSlideCW, byte pinSlideCCW, byte pinSlidePWM,
  byte pinPanCW, byte pinPanCCW, byte pinPanPWM,
  byte pinTiltCW, byte pinTiltCCW, byte pinTiltPWM)
{
  m_channels[cmdSlide] = new SlideCommandInterpreterChannel(pinSlideCW, pinSlideCCW, pinSlidePWM);
  m_channels[cmdPan] = new /*Pan*/ CommandInterpreterChannel(pinPanCW, pinPanCCW, pinPanPWM);
  m_channels[cmdTilt] = new /*Tilt*/ CommandInterpreterChannel(pinTiltCW, pinTiltCCW, pinTiltPWM);
}

/** 
 * external APIs 
 */
void ControllerCommandInterpreter::begin() {
  // begin for all channels.....
  for(char i = 0; i < sizeof(m_channels)/sizeof(m_channels[0]); i++)
    if(m_channels[i] != 0)
      m_channels[i]->begin();
}

//
// buffer for a sequence of one command
//
static Command cmds[] = {
  {cmdNone, 0, 0},
  {cmdNone,  0, 0}
};

void ControllerCommandInterpreter::beginRun(char cmd, char cSpeed, unsigned long ulDuration) {
  DEBUG_PRINTLN("CommandInterpreter::beginRun");
  cmds[0].m_command = cmd;
  cmds[0].m_speed = cSpeed;
  cmds[0].m_uDuration = ulDuration;
  beginRun(cmds);
}

void ControllerCommandInterpreter::beginRun(Command *p) {
  DEBUG_PRINTLN("CommandInterpreter::beginRun");
  // beginCommands for all channels.....
  for(char i = 0; i < sizeof(m_channels)/sizeof(m_channels[0]); i++)
    if(m_channels[i] != 0)
      m_channels[i]->beginCommands();

  m_bWaitingForCompletion = false;
  beginCommand(p, millis());
}

void ControllerCommandInterpreter::endRun() {
  DEBUG_PRINTLN("CommandInterpreter::endRun");
  m_bWaitingForCompletion = false;
  m_pCommand = 0;
  m_ulNext = 0;

  for(char i = 0; i < sizeof(m_channels)/sizeof(m_channels[0]); i++)
    if(m_channels[i] != 0)
      m_channels[i]->endCommands();
  /*    
  g_lcd.clear();
  g_lcd.setCursor(0, 0);
  g_lcd.print("Done!");
  */
}

/** force stop processing commands */
void ControllerCommandInterpreter::stopRun() {
  DEBUG_PRINTLN("CommandInterpreter::stopRun");
  m_bWaitingForCompletion = false;
  m_pCommand = 0;
  m_ulNext = m_ulPaused = 0;
  
  for(char i = 0; i < sizeof(m_channels)/sizeof(m_channels[0]); i++)
    if((m_channels[i] != 0) && m_channels[i]->isBusy())
      m_channels[i]->endCommand();
  /*    
  g_lcd.clear();
  g_lcd.setCursor(0, 0);
  g_lcd.print("Stopped!");
  */
}


/** suspend the run, can resume */
void ControllerCommandInterpreter::pauseRun() {
  if(m_ulPaused != 0)
    return;
  m_ulPaused = millis();
  for(char i = 0; i < sizeof(m_channels)/sizeof(m_channels[0]); i++)
    if((m_channels[i] != 0) && m_channels[i]->isBusy())
      m_channels[i]->pauseCommand();
  updateDisplay(m_ulPaused);
}

/** resume the run */
void ControllerCommandInterpreter::resumeRun() {
  if(m_ulPaused == 0)
    return;
  unsigned long now = millis();
  unsigned long ulPauseDuration = now - m_ulPaused;
  for(char i = 0; i < sizeof(m_channels)/sizeof(m_channels[0]); i++)
    if((m_channels[i] != 0) && m_channels[i]->isBusy())
      m_channels[i]->resumeCommand(ulPauseDuration);
  m_ulNext += ulPauseDuration;
  m_ulPaused = 0;
  updateDisplay(now);
}


/** 
 * Times tick, update interpreter status
 * return true to continue running
 * return false to end the run
 */
bool ControllerCommandInterpreter::continueRun(unsigned long now) 
{
/**
  DEBUG_PRINT("CommandInterpreter::continueRun now=");
  DEBUG_PRINTDEC(now);
  DEBUG_PRINTLN("");
*/
  if(m_pCommand == 0) {
    DEBUG_PRINTLN("CommandInterpreter::continueRun m_pCommand==0");
    return false;
  }
  if(isPaused())
    return true;
  // update the display at least once a sec
  bool bUpdateDisplay = ((now - m_ulLastDisplayUpdate) > 900);
  //
  // for all channels...
  // tick thus giving an opportunity to do trapezoidal velocity profile...
  // end command if it is time to do so
  //
  for(char i = 0; i < sizeof(m_channels)/sizeof(m_channels[0]); i++) {
    if(m_channels[i] == 0)
      continue;
    if(m_channels[i]->isReadyToEndCommand(now)) {
      m_channels[i]->endCommand();
      bUpdateDisplay = true;
    } else {
      m_channels[i]->tick(now);
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
  // execute next command pointed to by m_pCommand
  beginCommand(m_pCommand+1, now);
  return true;
}

boolean ControllerCommandInterpreter::isBusy(char cChannel) {
  if(!isRunning())
    return false;
  return (0 <= cChannel) 
    && (cChannel < cmdMax) 
    && (m_channels[cChannel] != 0) 
    && m_channels[cChannel]->isBusy();
}

/** get the # of channels running command */
char ControllerCommandInterpreter::getBusyChannels() {
  char iRes = 0;
  for(char i = 0; i < sizeof(m_channels)/sizeof(m_channels[0]); i++)
    if((m_channels[i] != 0) && (m_channels[i]->isBusy()))
      iRes++;
  return iRes;
}


/** 
 * execute the command pointed to by p
 * side effect - may change m_pCommand 
 */
void ControllerCommandInterpreter::beginCommand(Command *p, unsigned long now) {
  //DEBUG_PRINTLN("CommandInterpreter::beginCommand");
  
  for(bool bContinue = true; bContinue;) 
  {
    m_pCommand = p;
    char iCmd = p->m_command;
    switch(iCmd) {
      case cmdRest:
        beginRest(p->m_uDuration, now);
        bContinue = false;
        break;
      case cmdWaitForCompletion:
        beginWaitForCompletion();
        bContinue = false;
        break;
      case cmdSlide:
      case cmdPan:
      case cmdTilt:
      //case cmdZoom:
        if(m_channels[iCmd] == 0) {
          // we are not equiped to handle this command!  Ignore it!
          p++;
        } else {
          m_channels[iCmd]->beginCommand(p->m_speed, p->m_uDuration, now);
          bContinue = false;
        }
        break;
      case cmdBeginLoop:
        beginLoop(p);
        p++;
        break;
      case cmdEndLoop:
        p = endLoop();
        break;
      //case cmdNone:
      default:
        // melt the core here
        return;
    }
  }
  updateDisplay(now);
}

void ControllerCommandInterpreter::beginWaitForCompletion() {
  DEBUG_PRINTLN("CommandInterpreter::beginWaitForCompletion");
  m_bWaitingForCompletion = true;
}
void ControllerCommandInterpreter::endWaitForCompletion() {
  DEBUG_PRINTLN("CommandInterpreter::endWaitForCompletion");
  m_bWaitingForCompletion = false;
}

void ControllerCommandInterpreter::beginRest(unsigned long ulDuration, unsigned long now) {
  DEBUG_PRINT("CommandInterpreter::beginRest ulDuration=");
  DEBUG_PRINTDEC(ulDuration);
  DEBUG_PRINTLN("");
  m_ulNext = now + ulDuration;
}
void ControllerCommandInterpreter::endRest() {
  DEBUG_PRINTLN("CommandInterpreter::endRest()");
  m_ulNext = 0;
}
void ControllerCommandInterpreter::beginLoop(Command *p) {
  DEBUG_PRINT("CommandInterpreter::beginLoop p=0x");
  DEBUG_PRINTHEX((unsigned)p);
  DEBUG_PRINT(", p->m_command=");
  DEBUG_PRINTDEC(p->m_command);
  DEBUG_PRINTLN("");
  // the following is necessary because
  m_pBeginLoopCommand = p;
}

Command *ControllerCommandInterpreter::endLoop() {
  Command *p = m_pBeginLoopCommand;
  m_pBeginLoopCommand = 0;
  DEBUG_PRINT("CommandInterpreter::endLoop p=0x");
  DEBUG_PRINTHEX((unsigned)p);
  DEBUG_PRINT(", p->m_command=");
  DEBUG_PRINTDEC(p->m_command);
  DEBUG_PRINTLN("");
  return p;
}


/** 
 * External API 
 */
void ControllerCommandInterpreter::beginCommand(char cmd, char cSpeed, unsigned long ulDuration) {
  DEBUG_PRINTLN("CommandInterpreter::beginCommand");
  cmds[0].m_command = cmd;
  cmds[0].m_speed = cSpeed;
  cmds[0].m_uDuration = ulDuration;
  beginCommand(cmds, millis());
}

void ControllerCommandInterpreter::adjustCommandSpeed(char iCmd, char iSpeed) {
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

void ControllerCommandInterpreter::adjustCommandDuration(char iCmd, int iSecs) {
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

/** 
 * iCmd is actually a channel # 
 * to be called from interrupt handler or in response to kb
 */
void ControllerCommandInterpreter::stopCommand(char iCmd) {
  if(iCmd < 0 || iCmd > sizeof(m_channels)/sizeof(m_channels[0]))
    return;
  if(m_channels[iCmd] == 0)
   return;
  m_channels[iCmd]->stopCommand(millis());
}

/** 
 * find out for how long the first busy channel will be busy
 */
word ControllerCommandInterpreter::getBusySeconds(unsigned long now) {
  word wSecsRes = 0;
  for(char i = 0; i < sizeof(m_channels)/sizeof(m_channels[0]); i++)
    if((m_channels[i] != 0) && m_channels[i]->isBusy()) {
      unsigned long ulNext = m_channels[i]->getNext();
      if(now < ulNext) {
        word wSecs = (ulNext - now) / 1000L;
        if((wSecsRes == 0) || ((wSecs != 0) && (wSecs < wSecsRes)))
          wSecsRes = wSecs;
      }
    }
  return wSecsRes;
}

void ControllerCommandInterpreter::updateDisplay(unsigned long now) {
  const char *pLabel = 0;
  word wSecs = 0;
  byte cSelectedChannel = g_pView->s_cSelectedChannel;
  if((cmdFirst <= cSelectedChannel) 
    && (cSelectedChannel <= cmdLast) 
    && (m_channels[cSelectedChannel] != 0)) {
    unsigned long ulNext = m_channels[cSelectedChannel]->getNext();
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
  } else if(isPaused()) {
    pLabel = "Resu";
    // if(wSecs == 0) wSecs = getBusySeconds(now);
  } else {
    pLabel = "Stop";
    if(wSecs == 0)
      wSecs = getBusySeconds(now);
  }
  ScreenBuffer sb;
  g_pView->draw(pLabel, 
    m_channels[cmdSlide]->getCurrentSpeed(), 
    m_channels[cmdPan]->getCurrentSpeed(), 
    m_channels[cmdTilt]->getCurrentSpeed(), 
    wSecs,
    &sb);
  sb.draw(&g_lcd);
  // send it to remote over the network here...

  m_ulLastDisplayUpdate = now; 
}


