//file include
#include "movement.h"
#include "imu.h"
#include <WiFi.h>
#include <Wire.h>

//#define TESTING

#ifndef TESTING

float vx = 0.0;
float vy = 0.0;

#define USE_WIFI

#ifdef USE_WIFI

WiFiServer server(5000);
const char* ssid = "StormingKids";
const char* password = "todbot1234";
//const char* ssid = "Moon";
//const char* password = "Chris419";

#endif

void setup() {
  Serial.begin(115200);
  delay(5000);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.println("Starting...");

  Wire1.setSDA(2);
  Wire1.setSCL(3);
  Wire1.begin();

  Serial.println("Wire initialized. ");

  Motor1.init();
  Motor2.init();
  Motor3.init();

  Serial.println("Motor initialized. ");

  delay(1000);
  
  if (!initIMU()) {
    Serial.println("BNO055 not responding");
    for (int i = 0; i < 10; i++) { // some signal
      digitalWrite(LED_BUILTIN, HIGH); delay(80);
      digitalWrite(LED_BUILTIN, LOW);  delay(80);
    }
  } else {

    Serial.println("IMU initialized. ");

  }

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

  if (!calibrateRotation()) {
    Serial.println("Heading calibration failed");
  }
  Serial.println("IMU initialized. ");
  Serial.print("Heading zero set to ");
  Serial.println(zeroRot);

  motor(0, 0, 0);

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
    uint32_t lastControl = 0;

    while (client.connected()) {
      while (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (inputBuffer.indexOf("v") != -1) {
            parseVelocity(inputBuffer);
          }
          inputBuffer = "";
        } else {
          inputBuffer += c;
        }
      }
    
      uint32_t now = micros();
      if (now - lastControl >= CONTROL_PERIOD_US) {
        lastControl = now;
        updateRotation();
        vmotor(vx, vy, rotation, rotationRate);
      }

    }
    motor(0, 0, 0);
  }
  // Motor1.update();
  // Motor2.update();
  // Motor3.update();
  #else

  updateRotation();
  static uint32_t lastHeadingPrint = 0;
  if (millis() - lastHeadingPrint > 200) {
    Serial.printf("heading: %.1f deg, rate: %.1f deg/s\n", rotation, rotationRate);
    lastHeadingPrint = millis();
  }

  #endif
}

#else 

void setup() {
  Serial.begin(115200);
  delay(5000);
  Motor1.init();
  Motor2.init();
  Motor3.init();
  Serial.println("Running motor testing...");
}

void loop() {
  digitalWrite(Motor1.fpin, LOW);
  digitalWrite(Motor1.rpin, HIGH);
  analogWrite(Motor1.ctrl, 75);

  digitalWrite(Motor2.fpin, LOW);
  digitalWrite(Motor2.rpin, HIGH);
  analogWrite(Motor2.ctrl, 75);

  digitalWrite(Motor3.fpin, LOW);
  digitalWrite(Motor3.rpin, HIGH);
  analogWrite(Motor3.ctrl, 75);
}

#endif
