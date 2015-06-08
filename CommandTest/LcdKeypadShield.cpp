#include "LcdKeypadShield.h"
#include "Trace.h"
#include "Views.h"

/**
 * get one of VK_xxx
 */
byte LcdKeypadShield::getKey()
{
  int adc_key_in = analogRead(0);      // read the value from the sensor 
  // buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  if (adc_key_in > 1000) return VK_NONE; // We make this the 1st option for speed reasons since it will be the most likely result
  // For V1.1
  if (adc_key_in < 50)   return VK_RIGHT;  
  if (adc_key_in < 250)  return VK_UP; 
  if (adc_key_in < 450)  return VK_DOWN; 
  if (adc_key_in < 650)  return VK_LEFT; 
  if (adc_key_in < 850)  return VK_SEL;
  return VK_NONE;
}

boolean LcdKeypadShield::getAndDispatchKey(unsigned long ulNow)
{
  byte vk = getKey();
  if(vk == m_cOldKey) { 
    if(vk == VK_NONE)
      return false;
    // fire long key logic here  
    if((m_ulToFireLongKey == 0) || (ulNow < m_ulToFireLongKey))
      return false;
    m_ulToFireLongKey = 0;
    g_pView->onLongKeyDown(vk);
    return true;
  }
  
  if(m_cOldKey == VK_NONE) 
  {
    m_ulToFireLongKey = ulNow + s_iLongKeyDelay;
    g_pView->onKeyDown(vk);
  } 
  else if((m_ulToFireLongKey == 0) && (m_cOldKey == VK_SEL))
  {
    // onLongKeyDown was fired!
    // ignore this keyUp for VK_SEL
    ;
  } else {
    m_ulToFireLongKey = 0l;
    g_pView->onKeyUp(m_cOldKey);
  }
  m_cOldKey = vk;
  return true;
}


