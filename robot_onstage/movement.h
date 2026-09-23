#include <Arduino.h>
#include <cmath>
#include <numbers>
#include <PicoEncoder.h>

// define an arbitrary RPM as the 100% speed
const double MAX_RPM = 7200.0; // this is actually a relatively small fraction of the theoretical max RPM but we don't wanna push the motors too hard
const float OMNIWHEEL_DIAMETER_MM = 58;
const int GEAR_RATIO = 120;
const double PPR = 64.0; // pulses per one rotation of the DC motor

//motor init prototypes
int cmap(int);
void speed(int);

//motor object
struct Motor {
  // pins
  uint8_t fpin;
  uint8_t rpin;
  uint8_t ctrl;
  uint8_t enc_a;
  uint8_t enc_b; 

  // RPM PID
  double last_commanded_rpm = 0.0;
  double currentRPM = 0.0;
  float e_i = 0.0;
  float output = 0.0;  // last PWM written; persists between the 50ms PID windows
  int last_dir = 0;    // sign of the last command, so a reversal can reset the integral

  // Encoder
  long lastPulses = 0;
  uint32_t lastRPMCalculationTime = 0;
  PicoEncoder enc;

  void init();
  void speed(float val);
  float recalculateRPM(); // curveball - this returns a dt which is used for integration in motor control
};

extern Motor Motor1;
extern Motor Motor2;
extern Motor Motor3;

void motor(float speed1, float speed2, float speed3);
void vmotor(float Vx, float Vy, float headingError, float headingRate);