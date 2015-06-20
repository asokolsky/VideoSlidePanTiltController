#include <VideoSlidePanTiltController.h>
#include "RemoteCommandInterpreter.h"

void RemoteCommandInterpreter::begin()
{
}

void RemoteCommandInterpreter::beginRun(char cmd, char iSpeed, unsigned long ulDuration)
{
}


/** external API of this class */
void RemoteCommandInterpreter::beginCommand(char cmd, char cSpeed, unsigned long ulDuration)
{
}

/** adjust speed */
void RemoteCommandInterpreter::adjustCommandSpeed(char cmd, char cSpeedAdjustment)
{
}

/** Duration adjustment in seconds */
void RemoteCommandInterpreter::adjustCommandDuration(char cmd, int iDurationAdjustment)
{
}


void RemoteCommandInterpreter::stopRun()
{
}

boolean RemoteCommandInterpreter::isRunning()
{

}

boolean RemoteCommandInterpreter::isPaused()
{
}

/** is this channel busy? */
boolean RemoteCommandInterpreter::isBusy(char cChannel)
{
}


/** suspend the run, can resume */
void RemoteCommandInterpreter::pauseRun()
{
}

/** resume the run */
void RemoteCommandInterpreter::resumeRun()
{
}


/**
* iCmd is actually a channel #
* to be called from interrupt handler or in response to kb
*/
void RemoteCommandInterpreter::stopCommand(char cCmd)
{
}

void RemoteCommandInterpreter::updateDisplay(unsigned long now)
{
  const char *pLabel = 0;
  word wSecs = 0;
  byte cSelectedChannel = g_pView->s_cSelectedChannel;
  /*
  if((cmdFirst <= cSelectedChannel) 
    && (cSelectedChannel <= cmdLast) 
    && (m_channels[cSelectedChannel] != 0)) {
    unsigned long ulNext = m_channels[cSelectedChannel]->getNext();
    if(now < ulNext)
      wSecs = (ulNext - now) / 1000L;
  }*/

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
  g_pView->draw(pLabel, 
    (m_channels[cmdSlide]!= 0) ? m_channels[cmdSlide]->getCurrentSpeed() : 0, 
    (m_channels[cmdPan]  != 0) ? m_channels[cmdPan]->getCurrentSpeed() : 0, 
    (m_channels[cmdTilt] != 0) ? m_channels[cmdTilt]->getCurrentSpeed() : 0, 
    wSecs);
  m_ulLastDisplayUpdate = now; 
}




