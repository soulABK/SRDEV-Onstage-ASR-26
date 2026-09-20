#include "imu.h"

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire1);

int rotation = 0;
int zeroRot = 0;

void updateRotation() {
  sensors_event_t event;
  bno.getEvent(&event);
  int raw = ((int) event.orientation.x);
  rotation = raw + zeroRot;
  rotation = (rotation + 360) % 360;
  rotation = (rotation > 180) ? (rotation - 360) : rotation; 
}

bool calibrateRotation() {
  if (!bno.begin()) {
    return false;
  }
  int totalRot = 0;
  for (int i = 0; i < IMU_CALIB_SAMPLES; i++) {
    updateRotation();
    totalRot += rotation;
    delay(IMU_CALIB_DELAY);
  }
  zeroRot = (int)round(-(float)totalRot / IMU_CALIB_SAMPLES);
  return true;
}
