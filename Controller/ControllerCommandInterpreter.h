#include "TB6612FNG.h"


struct Command
{
  char m_command;            // one of cmdXX
  char m_speed;              // also carries information on direction and speed in %: -100%..100%
  unsigned long m_uDuration; // in milli seconds  
};


/**
* (abstract) channel for slide, pan or tilt or zoom
*  uses a motor
* implements trapezoidal speed profile on the motor
*/
class CommandInterpreterChannel
{
  /** try to update speed in this many ms.  Should be similar to main loop tick.  */
  const unsigned long ulSpeedUpdateTick = 100;
  /** update speed by this much  */
  const byte bSpeedUpdateStep = 5;

protected:
  /**
  * If a command is being executed on the channel this is when it will end in ms
  * otherwise - 0
  */
  unsigned long m_ulNext;
  /**
  * trapezoidal speed profile support
  * next speed update should be at this time - check it in tick();
  */
  unsigned long m_ulNextSpeedUpdate = 0;
  /** setpoint - where we want for the ultimate motor speed to be */
  char m_cSpeed = 0;
  /** where we are at a moment */
  char m_cCurrentSpeed = 0;

  DCMotor m_motor;

public:
  CommandInterpreterChannel(byte pinCW, byte pinCCW, byte pinPWM);


  unsigned long getNext() {
    return m_ulNext;
  }
  char getSpeed() {
    return m_cSpeed;
  }
  char getCurrentSpeed() {
    return m_cCurrentSpeed;
  }

  void begin() {
    m_motor.begin();
  }
  void beginCommands();
  void endCommands() {}

  /**
  * may communicate with hardware
  */
  virtual void beginCommand(char cSpeed, unsigned long ulDuration, unsigned long now);
  /**
  * may communicate with hardware
  */
  virtual boolean endCommand();

  /**
  * may communicate with hardware
  */
  void pauseCommand();
  /**
  * may communicate with hardware
  */
  void resumeCommand(unsigned long ulPauseDuration);
  /**
  * mark command as ready to be completed
  * actual hw communication is done in endCommand();
  */
  void stopCommand(unsigned long now) {
    if (m_ulNext > 0)
      m_ulNext = now;
  }

  /**
  * Did we work long enough or did we hit an endswitch?
  */
  virtual boolean isReadyToEndCommand(unsigned long now);
  /**
  * Handle a GUI request
  */
  void adjustCommandSpeed(char iSpeedAdjustment);
  /**
  * Handle a GUI request
  */
  void adjustCommandDuration(char iChangeSecs);
  /**
  * command is being executed by hardware
  */
  boolean isBusy() {
    return (m_ulNext > 0);
  }
  /**
  * allows for trapezoidal velocity profile implementation
  */
  void tick(unsigned long now);

private:
  /**
  * implement a trapezoidal velocity profile
  * communicate with hardware
  */
  void doSpeedStep(unsigned long now);
};

class SlideCommandInterpreterChannel : public CommandInterpreterChannel
{
  byte m_bEndSwitchCounter;
public:
  SlideCommandInterpreterChannel(byte pinCW, byte pinCCW, byte pinPWM) :
    CommandInterpreterChannel(pinCW, pinCCW, pinPWM), m_bEndSwitchCounter(0)
  {
  }

  void beginCommand(char cSpeed, unsigned long ulDuration, unsigned long now);
  boolean endCommand();
  boolean isReadyToEndCommand(unsigned long now);

};


/**
* Main Interface Class for Controller
*/
class ControllerCommandInterpreter : public CommandInterpreter
{
public:
  /**
  * Call this according to your connections.
  */
  ControllerCommandInterpreter(
    byte pinSlideCW, byte pinSlideCCW, byte pinSlidePWM,
    byte pinPanCW, byte pinPanCCW, byte pinPanPWM,
    byte pinTiltCW, byte pinTiltCCW, byte pinTiltPWM);
  ~ControllerCommandInterpreter() {}


  /** external APIs of this class */
  void begin();
  void beginRun(char cmd, char iSpeed, unsigned long ulDuration);
  void beginRun(Command *p);
  bool continueRun(unsigned long now);
  void endRun();

  /** stop processing commands */
  void stopRun();

  /** suspend the run, can resume */
  void pauseRun();
  /** resume the run */
  void resumeRun();


  /** is this command interpreter interpreting commands? */
  boolean isRunning() {
    return (m_pCommand != 0);
  }
  /** is this command interpreter paused? */
  boolean isPaused() {
    return (m_ulPaused != 0);
  }
  /** is this channel busy? */
  boolean isBusy(char cChannel);


  /** external API of this class */
  void beginCommand(char cmd, char cSpeed, unsigned long ulDuration);
  /** adjust speed */
  void adjustCommandSpeed(char cmd, char cSpeedAdjustment);
  /** Duration adjustment in seconds */
  void adjustCommandDuration(char cmd, int iDurationAdjustment);

  void updateDisplay(unsigned long now);

  /**
  * iCmd is actually a channel #
  * to be called from interrupt handler or in response to kb
  */
  void stopCommand(char cCmd);

private:
  void beginCommand(Command *p, unsigned long now);

  /** cmdWaitForCompletion command handler */
  void beginWaitForCompletion();
  void endWaitForCompletion();

  /** cmdRest command handler */
  void beginRest(unsigned long ulDuration, unsigned long now);
  void endRest();

  /** cmdBeginLoop command handler */
  void beginLoop(Command *p);
  /** cmdEndLoop command handler */
  Command *endLoop();

  boolean isResting() {
    return (m_ulNext > 0);
  }
  boolean isReadyToEndRest(unsigned long now) {
    return (m_ulNext > 0) && (now >= m_ulNext);
  }
  boolean isWaitingForCompletion() {
    return m_bWaitingForCompletion;
  }
  /** get the total # of busy channels */
  char getBusyChannels();
  unsigned getBusySeconds(unsigned long now);

  /** command currently being executed */
  Command *m_pCommand = 0;
  /** Loop command to jump back to when we encounter EndLoop */
  Command *m_pBeginLoopCommand = 0;
  /** array of interpreter channels such as slide/pan/tilt/zoom */
  CommandInterpreterChannel *m_channels[cmdMax];
  /** when to execute next command */
  unsigned long m_ulNext = 0;
  /** when the display was last updated */
  unsigned long m_ulLastDisplayUpdate;
  /** when we were paused */
  unsigned long m_ulPaused = 0;
  /** we are waiting for command(s) to be completed. */
  boolean m_bWaitingForCompletion = false;
};

