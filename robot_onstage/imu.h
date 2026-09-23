#include <Arduino.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#define IMU_CALIB_SAMPLES 50
#define IMU_CALIB_DELAY 10
#define IMU_SAMPLE_US 10000UL

extern Adafruit_BNO055 bno;

extern float rotation;
extern float rotationRate;
extern float zeroRot;

bool initIMU();    
void updateRotation();
bool calibrateRotation(); // blocks for 500ms