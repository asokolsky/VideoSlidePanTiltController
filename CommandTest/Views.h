

class View {
public:  
  View() {}
  ~View() {}

  /** Derived class will overwrite these.  Do nothing by default */
  virtual void draw(const char *pLabel, char cSlideSpeed, char cPanSpeed, char cTiltSpeed, unsigned int wRemainingSecs) = 0;
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

  void draw(const char *pLabel, char cSlideSpeed, char cPanSpeed, char cTiltSpeed, unsigned int wRemainingSecs);
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

  void draw(const char *pLabel, char cSlideSpeed, char cPanSpeed, char cTiltSpeed, unsigned int wRemainingSecs);
  void onKeyDown(byte vk);
  void onKeyUp(byte vk);
  void onLongKeyDown(byte vk);

private:
};

extern char tickSelection(char iSelectedChannel);

extern ChannelsView g_viewChannels;
extern ChannelView g_viewChannel;
extern View *g_pView;


