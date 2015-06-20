#ifndef Views_h
#define Views_h

class LiquidCrystal;

class ScreenBuffer {
public:
  char m_line1[21];
  char m_line2[21];
  char m_line3[21];
  char m_line4[21];
  byte m_cursorCol = 0;
  byte m_cursorRow = 0;

  ScreenBuffer();

  void draw(LiquidCrystal *plcd);

  static void begin(LiquidCrystal *plcd);
};

class View {
public:  
  View() {}
  ~View() {}

  /** Derived class will overwrite these.  Do nothing by default */
  virtual void draw(const char *pLabel, char cSlideSpeed, char cPanSpeed, char cTiltSpeed, unsigned int wRemainingSecs, ScreenBuffer *pScreen) = 0;
  virtual void onKeyDown(byte vk) = 0;
  virtual void onKeyUp(byte vk) = 0;
  virtual void onLongKeyDown(byte vk) = 0;
  
  /** 
   * Channel selected in the GUI.  
   * Should be one of cmdSlide, cmdPan or cmdTilt or cmdNone
   */
  static char s_cSelectedChannel;
  
};

class ChannelView : public View {
public:  
  ChannelView() {}
  ~ChannelView() {}

  virtual void draw(const char *pLabel, char cSlideSpeed, char cPanSpeed, char cTiltSpeed, unsigned int wRemainingSecs, ScreenBuffer *pScreen);
  void onKeyDown(byte vk);
  void onKeyUp(byte vk);
  void onLongKeyDown(byte vk);

private:
  /** 1..100 */
  char m_maxSpeed = 20;
  
  void onUpDownKeyUp(char delta);
  
};

class ChannelsView : public View {
public:  
  ChannelsView() {}
  ~ChannelsView() {}

  virtual void draw(const char *pLabel, char cSlideSpeed, char cPanSpeed, char cTiltSpeed, unsigned int wRemainingSecs, ScreenBuffer *pScreen);
  void onKeyDown(byte vk);
  void onKeyUp(byte vk);
  void onLongKeyDown(byte vk);

private:
};
/*
class RemoteView : public View {
public:
  RemoteView() {}
  ~RemoteView() {}

  virtual void draw(const char *pLabel, char cSlideSpeed, char cPanSpeed, char cTiltSpeed, unsigned int wRemainingSecs,
    byte *line1, byte *line2, byte *line3, byte *line4) = 0;
  void onKeyDown(byte vk);
  void onKeyUp(byte vk);
  void onLongKeyDown(byte vk);

private:
};*/

char tickSelection(char iSelectedChannel);
const char *getFullChannelName(byte channel);

extern ChannelsView g_viewChannels;
extern ChannelView g_viewChannel;
extern View *g_pView;

#endif
