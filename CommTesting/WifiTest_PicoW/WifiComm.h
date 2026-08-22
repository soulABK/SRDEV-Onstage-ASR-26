//libraries
#include <chrono>
#include <string.h>
#include <WiFi.h>

//objects
class WifiPeer {
private:
  const char* ssid;
  const char* password;
  int port;
  WiFiServer server; 
  WiFiClient client;
  String readBuffer = "";

public:
  WifiPeer(const char* PEER_SSID, const char* PEER_PASSWORD, int PEER_PORT = 5000);
  int connect(int timeout, bool debug = false);
  int read();
  String showBuffer();
  void clearBuffer();
  void write(String message, bool debug = false);
  void disconnect(bool debug = false);
  bool status();
};