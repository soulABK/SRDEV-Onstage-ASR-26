//file include
#include "WifiComm.h"

//functions
WifiPeer::WifiPeer(const char* PEER_SSID, const char* PEER_PASSWORD, int PEER_PORT) {
  ssid = PEER_SSID;
  password = PEER_PASSWORD;
  port = PEER_PORT;
  readBuffer.reserve(1024); 

  server = WiFiServer(port);
}

int WifiPeer::connect(int timeout, bool debug) {
  auto start = std::chrono::high_resolution_clock::now();
  if (debug == true) Serial.printf("Connecting to WiFi '%s'\n", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    auto current = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current - start);
    
    if (duration.count() % 1000) {
      if (debug == true) Serial.printf("Status: %d, Time elapsed: %lld ms\n", WiFi.status(), duration.count());
    }
    if (duration.count() > timeout) {
      if (debug == true) Serial.printf("Failed to connect to WiFi '%s' due to connection timeout (exceeded %d ms)\n", ssid, timeout);
      return -1;
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  if (debug == true) Serial.printf("Successfully connected to WiFi '%s', total time elapsed: %lld ms\n", ssid, duration.count());

  //begin PicoW as server (data sender)
  server.begin();
  //begin PicoW as client (data receiver)
  client = server.available();
  if (client) {
    if (debug == true) Serial.printf("Connected to server at %s:%d\n", WiFi.localIP().toString().c_str(), port);
  }
  else {
    if (debug == true) Serial.printf("Failed to connect to server\n");
    return -1;
  }
  return 0;
}

int WifiPeer::read() {
  if (client.connected()) {
    if (client.available()) {
      char c = client.read();
      readBuffer += c;
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

String WifiPeer::showBuffer() {
  return readBuffer;
}

void WifiPeer::clearBuffer() {
  readBuffer = "";
}

void WifiPeer::write(String message, bool debug) {
  if (client.connected()) {
    client.println(message);  
    if (debug == true) Serial.printf("Sent message '%s'\n", message.c_str());
  }
}

void WifiPeer::disconnect(bool debug) {
  client.stop();
  if (debug == true) Serial.printf("Disconnected client from server at %s:%d\n", WiFi.localIP().toString().c_str(), port);
}

bool WifiPeer::status() {
  return client.connected();
}