/**
 *  A library to controll the TB6612FNG Toshiba H-bridge
 * The pins must be connected accordingly (see the datasheet of the H-bridge).
    -the PWM pins must support PWM on the Arduino
    -the enable pin on the H-bridge should be connected either to 5V
     or to a pin on the Arduino, which is set to HIGH, for the H-bridge to function
 * Original code by shrajtofle@gmail.com.
 * Modified by Alex Sokolsky asokolsky@yahoo.com 4/2015
 */
 
#include <arduino.h>

class DCMotor {
private:
  byte m_pinCW;  // a.k.a. IN1
  byte m_pinCCW; // a.k.a. IN2
  byte m_pinPWM;
   
  boolean m_bGoing;
  boolean m_bDirectionCW;
  byte m_speed;
  //byte m_speedRegulation = 255;

public:
  /** IN1, IN2, PWM */
  DCMotor(byte pinCW, byte pinCCW, byte pinPWM) :
    m_pinCW(pinCW), m_pinCCW(pinCCW), m_pinPWM(pinPWM), 
    m_bGoing(false), m_bDirectionCW(true), m_speed(255)
  { 
  }
  
  void begin();
   
  void go();
  void stop();
  void shortBreak();
   
  void setSpeed(boolean bCW, byte speed);
   
  //void setSpeedRegulation(byte speedRegulation);
   
  /*byte getSpeedRegulation() {
    return m_speedRegulation;
  }*/
  boolean isGoing() {
    return m_bGoing;
  }
   
};


