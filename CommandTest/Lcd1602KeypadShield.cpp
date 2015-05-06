#include "Lcd1602KeypadShield.h"
#include "Trace.h"
#include "CommandInterpreter.h"

Lcd1602KeypadShield::Lcd1602KeypadShield() : 
  LiquidCrystal(8, 9, 4, 5, 6, 7), m_cOldKey(VK_NONE), m_bAutoRepeat(false), m_cSelectedChannel(cmdSlide)
{
  
}

/**
  * draw an aray of lines, iLines is the number of lines in the array
  * display iFirstLine on top
  */
/*void Lcd1602KeypadShield::draw(const char *lines[], int iLines, int iFirstRow, int iFirstCol)
{
  for(char iRow = 0; iRow < s_iRows; iRow++) {
    LiquidCrystal::setCursor(0, iRow);
    char line[s_iCols + 1];
    // code to demo all the chars in the charset
    //for(char i = 0; i<sizeof(line); i++)
    //  line[i] = iFirstCol + i + ' ';
    // wipe the line
    memset(line, ' ', sizeof(line));   
    int iLine = iFirstRow + iRow; // pointer into lines
    if(iLine < iLines) {
      int iLineLength = strlen(lines[iLine]);
      if(iLineLength > iFirstCol) {
        iLineLength = iLineLength - iFirstCol;
        if(iLineLength < sizeof(line)) {
          memcpy(line, lines[iLine] + iFirstCol, iLineLength);
        } else {
          memcpy(line, lines[iLine] + iFirstCol, sizeof(line));
        }
      }
    }
    line[sizeof(line) - 1] = 0;
    LiquidCrystal::print(line);
  }
}*/

static char tickSelection(char iSelectedChannel) {
  switch(iSelectedChannel) {
    case cmdSlide:
      return cmdPan;
    case cmdPan:
      return cmdTilt;
    case cmdTilt:
      return cmdRest;
    case cmdRest:
    case cmdWaitForCompletion:
      return cmdSlide;
    //default:
  }
  return cmdNone;
}
static char mapSelectionToColumn(char iSelectedChannel) {
  switch(iSelectedChannel) {
    case cmdPan:
      return 4;
    case cmdTilt:
      return 8;
    case cmdRest:
    case cmdWaitForCompletion:
      return 12;
    //case cmdSlide:
    //default:
  }
  return 0;
}

/** 
 * iSpeed is between -100 to 100
 * produce output like '  0' or '-.2' or '+ 1'
 */
static void formatSpeed(char *buf, char cSpeed) {
/*  DEBUG_PRINT("formatSpeed iSpeed=");
  DEBUG_PRINTDEC(iSpeed);
  DEBUG_PRINTLN(""); */
  if(cSpeed == 0) {
    //buf[0] = buf[1] = buf[2] = ' ';
    return;
  } else if (cSpeed > 0){
    buf[0] = '+';
  } else {
    buf[0] = '-';
    cSpeed = - cSpeed;
  }
  if(cSpeed >= 95) {
    //buf[1] = ' ';
    buf[2] = '1';
  } else {
    buf[1] = '.';
    cSpeed = cSpeed / 10;
    buf[2] = '0' + cSpeed;
  }
}

/**
 * draw somehing like this
 
12345678901234567890
Sli Pan Til Wait
- 1   0 -.3 99

 */

void Lcd1602KeypadShield::draw(const char *pLabel, char cSlideSpeed, char cPanSpeed, char cTiltSpeed, unsigned int wRemainingSecs)
{
  setCursor(0, 0);
  print("Sli Pan Til ");
  DEBUG_PRINT("Sli Pan Til ");
  print(pLabel);
  DEBUG_PRINTLN(pLabel);

  setCursor(0, 1);
  char line[32];
  memset(line, ' ', sizeof(line));
  formatSpeed(&line[0], cSlideSpeed);
  //line[3] = ' ';
  formatSpeed(&line[4], cPanSpeed);
  //line[7] = ' ';
  formatSpeed(&line[8], cTiltSpeed);
  //line[11] = ' ';

  // show the # right aligned
  char szNumBuf[32];
  itoa((int)wRemainingSecs, szNumBuf, 10);
  szNumBuf[4] = '\0';
  strcpy(&line[16 - strlen(szNumBuf)], szNumBuf);

  line[16] = '\0';
  print(line);
  DEBUG_PRINTLN(line);

  //updateCursor();
  //DEBUG_PRINTDEC(m_cSelectedChannel);
  //DEBUG_PRINTLN("");
  if(m_cSelectedChannel == cmdNone) {
    noCursor();
  } else {
    //DEBUG_PRINTLN("setCursor");
    setCursor(mapSelectionToColumn(m_cSelectedChannel), 0);
    cursor();
  }
}

