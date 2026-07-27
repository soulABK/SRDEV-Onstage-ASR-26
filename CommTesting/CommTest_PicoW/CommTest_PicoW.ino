//NOTE: USE SERIAL MONITOR TO TRACK CONNECTION PROGRESS
//NOTE: FOR TRIALS IN VOID LOOP(), STRINGS MUST END IN '\n' CHARACTER

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
  client = server.accept();
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
    char c = client.read();
    inputBuffer += c;
    if (c == '\n') {
      return 1; //return 1 if a character is successfully read and the full string has been completed (line break)
    }
    else {
      return 0; //return 0 if a character is successfully read but the full string has not been completed (no line break)
    }
  }
  else {
    return -1; //return -1 if client is disconnected
  }
}

void loop() {
  //run 3 trials of reading strings
  for (int i = 0; i < 3; i++) {
    Serial.printf("Starting Trial %d\n", i + 1);
    auto start = std::chrono::high_resolution_clock::now();
    int err = 0;
    while(err == 0) {
      err = readServerData(client);
    }
    auto end = std::chrono::high_resolution_clock::now();
    if (err == -1) {
      Serial.printf("Failed to read string due to disconnection from server, terminating program...\n");  
      exit(1);
    }
    if (err == 1) {
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
      Serial.printf("Successfully read %d bytes of string '%s', time elapsed: %lld ms\n", inputBuffer.length(), inputBuffer, duration.count());  
      inputBuffer = "";
    }
  }
  exit(0);
}
