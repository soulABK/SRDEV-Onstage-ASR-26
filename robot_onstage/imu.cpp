#include "imu.h"

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

float rotation = 0.0f;
float rotationRate = 0.0f;
float zeroRot = 0.0f;

static uint32_t lastSample_us = 0;

#define ROT_RATE_ALPHA 0.2f

static float wrap180(float deg) {
  while (deg > 180.0f) deg -= 360.0f;
  while (deg <= -180.0f) deg += 360.0f;
  return deg;
}

static float readRawHeading() {
  sensors_event_t event;
  bno.getEvent(&event);
  return (float)event.orientation.x; // [0, 360)
}

bool initIMU() {
  // imu plus - gyro/accel no magnetometer
  if (!bno.begin(OPERATION_MODE_IMUPLUS)) {
    return false;
  }

  lastSample_us = 0;
  return true;
}

void updateRotation() {
  uint32_t now = micros();
  
  if (lastSample_us != 0 && (now - lastSample_us) < IMU_SAMPLE_US) return;

  float dt = (lastSample_us == 0) ? 0.0f : (float)(now - lastSample_us) * 1e-6f;
  lastSample_us = now;

  float prev = rotation;
  rotation = wrap180(readRawHeading() - zeroRot);

  if (dt > 0.0f) {
    float d = wrap180(rotation - prev) / dt;
    rotationRate += ROT_RATE_ALPHA * (d - rotationRate);
  }
}

bool calibrateRotation() {
  float sumSin = 0.0f, sumCos = 0.0f;

  for (int i = 0; i < IMU_CALIB_SAMPLES; i++) {
    float h = radians(readRawHeading());
    sumSin += sinf(h);
    sumCos += cosf(h);
    delay(IMU_CALIB_DELAY);
  }

  if (fabsf(sumSin) < 1e-3f && fabsf(sumCos) < 1e-3f) {
    return false;
  }

  zeroRot = degrees(atan2f(sumSin, sumCos));

  rotation = 0.0f;
  rotationRate = 0.0f;
  lastSample_us = 0;
  return true;
}
