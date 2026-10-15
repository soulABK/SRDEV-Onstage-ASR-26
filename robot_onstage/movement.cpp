#include "movement.h"
using namespace std;

#define MOTOR_KFF (255.0f / (float)MAX_RPM)
#define MOTOR_KP  0.02f
#define MOTOR_KI  0.05f
#define MAX_I_PWM 80.0f                  
#define MAX_I     (MAX_I_PWM / MOTOR_KI) 
#define FULL_SCALE 100.0f            

// //set motor pins
Motor Motor1{8, 9, 7, 16, 17}, Motor2{12, 13, 14, 18, 19}, Motor3{10, 11, 5, 20, 21};
// // motor 1 and 2 are the left/right motors; motor 2 should be clockwise of motor 1, back motor should be motor 3

//motor functions
//motor mapping
int cmap(int val, int olow, int ohigh, int mlow, int mhigh) {
  return constrain(map(val, olow, ohigh, mlow, mhigh), mlow, mhigh);
}

void Motor::init() {
  pinMode(fpin, OUTPUT);
  pinMode(rpin, OUTPUT);
  pinMode(ctrl, OUTPUT);
#ifdef USE_VELOCITY_PID
  enc.begin(enc_a);
#endif
  lastRPMCalculationTime = micros();
  lastPulses = 0;
}

//motor speed
void Motor::speed(float val) {
  if (fabsf(val) < MIN_COMMAND) {
    e_i = 0.0f;
    output = 0.0f;
    last_commanded_rpm = 0.0;
    last_dir = 0;
    digitalWrite(fpin, LOW);
    digitalWrite(rpin, LOW);
    analogWrite(ctrl, 0);
    return;
  }

  int dir = (val > 0.0f) ? 1 : -1;

#ifdef USE_VELOCITY_PID
  float dt = recalculateRPM();
  double commanded_rpm = (double)fabsf(val) / FULL_SCALE * MAX_RPM;

  if (dir != last_dir) {
    e_i = 0.0f;
  }
  if (dir != last_dir || fabs(commanded_rpm - last_commanded_rpm) > 500.0) {
    float ff_seed = MOTOR_KFF * (float)commanded_rpm;
    output = constrain(ff_seed, 0.0f, 255.0f);
  }
  last_commanded_rpm = commanded_rpm;

  if (dt > 0.0f) {
    float error = (float)(commanded_rpm - currentRPM);
    float ff = MOTOR_KFF * (float)commanded_rpm;
    float p  = MOTOR_KP * error;

    float trial = ff + p + MOTOR_KI * (e_i + error * dt);
    if (trial > 0.0f && trial < 255.0f) {
      e_i += error * dt;
    }
    e_i = constrain(e_i, -MAX_I, MAX_I);

    float u = ff + p + MOTOR_KI * e_i;
    output = constrain(u, 0.0f, MOTOR_MAX_PWM);
  }
#else
  float pwm = fabsf(val) * (MOTOR_MAX_PWM / FULL_SCALE);
  output = constrain(pwm, 0.0f, MOTOR_MAX_PWM);
  last_commanded_rpm = (double)fabsf(val) / FULL_SCALE * MAX_RPM; // telemetry only
#endif

  last_dir = dir;

  if (output > 0.0f && output < PWM_STICTION) {
    output = PWM_STICTION;
  }

  if (dir > 0) {
    digitalWrite(fpin, HIGH);
    digitalWrite(rpin, LOW);
  } else {
    digitalWrite(fpin, LOW);
    digitalWrite(rpin, HIGH);
  }
  analogWrite(ctrl, (int)output);
}

// return dt solely for the purpose of integration in motor control
float Motor::recalculateRPM() {
#ifndef USE_VELOCITY_PID
  return 0.0f;
#else
  enc.update();
  uint32_t us = micros();
  uint32_t dt = us - lastRPMCalculationTime;

  if (dt > 50000) { // 50ms window
    long curPulses = enc.step;

    long deltaPulses = curPulses - lastPulses;
    currentRPM = ((double)labs(deltaPulses)*60.0) / (PPR * ((double)dt / 1000000.0));

    lastRPMCalculationTime = us;
    lastPulses = curPulses;

    return (float)dt / 1000000.0;
  }

  return 0.0;
#endif
}

//function: set motor speeds
static uint32_t lastPrintTime = 0.0;
void motor(float speed1, float speed2, float speed3) {
  // obsolete factors for natural motor deficiency correction
  // float fmults[] = {1.4, 1.2, 1};
  // float bmults[] = {1.5, 1.5, 1};
  
  // Motor1.speed(speed1*((speed1>0) ? fmults[0] : bmults[0]));
  // Motor2.speed(speed2*((speed2>0) ? fmults[1] : bmults[1]));
  // Motor3.speed(speed3*((speed3>0) ? fmults[2] : bmults[2]));

  Motor1.speed(speed1);
  Motor2.speed(speed2);
  Motor3.speed(speed3);

  if (millis() - lastPrintTime > 250) {
    Serial.printf("cmd %6.1f %6.1f %6.1f | pwm %5.0f %5.0f %5.0f\n",
                  speed1, speed2, speed3,
                  Motor1.output, Motor2.output, Motor3.output);
    lastPrintTime = millis();
  }
}

float degtorad(int degrees) {
  float radians = degrees * PI / 180;
  return radians;
}

//function: set motor speeds based on velocity
const float radius = 0.346; //ft
const float ftsToSpeed = 157.65;

const float ROTATION_KP = 2.5f;   
const float ROTATION_KD = 0.06f; 
const float ROTATION_MAX = 25.0f; 
const float ROTATION_SIGN = 1.0f;

// treat velocities as being in world frame
const bool  FIELD_ORIENTED = true;
const float FIELD_SIGN = 1.0f; // can flip in case imu bugs out

void vmotor(float Vx, float Vy, float headingError, float headingRate) {
  // correction for translational offset caused by heading correction
  if (FIELD_ORIENTED) {
    float th = FIELD_SIGN * radians(headingError);
    float c = cosf(th), s = sinf(th);
    float bx = Vx * c - Vy * s;
    float by = Vx * s + Vy * c;
    Vx = bx;
    Vy = by;
  }

  float s1 = ftsToSpeed * (-Vx/2 - sqrt(3)*Vy/2);
  float s2 = ftsToSpeed * (-Vx/2 + sqrt(3)*Vy/2);
  float s3 = ftsToSpeed * (Vx);

  //float yaw = ROTATION_SIGN * (ROTATION_KP * headingError + ROTATION_KD * headingRate);
  // no kd needed i guess
  float yaw = ROTATION_SIGN * (ROTATION_KP * headingError);
  yaw = constrain(yaw, -ROTATION_MAX, ROTATION_MAX);

  s1 += yaw;
  s2 += yaw;
  s3 += yaw;

  // Serial.print("Left motor: ");
  // Serial.println(s1);
  // Serial.print("Right motor: ");
  // Serial.println(s2);
  // Serial.print("Back motor: ");
  // Serial.println(s3);
  motor(s1, s2, s3);
}

