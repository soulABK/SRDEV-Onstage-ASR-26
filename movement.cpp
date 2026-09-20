#include "movement.h"
using namespace std;

#define MOTOR_KFF (255.0f / (float)MAX_RPM)
#define MOTOR_KP  0.02f
#define MOTOR_KI  0.05f
#define MAX_I_PWM 80.0f                  
#define MAX_I     (MAX_I_PWM / MOTOR_KI) 
#define FULL_SCALE 100.0f            

//set motor pins
Motor Motor1{8, 9, 7, 16, 17}, Motor2{12, 13, 14, 18, 19}, Motor3{10, 11, 6, 20, 21};
// motor 1 and 2 are the left/right motors; motor 2 should be clockwise of motor 1, back motor should be motor 3

//motor functions
//motor mapping
int cmap(int val, int olow, int ohigh, int mlow, int mhigh) {
  return constrain(map(val, olow, ohigh, mlow, mhigh), mlow, mhigh);
}

void Motor::init() {
  pinMode(fpin, OUTPUT);
  pinMode(rpin, OUTPUT);
  pinMode(ctrl, OUTPUT);
  enc.begin(enc_a);
  lastRPMCalculationTime = micros();
  lastPulses = 0;
}

//motor speed
void Motor::speed(float val) {
  float dt = recalculateRPM();

  if (fabsf(val) < 1.0f) {
    // brake
    e_i = 0.0f;
    output = 0.0f;
    last_commanded_rpm = 0.0;
    last_dir = 0;
    digitalWrite(fpin, LOW);
    digitalWrite(rpin, LOW);
    digitalWrite(ctrl, HIGH);
    return;
  }

  double commanded_rpm = (double)fabsf(val) / FULL_SCALE * MAX_RPM;
  int dir = (val > 0.0f) ? 1 : -1;

  if (dir != last_dir || fabs(commanded_rpm - last_commanded_rpm) > 100.0) {
    e_i = 0.0f;
    float ff_seed = MOTOR_KFF * (float)commanded_rpm;
    output = constrain(ff_seed, 0.0f, 255.0f);
  }
  last_commanded_rpm = commanded_rpm;
  last_dir = dir;

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
    output = constrain(u, 0.0f, 255.0f);
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
}

//function: set motor speeds
static uint32_t lastPrintTime = 0.0;
void motor(float speed1, float speed2, float speed3) {
  // float fmults[] = {1.4, 1.2, 1};
  // float bmults[] = {1.5, 1.5, 1};
  
  // Motor1.speed(speed1*((speed1>0) ? fmults[0] : bmults[0]));
  // Motor2.speed(speed2*((speed2>0) ? fmults[1] : bmults[1]));
  // Motor3.speed(speed3*((speed3>0) ? fmults[2] : bmults[2]));

  Motor1.speed(speed1);
  Motor2.speed(speed2);
  Motor3.speed(speed3);

  if (millis() - lastPrintTime > 500.0) {
    Serial.printf("Target RPM 1: %f, Actual RPM 1: %lf, I: %f, Command: %f, PWM: %f\n", Motor1.last_commanded_rpm, Motor1.currentRPM, Motor1.e_i, speed1, Motor1.output);
    Serial.printf("Target RPM 2: %f, Actual RPM 2: %lf, I: %f, Command: %f, PWM: %f\n", Motor2.last_commanded_rpm, Motor2.currentRPM, Motor2.e_i, speed2, Motor2.output);
    Serial.printf("Target RPM 3: %f, Actual RPM 3: %lf, I: %f, Command: %f, PWM: %f\n", Motor3.last_commanded_rpm, Motor3.currentRPM, Motor3.e_i, speed3, Motor3.output);
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

const float ROTATION_KP = 0.3;

void vmotor(float Vx, float Vy, float rotation) {
  static float prev_rot;

  // int s1 = ftsToSpeed * (-Vx/2 - sqrt(3)*Vy/2 - radius * degtorad(rotation) + degtorad(prev_rot)*Kd);
  // int s2 = ftsToSpeed * (-Vx/2 + sqrt(3)*Vy/2 - radius * degtorad(rotation) + degtorad(prev_rot)*Kd);
  // int s3 = ftsToSpeed * (Vx - radius * degtorad(rotation) + degtorad(prev_rot)*Kd);

  float s1 = ftsToSpeed * (-Vx/2 - sqrt(3)*Vy/2);
  float s2 = ftsToSpeed * (-Vx/2 + sqrt(3)*Vy/2);
  float s3 = ftsToSpeed * (Vx);

  s1 += degtorad(rotation) * ROTATION_KP;
  s2 += degtorad(rotation) * ROTATION_KP;
  s3 += degtorad(rotation) * ROTATION_KP;

  float a1 = fabsf(s1), a2 = fabsf(s2), a3 = fabsf(s3);
  float peak = a1;
  if (a2 > peak) peak = a2;
  if (a3 > peak) peak = a3;

  if (peak > FULL_SCALE) {
    float k = FULL_SCALE / peak;
    s1 *= k;
    s2 *= k;
    s3 *= k;
  }

  // Serial.print("Left motor: ");
  // Serial.println(s1);
  // Serial.print("Right motor: ");
  // Serial.println(s2);
  // Serial.print("Back motor: ");
  // Serial.println(s3);
  motor(s1, s2, s3);
}
