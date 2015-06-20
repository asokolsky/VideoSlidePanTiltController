/**
 *  A library to controll the TB6612FNG Toshiba H-bridge
 * The pins must be connected accordingly (see the datasheet of the H-bridge).
    -the PWM pins must support PWM on the Arduino
    -the enable pin on the H-bridge should be connected either to 5V
     or to a pin on the Arduino, which is set to HIGH, for the H-bridge to function
 * Original code by shrajtofle@gmail.com.
 * Modified by Alex Sokolsky asokolsky@yahoo.com 4/2015
 */
#ifndef TB6612FNG_h
#define TB6612FNG_h
 
class DCMotor {

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
   
  /** speed is in 0%...100% */
  void setSpeed(boolean bCW, byte speed);
   
  boolean isGoing() {
    return m_bGoing;
  }
   
private:
  byte m_pinCW;  // a.k.a. IN1
  byte m_pinCCW; // a.k.a. IN2
  byte m_pinPWM;
   
  boolean m_bGoing;
  boolean m_bDirectionCW;
  byte m_speed;
};

#endif

