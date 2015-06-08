#include "Trace.h"
#include "LcdKeypadShield.h"
#include "CommandInterpreter.h"
#include "Views.h"

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
void ChannelsView::draw(const char *pLabel, char cSlideSpeed, char cPanSpeed, char cTiltSpeed, unsigned int wRemainingSecs)
{
  g_lcd.setCursor(0, 0);
  g_lcd.print("Sli Pan Til ");
  DEBUG_PRINT("Sli Pan Til ");
  g_lcd.print(pLabel);
  DEBUG_PRINTLN(pLabel);

  g_lcd.setCursor(0, 1);
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
  g_lcd.print(line);
  DEBUG_PRINTLN(line);

  //updateCursor();
  //DEBUG_PRINTDEC(s_cSelectedChannel);
  //DEBUG_PRINTLN("");
  if(s_cSelectedChannel == cmdNone) {
    g_lcd.noCursor();
  } else {
    //DEBUG_PRINTLN("setCursor");
    g_lcd.setCursor(mapSelectionToColumn(s_cSelectedChannel), 0);
    g_lcd.cursor();
  }
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
    if(!g_ci.isRunning())
      return;
    if(iSpeed < 0) {
      // left key - STOP
      g_ci.stopRun();
    } else {
      // right key - PAUSE/RESUME
      if(g_ci.isPaused()) {
        g_ci.resumeRun();
      } else {
        g_ci.pauseRun();
      }
    }
    return;
  }
  if(!g_ci.isRunning()) {
    g_ci.beginRun(cSelectedChannel, iSpeed, 30*1000L);
  } else if(g_ci.isBusy(cSelectedChannel)) {
    g_ci.adjustCommandSpeed(cSelectedChannel, iSpeed);
  } else {
    g_ci.beginCommand(cSelectedChannel, iSpeed, 30*1000L);
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
  g_ci.adjustCommandDuration(cSelectedChannel, iSecs);
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
  g_ci.updateDisplay(millis());
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
  g_ci.updateDisplay(millis());
}

