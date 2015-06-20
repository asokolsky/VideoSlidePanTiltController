
/**
 * The class to marshall commands to a remote controller
 *
 */
class RemoteCommandInterpreter : public CommandInterpreter
{
public:
  void begin();
  void beginRun(char cmd, char iSpeed, unsigned long ulDuration);

  /** external API of this class */
  void beginCommand(char cmd, char cSpeed, unsigned long ulDuration);
  /** adjust speed */
  void adjustCommandSpeed(char cmd, char cSpeedAdjustment);
  /** Duration adjustment in seconds */
  void adjustCommandDuration(char cmd, int iDurationAdjustment);


  boolean isRunning();
  boolean isPaused();
  /** is this channel busy? */
  boolean isBusy(char cChannel);

  void stopRun();
  /** suspend the run, can resume */
  void pauseRun();
  /** resume the run */
  void resumeRun();

  /**
  * iCmd is actually a channel #
  * to be called from interrupt handler or in response to kb
  */
  void stopCommand(char cCmd);

  void updateDisplay(unsigned long now);

};


