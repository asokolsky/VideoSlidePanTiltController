#include <VideoSlidePanTiltController.h>


char tickSelection(char iSelectedChannel) {
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

const char *getFullChannelName(byte channel) {
  switch(channel) {
    case cmdNone: return "None";
    case cmdSlide: return "Slide";
    case cmdPan: return "Pan";
    case cmdTilt: return "Tilt";
    case cmdRest: return "Rest";
    case cmdWaitForCompletion: return "Wait";
  }
  return "Unknown";
}

static char mapSelectionToColumn(char iSelectedChannel) {
  switch(iSelectedChannel) {
    case cmdPan:
      //return 4;
      return 7;
    case cmdTilt:
      //return 8;
      return 11;
    case cmdRest:
    case cmdWaitForCompletion:
      //return 12;
      return 16;
    //case cmdSlide:
    //default:
  }
  return 1;
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
 
012345678901234567890
Sli Pan Til Wait
- 1   0 -.3 99

 */
/*void ChannelsView::draw(
  const char *pLabel, char cSlideSpeed, char cPanSpeed, char cTiltSpeed, unsigned int wRemainingSecs, ScreenBuffer *pScreen)
{
  char *p = "Sli Pan Til ";
  int iPos = strlen(p);
  memcpy(pScreen->m_line1, p, iPos);
  strcpy(&pScreen->m_line1[iPos], pLabel);

  formatSpeed(&pScreen->m_line2[0], cSlideSpeed);
  //line[3] = ' ';
  formatSpeed(&pScreen->m_line2[4], cPanSpeed);
  //line[7] = ' ';
  formatSpeed(&pScreen->m_line2[8], cTiltSpeed);
  //line[11] = ' ';

  // show the # right aligned
  char szNumBuf[32];
  itoa((int)wRemainingSecs, szNumBuf, 10);
  szNumBuf[4] = '\0';
  strcpy(&pScreen->m_line2[16 - strlen(szNumBuf)], szNumBuf);
  //pScreen->m_line2[16] = '\0';

  //updateCursor();
  //DEBUG_PRINTDEC(s_cSelectedChannel);
  //DEBUG_PRINTLN("");
  
  //if(s_cSelectedChannel == cmdNone) {
  //  g_lcd.noCursor();
  //} else {
  //  g_lcd.setCursor(mapSelectionToColumn(s_cSelectedChannel), 0);
  //  g_lcd.cursor();
  //}
  //pScreen->m_cursorRow = 0;
  pScreen->m_cursorCol = mapSelectionToColumn(s_cSelectedChannel);
}*/

/**
 * draw somehing like this
 
0123456789012345678901
 Slide Pan Tilt Stop
   - 1   0  -.3   99

 Speed <  Duration V

 */
void ChannelsView::draw(
  const char *pLabel, char cSlideSpeed, char cPanSpeed, char cTiltSpeed, unsigned int wRemainingSecs, ScreenBuffer *pScreen)
{
  sprintf(pScreen->m_line1, " Slide Pan Tilt %s", pLabel);
  formatSpeed(&pScreen->m_line2[3], cSlideSpeed);
  formatSpeed(&pScreen->m_line2[7], cPanSpeed);
  formatSpeed(&pScreen->m_line2[12], cTiltSpeed);

  // show the # right aligned
  char szNumBuf[32];
  itoa((int)wRemainingSecs, szNumBuf, 10);
  szNumBuf[4] = '\0';
  strcpy(&pScreen->m_line2[20 - strlen(szNumBuf)], szNumBuf);
  //pScreen->m_line2[20] = '\0';

  switch(s_cSelectedChannel) {
    case cmdSlide:
    case cmdPan:
    case cmdTilt:
      strcpy(&pScreen->m_line4[1], "Speed \x7F  Duration \x7E");
      break;
    default:
      strcpy(&pScreen->m_line4[5],      "Pause \x7F  Stop \x7E");
  }
  

  //updateCursor();
  //DEBUG_PRINTDEC(s_cSelectedChannel);
  //DEBUG_PRINTLN("");
  
  //if(s_cSelectedChannel == cmdNone) {
  //  g_lcd.noCursor();
  //} else {
  //  g_lcd.setCursor(mapSelectionToColumn(s_cSelectedChannel), 0);
  //  g_lcd.cursor();
  //}
  //pScreen->m_cursorRow = 0;
  pScreen->m_cursorCol = mapSelectionToColumn(s_cSelectedChannel);
}

/**
 * kb handling
 */

/** 
 * left/right key handler 
 * Begin command or adjust current command speed
 */
void onLeftRightKeyUp(byte cSelectedChannel, char iSpeed) {
  DEBUG_PRINT("onLeftRightKeyUp iSpeed=");
  DEBUG_PRINTDEC(iSpeed);
  DEBUG_PRINTLN("");
  if(cSelectedChannel == cmdNone)
    return;
  if((cSelectedChannel == cmdRest) || (cSelectedChannel == cmdWaitForCompletion)) {
	  if (!g_pCommandInterpreter->isRunning())
      return;
    if(iSpeed < 0) {
      // left key - STOP
      g_pCommandInterpreter->stopRun();
    } else {
      // right key - PAUSE/RESUME
	  if (g_pCommandInterpreter->isPaused()) {
		    g_pCommandInterpreter->resumeRun();
      } else {
		    g_pCommandInterpreter->pauseRun();
      }
    }
    return;
  }
  if(!g_pCommandInterpreter->isRunning()) {
    g_pCommandInterpreter->beginRun(cSelectedChannel, iSpeed, 30 * 1000L);
  } else if (g_pCommandInterpreter->isBusy(cSelectedChannel)) {
    g_pCommandInterpreter->adjustCommandSpeed(cSelectedChannel, iSpeed);
  } else {
    g_pCommandInterpreter->beginCommand(cSelectedChannel, iSpeed, 30 * 1000L);
  }
}

/** 
 * up/down arrow key handler 
 * Adjust current command duration
 */
static void onUpDownKeyUp(byte cSelectedChannel, char iSecs) {
  DEBUG_PRINT("onUpDownKeyDown iSecs=");
  DEBUG_PRINTDEC(iSecs);
  DEBUG_PRINTLN("");
  g_pCommandInterpreter->adjustCommandDuration(cSelectedChannel, iSecs);
}

 
void ChannelsView::onKeyDown(byte vk) {
  DEBUG_PRINTLN("ChannelsView::onKeyDown");
}

void ChannelsView::onKeyUp(byte vk) {
  DEBUG_PRINTLN("ChannelsView::onKeyUp");
  switch(vk) {
    case VK_NONE:
      return;
    case VK_LEFT:
      onLeftRightKeyUp(s_cSelectedChannel, -20);
      break;
    case VK_RIGHT:
      onLeftRightKeyUp(s_cSelectedChannel, 20);
      break;
    case VK_DOWN:
      onUpDownKeyUp(s_cSelectedChannel, -2);
      break;
    case VK_UP:
      onUpDownKeyUp(s_cSelectedChannel, 2);
      break;
    case VK_SEL:
      // move selection
      s_cSelectedChannel = tickSelection(s_cSelectedChannel);
      break;
  }
  g_pCommandInterpreter->updateDisplay(millis());
}

void ChannelsView::onLongKeyDown(byte vk)
{
  DEBUG_PRINTLN("ChannelsView::onLongKeyDown");
  switch(vk) {
    case VK_LEFT:
      onLeftRightKeyUp(s_cSelectedChannel, -40);
      break;
    case VK_RIGHT:
      onLeftRightKeyUp(s_cSelectedChannel, 40);
      break;
    case VK_DOWN:
      onUpDownKeyUp(s_cSelectedChannel, -4);
      break;
    case VK_UP:
      onUpDownKeyUp(s_cSelectedChannel, 4);
      break;
    case VK_SEL:
      // switch view
      g_pView = &g_viewChannel;
      break;
    default:
      return;
  }
  g_pCommandInterpreter->updateDisplay(millis());
}

