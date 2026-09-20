//file include
#include "movement.h"
#include "imu.h"
#include <WiFi.h>
#include <Wire.h>

float vx = 0.0;
float vy = 0.0;

#define USE_WIFI

#ifdef USE_WIFI

WiFiServer server(5000);
const char* ssid = "StormingKids";
const char* password = "todbot1234";

#endif

void setup() {
  Serial.begin(115200);
  delay(5000);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(2000);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.println("Starting...");

  Wire1.setSDA(2);
  Wire1.setSCL(3);
  Wire1.begin();

  Serial.println("Wire initialized. ");

  Motor1.init();
  Motor2.init();
  Motor3.init();

  Serial.println("Motors initialized. ");

  if (!calibrateRotation()) {
    Serial.println("IMU failed to initialize.");
  }
  delay(IMU_CALIB_SAMPLES * IMU_CALIB_DELAY);

  Serial.println("IMU calibrated. ");

  #ifdef USE_WIFI

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Status: ");
    Serial.println(WiFi.status());
  }
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
  server.begin();
  digitalWrite(LED_BUILTIN, HIGH);

  #endif

  motor(0, 0, 0);
  delay(5000);
  Serial.println("Ready.");

}

bool started = false;

void parseVelocity(String data) {
  int xpos = data.indexOf("x");
  int ypos = data.indexOf("y");
  if (xpos == -1 || ypos == -1) return;

  String vxStr = data.substring(data.indexOf("x:") + 3, ypos-3);
  String vyStr = data.substring(data.indexOf("y:") + 3, data.length());

  vxStr.trim();
  vyStr.trim();

  vx = vxStr.toFloat();
  vy = vyStr.toFloat();
}

String inputBuffer = "";

void loop() {
  #ifdef USE_WIFI
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Connected");
    while (client.connected()) {
      updateRotation();
      vmotor(vx, vy, rotation);
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (inputBuffer.indexOf("v") != -1) {
            parseVelocity(inputBuffer);
            //Serial.println(vx);
            //Serial.println(vy);
            inputBuffer = "";
            // String str = String(vx_act) + ", " + String(vy_act);
            // client.println(str);
            // vx_act = 0;
            // vy_act = 0;
          } else {
            inputBuffer = "";
          }
        } else {
          inputBuffer += c;
        }
      }
    }
    motor(0, 0, 0);
  }
  // Motor1.update();
  // Motor2.update();
  // Motor3.update();
  #else

  //motor(25, 25, 25);
  updateRotation();
  rotation_test_motors();
  
  #endif
}
