#include "MotorSystem.h"

#include "PinManager.h"
#include <Arduino.h>

void drive(double vL, double vR, int time) {
    driveLeft(vL);
    driveRight(vR);
    delay(time);
    driveLeft(0);
    driveRight(0);
}

void driveLeft(double v) {
    driveIndividual(MOTOR_PWMA, MOTOR_AIN1, MOTOR_AIN2, v);
}

void driveRight(double v) {
    v *= 0.966; // scaling adjustment for right motor
    driveIndividual(MOTOR_PWMB, MOTOR_BIN1, MOTOR_BIN2, v);
}

void driveIndividual(int pwm, int in1, int in2, double v) {
    if (abs(v) < 1) {
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        return;
    }

    analogWrite(pwm, abs(v));
    digitalWrite(in1, v > 0 ? HIGH : LOW);
    digitalWrite(in2, v > 0 ? LOW : HIGH);
}