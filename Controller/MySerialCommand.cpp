#include <VideoSlidePanTiltController.h>
#include "SerialCommand.h"
#include "ControllerCommandInterpreter.h"

/**
 * execute serial command to move (slide/pan/tilt)
 */
static void onSerialMoveCommand(char iCmd, char iSpeed, unsigned uDurationSecs) {
  if(g_pCommandInterpreter->isRunning()) {
    if(g_pCommandInterpreter->isBusy(iCmd))
      g_pCommandInterpreter->stopCommand(iCmd);
    if((iSpeed != 0) && (uDurationSecs > 0))
      g_pCommandInterpreter->beginCommand(iCmd, iSpeed, uDurationSecs*1000L);
  } else {
    if((iSpeed != 0) && (uDurationSecs > 0))
      g_pCommandInterpreter->beginRun(iCmd, iSpeed, uDurationSecs*1000L);
  }
}

/**
 * get the command args and pass on to execute it
 */
static void onSerialMoveCommand(char iCmd) {
  char *pArg = g_sci.next();
  if(pArg == 0)
    return;     // wrong command format
  char iSpeed = (char)atoi(pArg);
  unsigned uDurationSecs = 30;
  pArg = g_sci.next();
  if(pArg != 0)
    uDurationSecs = atoi(pArg);
  onSerialMoveCommand(iCmd, iSpeed, uDurationSecs);
}

/**
 * expected: SLI <signed speed in %> [duration in unsigned secs]
 * speed or duration 0 will result in stopping the slide
 */
static void onSerialSlide()
{
  onSerialMoveCommand(cmdSlide);
}

static void onSerialPan()
{
  onSerialMoveCommand(cmdPan);
}
static void onSerialTilt()
{
  onSerialMoveCommand(cmdTilt);
}

/**
 * also purge serial command queue if any!
 */
static void onSerialHalt()
{
  if(g_pCommandInterpreter->isRunning())
    g_pCommandInterpreter->stopRun();
}
static void onSerialPause()
{
  if(g_pCommandInterpreter->isRunning())
    g_pCommandInterpreter->pauseRun();
}
static void onSerialResume()
{
  if(g_pCommandInterpreter->isRunning() && g_pCommandInterpreter->isPaused())
    g_pCommandInterpreter->resumeRun();
}
static void onSerialWaitForCompletion()
{
}
static void onSerialRest()
{
}
static void onSerialBeginLoop()
{
}
static void onSerialEndLoop()
{
}

/**
 * set serial command handlers
 */
void MySerialCommand::begin()
{
  addCommand("SLIDE", onSerialSlide);
  addCommand("PAN", onSerialPan); 
  addCommand("TILT", onSerialTilt);
  //addCommand("WFC", onSerialWaitForCompletion);
  //addCommand("REST", onSerialRest);
  //addCommand("BLOOP", onSerialBeginLoop);
  //addCommand("ELOOOP", onSerialEndLoop);
  addCommand("HALT", onSerialHalt);
  addCommand("STOP", onSerialHalt);
  addCommand("PAUSE", onSerialPause);
  addCommand("RESUME", onSerialResume);
}



