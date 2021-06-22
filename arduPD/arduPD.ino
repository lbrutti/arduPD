#include <OSCBoards.h>
#include <OSCBundle.h>


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

//converts the pin to an osc address
char* numToOSCAddress(int pin) {
  static char s[10];
  int i = 9;

  s[i--] = '\0';
  do {
    s[i] = "0123456789"[pin % 10];
    --i;
    pin /= 10;
  } while (pin && i);
  s[i] = '/';
  return &s[i];
}

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
  OSCBundle bndl;

  MPR121.updateAll();
  for (int i = 0; i < 13; i++) {  // 13 filtered values
    int data = MPR121.getFilteredData(i);


    //setup the output address which should be /d/(pin)
    char outputAddress[6];

    char touchAddress[10];
    char tthsAddress[10];
    char rthsAddress[10];
    char ftdatAddress[10];
    char bvalAddress[10];

    strcpy(touchAddress, "/touch");
    strcat(touchAddress, numToOSCAddress(i));
    bndl.add(touchAddress).add((int32_t)MPR121.getTouchData(i));

    strcpy(tthsAddress, "/tths");
    strcat(tthsAddress, numToOSCAddress(i));
    bndl.add(tthsAddress).add((int32_t)MPR121.getTouchThreshold(i));

    strcpy(rthsAddress, "/rths");
    strcat(rthsAddress, numToOSCAddress(i));
    bndl.add(rthsAddress).add((int32_t)MPR121.getReleaseThreshold(i));

    strcpy(ftdatAddress, "/ftdat");
    strcat(ftdatAddress, numToOSCAddress(i));
    bndl.add(ftdatAddress).add((int32_t)MPR121.getFilteredData(i));

    strcpy(bvalAddress, "/bval");
    strcat(bvalAddress, numToOSCAddress(i));
    bndl.add(bvalAddress).add((int32_t)MPR121.getBaselineData(i));


    strcpy(outputAddress, "/t");
    strcat(outputAddress, numToOSCAddress(i));
    //do the digital read and send the results
    bndl.add(outputAddress).add(data);

    SLIPSerial.beginPacket();

    bndl.send(SLIPSerial);  // send the bytes to the SLIP stream

    SLIPSerial.endPacket();  // mark the end of the OSC Packet

    bndl.empty();
  }

  delay(20);
}
