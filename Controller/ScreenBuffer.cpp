#include <LiquidCrystal.h>
#include <VideoSlidePanTiltController.h>

extern LiquidCrystal g_lcd;

static void initBySpaces(char buf[], byte len)
{
  memset(buf, ' ', len - 1);
  buf[len - 1] = 0;
}

ScreenBuffer::ScreenBuffer()
{
  initBySpaces(m_line1, sizeof(m_line1));
  initBySpaces(m_line2, sizeof(m_line2));
  initBySpaces(m_line3, sizeof(m_line3));
  initBySpaces(m_line4, sizeof(m_line4));
}

static byte arrowsLefRight[8] = {
  0b00000,
  0b01000,
  0b11111,
  0b01000,
  0b00010,
  0b11111,
  0b00010,
  0b00000
};

static byte arrowsUpDown[8] = {
  0b01000,
  0b11100,
  0b01000,
  0b01010,
  0b01010,
  0b00010,
  0b00111,
  0b00010
};

void ScreenBuffer::begin(LiquidCrystal *plcd) {
  // create a new character
  plcd->createChar(0, arrowsLefRight);
  // create a new character
  plcd->createChar(1, arrowsUpDown);
}


/**
 * output \x7E as custom character 0 and \x7F as custom char 1
 */
static void drawLine(LiquidCrystal *plcd, char buf[])
{
  char *pChar0 = strchr(buf, '\x7F');
  char *pChar1 = strchr(buf, '\x7E');
  if((pChar0 == 0) && (pChar1 == 0)) 
  {
    plcd->print(buf);
  } 
  else if((pChar0 != 0) && (pChar1 != 0)) 
  {
    *pChar0++ = 0;
    *pChar1++ = 0;
    plcd->print(buf);
    if(pChar0 < pChar1) {
      plcd->write((byte)0);
      plcd->print(pChar0);
      plcd->write((byte)1);
      plcd->print(pChar1);
    } else {
      plcd->write((byte)1);
      plcd->print(pChar1);
      plcd->write((byte)0);
      plcd->print(pChar0);
    }
  } 
  else if(pChar0 != 0) 
  {
    // pChar0 != 0
    *pChar0 = 0;
    plcd->print(buf);
    plcd->write((byte)0);
    plcd->write(++pChar0);
  } 
  else 
  {
    // pChar1 != 0
    *pChar1 = 0;
    plcd->print(buf);
    plcd->write((byte)1);
    plcd->write(++pChar1);
  }
}


void ScreenBuffer::draw(LiquidCrystal *plcd)
{
  plcd->setCursor(0, 0);
  drawLine(plcd, m_line1);
  DEBUG_PRINTLN(m_line1);
    
  g_lcd.setCursor(0, 1);
  drawLine(plcd, m_line2);
  DEBUG_PRINTLN(m_line2);

  g_lcd.setCursor(0, 2);
  drawLine(plcd, m_line3);
  DEBUG_PRINTLN(m_line3);

  g_lcd.setCursor(0, 3);
  drawLine(plcd, m_line4);
  DEBUG_PRINTLN(m_line4);

  plcd->setCursor(m_cursorCol, m_cursorRow);
  plcd->cursor();
}





