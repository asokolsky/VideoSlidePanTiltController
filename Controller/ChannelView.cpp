#include <VideoSlidePanTiltController.h>

static char getChannelSpeed(byte channel, char cSlideSpeed, char cPanSpeed, char cTiltSpeed) {
  switch(channel) {
    case cmdSlide: return cSlideSpeed;
    case cmdPan: return cPanSpeed;
    case cmdTilt: return cTiltSpeed;
  }
  return '\0';
}

/**
 * draw somehing like this in a 16x2 screeen
 
0123456789012345
Slide        < >
Speed: -50%

0123456789012345
Slide        < >
MaxSpeed: 100%

 */
/*
void ChannelView::draw(
  const char *pLabel, char cSlideSpeed, char cPanSpeed, char cTiltSpeed, unsigned int wRemainingSecs, ScreenBuffer *pScreen)
{
  const char *p = getFullChannelName(s_cSelectedChannel);
  memcpy(pScreen->m_line1, p, strlen(p));
  pScreen->m_line1[13] = '<';
  pScreen->m_line1[15] = '>';
  pScreen->m_line1[16] = '\0';

  int iPos = 0;
  char cSpeed = getChannelSpeed(s_cSelectedChannel, cSlideSpeed, cPanSpeed, cTiltSpeed);
  if(cSpeed == 0) {
    // show and adjust max speed
    cSpeed = m_maxSpeed;
    p = "Max";
    memcpy(&pScreen->m_line2[iPos], p, strlen(p));
    iPos = 3;
  } else {
    // show current speed
    ;
  }
  p = "Speed:";
  memcpy(&pScreen->m_line2[iPos], p, strlen(p));
  char szNum[8];
  itoa(cSpeed, szNum, 10);
  iPos += 6;
  if(szNum[0] != '-') iPos++;
  memcpy(&pScreen->m_line2[iPos], szNum, strlen(szNum));
  iPos += strlen(szNum);
  pScreen->m_line2[iPos] = '%';
  pScreen->m_line2[16] = '\0';

  //if(s_cSelectedChannel == cmdNone) {
  //  g_lcd.noCursor();
  //} else {
  //  g_lcd.setCursor(0, 0);
  //  g_lcd.cursor();
  //}

  //pScreen->m_cursorRow = 0;
  //pScreen->m_cursorCol = 0;
}
*/

/**
 * draw somehing like this in a 20x4 screeen
 
0123456789012345678901
Slide             < >
Speed: -50%

0123456789012345678901
Slide             < >
MaxSpeed: 100%

 */
void ChannelView::draw(
  const char *pLabel, char cSlideSpeed, char cPanSpeed, char cTiltSpeed, unsigned int wRemainingSecs, ScreenBuffer *pScreen)
{
  const char *p = getFullChannelName(s_cSelectedChannel);
  memcpy(pScreen->m_line1, p, strlen(p));
  pScreen->m_line1[19] = '\x7F'; // <- ->

  int iPos = 0;
  char cSpeed = getChannelSpeed(s_cSelectedChannel, cSlideSpeed, cPanSpeed, cTiltSpeed);
  if(cSpeed == 0) {
    // show and adjust max speed
    cSpeed = m_maxSpeed;
    p = "Max";
    memcpy(&pScreen->m_line2[iPos], p, strlen(p));
    iPos = 3;
    pScreen->m_line2[19] = '\x7E'; // /|\  \|/
  } else {
    // show current speed
    ;
  }
  p = "Speed:";
  memcpy(&pScreen->m_line2[iPos], p, strlen(p));
  char szNum[8];
  itoa(cSpeed, szNum, 10);
  iPos += 7;
  if(szNum[0] != '-') iPos++;
  memcpy(&pScreen->m_line2[iPos], szNum, strlen(szNum));
  iPos += strlen(szNum);
  pScreen->m_line2[iPos] = '%';
  //pScreen->m_line2[16] = '\0';

  //if(s_cSelectedChannel == cmdNone) {
  //  g_lcd.noCursor();
  //} else {
  //  g_lcd.setCursor(0, 0);
  //  g_lcd.cursor();
  //}

  //pScreen->m_cursorRow = 0;
  //pScreen->m_cursorCol = 0;
}


extern void onLeftRightKeyUp(byte cSelectedChannel, char iSpeed);


void ChannelView::onKeyDown(byte vk)
{
  DEBUG_PRINTLN("ChannelView::onKeyDown");
  switch(vk) {
    case VK_NONE:
      return;
    case VK_LEFT:
      onLeftRightKeyUp(s_cSelectedChannel, -m_maxSpeed);
      break;
    case VK_RIGHT:
      onLeftRightKeyUp(s_cSelectedChannel, m_maxSpeed);
      break;
    case VK_DOWN:
      //onUpDownKeyDown(s_cSelectedChannel, -2);
      break;
    case VK_UP:
      //onUpDownKeyDown(s_cSelectedChannel, 2);
      break;
    case VK_SEL:
      break;
  }
  g_pCommandInterpreter->updateDisplay(millis());
}

void ChannelView::onUpDownKeyUp(char delta) {
  char cSpeed = m_maxSpeed + delta;
  if(cSpeed < 1)
    cSpeed = 1;
  else if (cSpeed > 100)
    cSpeed = 100;
  m_maxSpeed = cSpeed;
}

void ChannelView::onKeyUp(byte vk) {
  DEBUG_PRINTLN("ChannelView::onKeyUp");
  switch(vk) {
    case VK_NONE:
      return;
    case VK_LEFT:
    case VK_RIGHT:
      g_pCommandInterpreter->stopCommand(s_cSelectedChannel);
      break;
    case VK_DOWN:
      onUpDownKeyUp(-5);
      break;
    case VK_UP:
      onUpDownKeyUp(5);
      break;
    case VK_SEL:
      // move selection
      s_cSelectedChannel = tickSelection(s_cSelectedChannel);
      break;
  }
  g_pCommandInterpreter->updateDisplay(millis());
}

void ChannelView::onLongKeyDown(byte vk)
{
  DEBUG_PRINTLN("ChannelView::onLongKeyDown");
  switch(vk) {
    case VK_DOWN:
      onUpDownKeyUp(-90);
      break;
    case VK_UP:
      onUpDownKeyUp(90);
      break;
    case VK_SEL:
      // switch view on long key VK_SEL
      g_pView = &g_viewChannels;
      break;
    deafault:
      return;
  }
  g_pCommandInterpreter->updateDisplay(millis());
}



