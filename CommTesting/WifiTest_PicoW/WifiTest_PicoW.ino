//COMMUNICATION TEST - SEND STRING FROM JETSON TO PICOW VIA WIFI AND TRACK OVERALL TIME TO SEND AND RECEIVE
//NOTE: USE SERIAL MONITOR TO TRACK STEP-BY-STEP CONNECTION PROGRESS
//NOTE: STRINGS SENT TO THE PICOW MUST END IN '\n' CHARACTER 

//file include
#include "WifiComm.h"

//libraries
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

//definitions
#define CONNECTION_TIMEOUT 15000

WifiPeer peer("StormingKids", "todbot1234", 5000);

void setup() {
  //initialize Serial
  Serial.begin(115200);
  delay(2000);

  //initialize PicoW LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); //turn off LED

  peer.connect(CONNECTION_TIMEOUT, true);

  digitalWrite(LED_BUILTIN, HIGH); //turn on LED
}

void loop() {
  while (peer.status()) {
    int status = 0;
    while(status == 0) {
      status = peer.read();
    }
    if (status == -1) {
      Serial.printf("Failed to read string due to disconnection from server, terminating program...\n");  
      break;
    }
    if (status == 1) {
      String reply = peer.showBuffer();
      Serial.printf("Successfully read %d bytes of string '%s'", reply.length(), reply);  
      peer.write(reply);
      peer.clearBuffer();
    }
  }
  peer.disconnect();
  exit(0);
}