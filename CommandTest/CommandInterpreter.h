
/**
 * DEBUG utils
 */
#define DEBUG 1

#ifdef DEBUG
  #define DEBUG_PRINT(x)    Serial.print(x)
  #define DEBUG_PRINTDEC(x) Serial.print(x, DEC)
  #define DEBUG_PRINTLN(x)  Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTDEC(x)
  #define DEBUG_PRINTLN(x)
#endif 

#include "TB6612FNG.h"

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
/** not really a command but the marker of the end of commands */
const char cmdNone  = -1;


struct Command
{
  char m_command;            // one of cmdXX
  char m_speed;             //  carries information on direction and speed in %: -100%..100%
  unsigned long m_uDuration; // in milli seconds  
};

/**
 * (abstract) channel for slide, pan or tilt or zoom
 */
struct CommandInterpreterChannel
{
  /** 
   * If a command is being executed on the channel this is when it will end in ms 
   * otherwise - 0
   */
  unsigned long m_ulNext;
  char m_cSpeed;
  
  virtual void beginCommands();
  /** may communicate with hardware */
  virtual void beginCommand(char cSpeed, unsigned long ulDuration, unsigned long now) = 0;
  /** may communicate with hardware */
  virtual boolean endCommand() = 0;
  virtual void endCommands();

  void saveCommand(char cSpeed, unsigned long ulDuration, unsigned long now);
  void adjustCommandSpeed(char iSpeedAdjustment);
  void adjustCommandDuration(char iSecs);

  /** 
   * mark command as ready to be completed
   * actual hw communication is done in endCommand();
   */
  void stopCommand(unsigned long now) {
    if(m_ulNext > 0)
      m_ulNext = now;
  }
  
  /**
   * command is being executed by hardware
   */
  boolean isBusy() {
    return (m_ulNext > 0);
  }
  boolean isReadyToEndCommand(unsigned long now) {
    return (m_ulNext > 0) && (now >= m_ulNext);
  }
};

class SlideCommandInterpreterChannel :  public CommandInterpreterChannel 
{
  DCMotor m_motor;

public:  
  SlideCommandInterpreterChannel(byte pinCW, byte pinCCW, byte pinPWM) : 
    m_motor(pinCW, pinCCW, pinPWM) {
    m_motor.setSpeedRegulation(100);
  }
  
  void beginCommand(char cSpeed, unsigned long ulDuration, unsigned long now);
  boolean endCommand();
};

class PanCommandInterpreterChannel :  public CommandInterpreterChannel 
{
  DCMotor m_motor;

public:  
  PanCommandInterpreterChannel(byte pinCW, byte pinCCW, byte pinPWM) : 
    m_motor(pinCW, pinCCW, pinPWM) {
    m_motor.setSpeedRegulation(100);
  }

  void beginCommand(char cSpeed, unsigned long ulDuration, unsigned long now);
  boolean endCommand();
};

class TiltCommandInterpreterChannel :  public CommandInterpreterChannel 
{
  DCMotor m_motor;

public:  
  TiltCommandInterpreterChannel(byte pinCW, byte pinCCW, byte pinPWM) : 
    m_motor(pinCW, pinCCW, pinPWM) {
    m_motor.setSpeedRegulation(100);
  }

  void beginCommand(char cSpeed, unsigned long ulDuration, unsigned long now);
  boolean endCommand();
};

/**
 * Main Interface Class
 */
class CommandInterpreter
{
public:
  /** 
   * Call this according to your connections.
   */
  CommandInterpreter(
    byte pinSlideCW, byte pinSlideCCW, byte pinSlidePWM,
    byte pinPanCW, byte pinPanCCW, byte pinPanPWM,
    byte pinTiltCW, byte pinTiltCCW, byte pinTiltPWM);    
  ~CommandInterpreter() {}
	
  /** external API of this class */
  void beginRun(char cmd, char iSpeed, unsigned long ulDuration);

  void beginRun(Command *p);
  bool continueRun(unsigned long now);
  void endRun();

  /** stop processing commands */
  void stopRun();
  
  boolean isRunning() {
    return (m_pCommand != 0);
  }
  boolean isBusy(char cChannel);
  

  /** external API of this class */
  void beginCommand(char cmd, char cSpeed, unsigned long ulDuration);
  void adjustCommandSpeed(char cmd, char cSpeedAdjustment);
  /** Duration adjustment in seconds */
  void adjustCommandDuration(char cmd, int iDurationAdjustment); 

  void updateDisplay(unsigned long now);
  
private:
  void beginCommand(Command *p, unsigned long now);
  /** 
   * iCmd is actually a channel # 
   * to be called from interrupt handler or in response to kb
   */
  void stopCommand(char cCmd);
  
  void beginWaitForCompletion() {
    m_bWaitingForCompletion = true;
  }
  void endWaitForCompletion() {
    m_bWaitingForCompletion = false;
  }
  
  void beginRest(unsigned long ulDuration, unsigned long now) {
    m_ulNext = now + ulDuration;
    updateDisplay(now);
  }
  void endRest() {
    m_ulNext = 0;
  }
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
  word getBusySeconds(unsigned long now);
  
  Command *m_pCommand;
  CommandInterpreterChannel *m_channels[cmdMax];
  unsigned long m_ulNext; // to execute Rest
  unsigned long m_ulLastDisplayUpdate; // when the display was last updated
  boolean m_bWaitingForCompletion;
};


