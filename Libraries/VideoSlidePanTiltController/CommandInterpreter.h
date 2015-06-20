#ifndef CommandInterpreter_h
#define CommandInterpreter_h

/** 
 * constants for commands 
 * these are ALSO OFFSETS!
 */
const char cmdSlide = 0;
const char cmdPan   = 1;
const char cmdTilt  = 2;
//const char cmdZoom  = 3;

const char cmdFirst = cmdSlide;
const char cmdLast  = cmdTilt;
const char cmdMax = cmdLast+1; // the # of commands!  This is array size!
/** 
 * not really a command but the absense of commands 
 * delay consideration of the next command for this many millis
 */
const char cmdRest  = 10;
/** 
 * not really a command but a synchronization primitive
 * wait for all the channels to complete execution
 * either by time expiration or by exception such as endswitch
 */
const char cmdWaitForCompletion  = 11;

/**
 * Loop commands
 */
const char cmdBeginLoop  = 21;
const char cmdEndLoop  = 22;
 
/** not really a command but the marker of the end of commands */
const char cmdNone  = -1;

/**
 * Main Interface Class
 */
class CommandInterpreter
{
public:
  virtual void begin();
  virtual void beginRun(char cmd, char iSpeed, unsigned long ulDuration);

  /** external API of this class */
  virtual void beginCommand(char cmd, char cSpeed, unsigned long ulDuration);
  /**
  * iCmd is actually a channel #
  * to be called from interrupt handler or in response to kb
  */
  virtual void stopCommand(char cCmd);
  /** adjust speed */
  virtual void adjustCommandSpeed(char cmd, char cSpeedAdjustment);
  /** Duration adjustment in seconds */
  virtual void adjustCommandDuration(char cmd, int iDurationAdjustment);

  virtual void stopRun();

  virtual boolean isRunning();
  virtual boolean isPaused();
  /** is this channel busy? */
  virtual boolean isBusy(char cChannel);

  /** suspend the run, can resume */
  virtual void pauseRun();
  /** resume the run */
  virtual void resumeRun();

  virtual void updateDisplay(unsigned long now);
};

extern CommandInterpreter *g_pCommandInterpreter;

#endif

