//libraries
#include <chrono>
#include <string.h>
#include <SerialBT.h>

//objects
class BTPeer {
private:
  const char* name;
  String readBuffer = "";

public:
  BTPeer(const char* name);
  void connect(bool debug = false);
  int read();
  String showBuffer();
  void clearBuffer();
  void write(String message, bool debug = false);
  bool status();
};