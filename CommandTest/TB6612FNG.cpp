#include "TB6612FNG.h"

void DCMotor::begin() {
  pinMode(m_pinCW, OUTPUT);
  pinMode(m_pinCCW, OUTPUT);
  pinMode(m_pinPWM, OUTPUT);
}

void DCMotor::go() {
  digitalWrite(m_pinCW,  m_bDirectionCW ? HIGH : LOW);
  digitalWrite(m_pinCCW, m_bDirectionCW ? LOW : HIGH);
  analogWrite(m_pinPWM, m_speed);
  m_bGoing = true;
}

void DCMotor::stop() {
  digitalWrite(m_pinCW, LOW);
  digitalWrite(m_pinCCW, LOW);
  analogWrite(m_pinPWM, HIGH);
  m_bGoing = false;
}

void DCMotor::shortBreak() {
  digitalWrite(m_pinCW, HIGH);
  digitalWrite(m_pinCCW, HIGH);
  analogWrite(m_pinPWM, LOW);
  m_bGoing = true;
}


/*void DCMotor::setSpeedRegulation(byte speedRegulation) {
  m_speedRegulation = speedRegulation;
  if(m_bGoing)
    go();
}*/

void DCMotor::setSpeed(boolean bCW, byte speed) {
  m_bDirectionCW = bCW;
  m_speed = map(speed, 0, 255, 0, 100 /*m_speedRegulation*/);
  go();
}


