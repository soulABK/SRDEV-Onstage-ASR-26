//COMMUNICATION TEST - SEND STRING FROM JETSON TO PICOW AND TRACK OVERALL TIME TO SEND AND RECEIVE
//NOTE: USE SERIAL MONITOR TO TRACK STEP-BY-STEP CONNECTION PROGRESS
//NOTE: STRINGS SENT TO THE PICOW MUST END IN '\n' CHARACTER 

//libraries
#include <iostream>
#include <chrono>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <WiFi.h>

//definitions
#define CONNECTION_TIMEOUT 15000

//constants
const char* SSID = "StormingKids";
const char* PASSWORD = "todbot1234";
const int PORT = 5000;

//global variables
String inputBuffer = "";

//other
WiFiServer server(PORT); 
WiFiClient client;

void setup() {
  //initialize Serial
  Serial.begin(115200);
  delay(2000);

  //pre-allocate memory for WiFi input buffer
  inputBuffer.reserve(1024); 

  //initialize PicoW LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); //turn off LED

  //connect to WiFi
  bool success = true;
  auto start = std::chrono::high_resolution_clock::now();
  Serial.printf("Connecting to WiFi '%s'\n", SSID);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    auto current = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current - start);
    
    if (duration.count() % 1000) {
      Serial.printf("Status: %d, Time elapsed: %lld ms\n", WiFi.status(), duration.count());
    }
    if (duration.count() > CONNECTION_TIMEOUT) {
      success = false;
      break;
    }
  }
  if (success == false) {
    Serial.printf("Failed to connect to WiFi '%s' due to connection timeout (exceeded %d ms)\n", SSID, CONNECTION_TIMEOUT);
    exit(1);
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  Serial.printf("Successfully connected to WiFi '%s', total time elapsed: %lld ms\n", SSID, duration.count());

  //begin PicoW as server (data sender)
  server.begin();
  //begin PicoW as client (data receiver)
  client = server.available();
  if (client) {
    Serial.printf("Connected to server at %s:%d\n", WiFi.localIP().toString().c_str(), PORT);
  }
  else {
    Serial.printf("Failed to connect to server\n");
    exit(1);
  }
  digitalWrite(LED_BUILTIN, HIGH); //turn on LED
}

//function: readServerData, read characters from server and store in inputBuffer
int readServerData(WiFiClient &client) {
  if (client.connected()) {
    if (client.available()) {
      char c = client.read();
      inputBuffer += c;
      if (c == '\n') {
        return 1; //return 1 if client is connected and line break character is read
      }
    }
    return 0; //return 0 if client is connected but line break character has not been read / no character has been read
  }
  else {
    return -1; //return -1 if client is disconnected
  }
}

void loop() {
  while (client.connected()) {
    int err = 0;
    while(err == 0) {
      err = readServerData(client);
    }
    if (err == -1) {
      Serial.printf("Failed to read string due to disconnection from server, terminating program...\n");  
      break;
    }
    if (err == 1) {
      Serial.printf("Successfully read %d bytes of string '%s'", inputBuffer.length(), inputBuffer);  
      client.println(inputBuffer);
      inputBuffer = "";
    }
  }
  client.stop();
  exit(0);
}
