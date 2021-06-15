#include <OSCMessage.h>
#include <OSCBoards.h>

#include <MPR121.h>
#include <Wire.h>

/*
Make an OSC message and send it over serial
 */

#ifdef BOARD_HAS_USB_SERIAL
#include <SLIPEncodedUSBSerial.h>
SLIPEncodedUSBSerial SLIPSerial(thisBoardsSerialUSB);
#else
#include <SLIPEncodedSerial.h>
SLIPEncodedSerial SLIPSerial(Serial);  // Change to Serial1 or Serial2 etc. for boards with multiple serial ports that don’t have Serial
#endif
// this is the touch threshold - setting it low makes it more like a proximity trigger
// default value is 40 for touch
const int touchThreshold = 1;
// this is the release threshold - must ALWAYS be smaller than the touch threshold
// default value is 20 for touch
const int releaseThreshold = 0;

void setup() {
  //begin SLIPSerial just like Serial
  SLIPSerial.begin(9600);  // set this as high as you can reliably run on your platform


  // 0x5C is the MPR121 I2C address on the Bare Touch Board
  if (!MPR121.begin(0x5A)) {
    Serial.println("error setting up MPR121");
    switch (MPR121.getError()) {
      case NO_ERROR:
        Serial.println("no error");
        break;
      case ADDRESS_UNKNOWN:
        Serial.println("incorrect address");
        break;
      case READBACK_FAIL:
        Serial.println("readback failure");
        break;
      case OVERCURRENT_FLAG:
        Serial.println("overcurrent on REXT pin");
        break;
      case OUT_OF_RANGE:
        Serial.println("electrode out of range");
        break;
      case NOT_INITED:
        Serial.println("not initialised");
        break;
      default:
        Serial.println("unknown error");
        break;
    }
    while (1)
      ;
  }


  MPR121.setTouchThreshold(touchThreshold);
  MPR121.setReleaseThreshold(releaseThreshold);
}


void loop() {
  //the message wants an OSC address as first argument
  int i = 0;
    OSCMessage msg("/analog/" + i);
  for (i = 0; i < 13; i++) {  // 13 baseline values
    msg.add((int32_t)MPR121.getBaselineData(i));
  }
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial);    // send the bytes to the SLIP stream
    SLIPSerial.endPacket();  // mark the end of the OSC Packet
    msg.empty();             // free space occupied by message

  delay(20);
}
