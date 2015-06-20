#include <SPI.h>
#include <RH_RF69.h>

/**
 * Abstraction for communication channel between a Controller and a Remote
 *
 */
#define RADIO_MYFREQUENCY   915.0 
#define RADIO_MYPOWER       14
#define RADIO_MYMODULATION  RH_RF69::GFSK_Rb250Fd250


/**
 *
 */
class Radio : public RH_RF69
{
public:  
  void begin();
  
  /**
   * main read API.
   * Typically called after available() returns true;
   * Read a message and dispatch it - typically to RemoteCommandInterpreter
   */
  void getAndDispatch();
  
};

extern Radio g_radio;

/**
 * Messages
 */
const char msgCmdNone     = 0;
const char msgCmdStop     = 1;
const char msgCmdRun      = 2;
const char msgCmdPause    = 11;
const char msgCmdResume   = 12;
const char msgCmdSetSpeed = 21;
 
/** 
 * longest set of commands we can pass to controller in one message 
 * 
 */
#define MAX_RUN_COMMANDS  10

struct VideoSlidePanTiltMessage
{
  byte m_msgCmd;  // one of msgCmdXXX
  union {
    struct {
      char m_command;  // one of cmdXXX from CommandInterpreter.h
      char m_speed;    // -100...100
      byte m_duration; // in seconds
    } m_commands[MAX_RUN_COMMANDS];
    struct {
      char m_slide;
      char m_pan;
      char m_tilt;
    } m_speed; // for msgCmdSetSpeed
    
    
  } m_payload;
};



