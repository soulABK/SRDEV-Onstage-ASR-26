#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#define IMU_CALIB_SAMPLES 50
#define IMU_CALIB_DELAY 10 // ms

extern Adafruit_BNO055 bno;

extern int rotation;
extern int zeroRot;

void updateRotation();
void calibrateRotation();