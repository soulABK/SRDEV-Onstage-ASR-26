//COMMUNICATION TEST - SEND STRING FROM JETSON TO PICOW VIA BLUETOOTH AND TRACK OVERALL TIME TO SEND AND RECEIVE

//libraries
#include <iostream>
#include <chrono>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <SerialBT.h>

//definitions
#define CONNECTION_TIMEOUT 15000

//global variables
String inputBuffer = "";

void setup() {
  //initialize Serial
  Serial.begin(115200);
  delay(2000); 

  //pre-allocate memory for WiFi input buffer
  inputBuffer.reserve(1024); 

  //initialize PicoW LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); //turn off LED
  
  //initialize Bluetooth with the broadcast name "PicoW_Serial"
  SerialBT.setName("PicoW_Serial");
  SerialBT.begin();
  Serial.printf("Successfully initialized Bluetooth Serial, ready to pair\n");
  digitalWrite(LED_BUILTIN, HIGH);
}

//function: readData
int readData() {
  if (SerialBT.available()) {
    char c = SerialBT.read();
    inputBuffer += c;
    if (c == '\n') {
      return 1; //return 1 if client is connected and line break character is read
    }
  }
  return 0;
}

//function: writeData
void writeData(String message) {
  if (SerialBT.available()) {
    SerialBT.println(message);
  }
}

void loop() {
  while(1) {
    int err = 0;
    while(err == 0) {
      err = readData();
    }
    Serial.printf("Successfully read %d bytes of string '%s'", inputBuffer.length(), inputBuffer);  
    if (strcmp(inputBuffer.c_str(), "stop") == 0)
      break;
    writeData(inputBuffer);
    inputBuffer = "";
  }
  exit(0);
}
