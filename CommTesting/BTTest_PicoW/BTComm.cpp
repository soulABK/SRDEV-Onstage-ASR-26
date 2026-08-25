//file include
#include "BTComm.h"

//functions
BTPeer::BTPeer(const char* PEER_NAME) {
  name = PEER_NAME;
  readBuffer.reserve(1024);
}

void BTPeer::connect(bool debug) {
  SerialBT.setName("PicoW_Serial");
  SerialBT.begin();
  if (debug == true) Serial.printf("Successfully initialized Bluetooth Serial, ready to pair\n");
}

int BTPeer::read() {
  if (SerialBT.available()) {
    char c = SerialBT.read();
    readBuffer += c;
    if (c == '\n') {
      return 1; //return 1 if client is connected and line break character is read
    }
  }
  return 0;
}

String BTPeer::showBuffer() {
  return readBuffer;
}

void BTPeer::clearBuffer() {
  readBuffer = "";
}

void BTPeer::write(String message, bool debug) {
  if (SerialBT.available()) {
    SerialBT.println(message);
    if (debug == true) Serial.printf("Sent message '%s'\n", message.c_str());
  }
}

bool BTPeer::status() {
  return SerialBT.available();
}