/**
 * get one of VK_xxx
 */
char Lcd1602KeypadShield::getKey()
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
  if (adc_key_in < 850)  return VK_ENTER;  
  return VK_NONE;
}

boolean Lcd1602KeypadShield::getAndDispatchKey(unsigned long ulNow)
{
  char vk = getKey();
  if(vk == m_cOldKey) { 
    if(vk == VK_NONE)
      return false;
    // auto-repeat logic here  
    if(!m_bAutoRepeat) 
        return false;
    if(ulNow < m_ulToFireAutoRepeat)
      return false;
    m_ulToFireAutoRepeat = ulNow + s_iAutoRepeatPeriod;
    onKeyDown(vk);
    onKeyUp(vk);
    return true;
  }
  if(m_cOldKey == VK_NONE) {
    m_ulToFireAutoRepeat = ulNow + s_iAutoRepeatDelay;
    onKeyDown(vk);
  } else {
    m_ulToFireAutoRepeat = 0;
    onKeyUp(m_cOldKey);
  }
  m_cOldKey = vk;
  return true;
}

/**
 * kb handling
 */
 
extern CommandInterpreter g_ci;

/** 
 * left/right key handler 
 * Begin command or adjust current command speed
 */
void Lcd1602KeypadShield::onLeftRightKeyDown(char iSpeed) {
  DEBUG_PRINT("Lcd1602KeypadShield::onLeftRightKeyDown iSpeed=");
  DEBUG_PRINTDEC(iSpeed);
  DEBUG_PRINTLN("");
  if(m_cSelectedChannel == cmdNone)
    return;
  if((m_cSelectedChannel == cmdRest) || (m_cSelectedChannel == cmdWaitForCompletion)){
    if(g_ci.isRunning())
      g_ci.stopRun();
    return;
  }
  if(!g_ci.isRunning()) {
    g_ci.beginRun(m_cSelectedChannel, iSpeed, 30*1000L);
  } else if(g_ci.isBusy(m_cSelectedChannel)) {
    g_ci.adjustCommandSpeed(m_cSelectedChannel, iSpeed);
  } else {
    g_ci.beginCommand(m_cSelectedChannel, iSpeed, 30*1000L);
  }
}

/** 
 * up/down arrow key handler 
 * Adjust current command duration
 */
void Lcd1602KeypadShield::onUpDownKeyDown(char iSecs) {
  DEBUG_PRINT("Lcd1602KeypadShield::onUpDownKeyDown iSecs=");
  DEBUG_PRINTDEC(iSecs);
  DEBUG_PRINTLN("");
  g_ci.adjustCommandDuration(m_cSelectedChannel, iSecs);
}

void Lcd1602KeypadShield::onKeyDown(int vk) {
  DEBUG_PRINTLN("Lcd1602KeypadShield::onKeyDown");
  switch(vk) {
    case VK_NONE:
      return;
    case VK_LEFT:
      onLeftRightKeyDown(-20);
      break;
    case VK_RIGHT:
      onLeftRightKeyDown(20);
      break;
    case VK_DOWN:
      onUpDownKeyDown(-2);
      break;
    case VK_UP:
      onUpDownKeyDown(2);
      break;
    case VK_ENTER:
      // move selection
      m_cSelectedChannel = tickSelection(m_cSelectedChannel);
      break;
  }
  g_ci.updateDisplay(millis());
}

void Lcd1602KeypadShield::onKeyUp(int vk) {
}


