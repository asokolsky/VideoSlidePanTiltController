#include <VideoSlidePanTiltController.h>
#include "Radio.h"



void Radio::begin()
{
  if(!this->init()) {
    DEBUG_PRINTLN("Radio.init() failed!");
    return;
  }
  // Defaults after init are:
  //    434.0MHz, 
  //    modulation GFSK_Rb250Fd250
  //    +13dbM
  //    No encryption
  if(!this->setFrequency(RADIO_MYFREQUENCY)) {
    DEBUG_PRINTLN("Radio.setFrequency() failed!");
    return;
  }
  this->setModemConfig(RADIO_MYMODULATION);

  // If you are using a high power RF69, 
  // you *must* set a Tx power in the range 14 to 20 like this:
  this->setTxPower(RADIO_MYPOWER);
  
  //this->setPreambleLength(3);
  
  //uint8_t syncwords[] = { 'A', 'S' };
  //this->setSyncWords(syncwords, sizeof(syncwords));

  // The encryption key has to be the same as the one in the server
  /*
static uint8_t key[] = { 
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

  this->setEncryptionKey(key);
  */

  
}

void Radio::getAndDispatch() 
{
  // Should be a message for us now!
  uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  if(!this->recv(buf, &len)) {
    DEBUG_PRINTLN("Radio.recv() failed!");
    return;
  }
  DEBUG_PRINT("RSSI: ");
  DEBUG_PRINTDEC(this->lastRssi());
  this->printBuffer("Rx: ", buf, len);
  //DEBUG_PRINT("Rx: ");
  //DEBUG_PRINTLN((char*)buf);
  
  // dispatch!
  //..
  
}




