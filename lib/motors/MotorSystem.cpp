#include "MotorSystem.h"
#include "Constants.h"

#include "PinManager.h"
#include <Arduino.h>

void driveKinematic(double speed, double turn)
{
    double vL = speed + turn;
    double vR = speed - turn;
    drive(vL, vR);
}

void drive(double vL, double vR)
{
    driveLeft(vL);
    driveRight(vR);
}

void driveFor(double vL, double vR, int time)
{
    driveLeft(vL);
    driveRight(vR);
    delay(time);
    driveLeft(0);
    driveRight(0);
}

void driveLeft(double v)
{
    driveIndividual(MOTOR_PWMA, MOTOR_AIN1, MOTOR_AIN2, v);
}

void driveRight(double v)
{
    v *= SPEED_ADUSTING_CONSTANT; // scaling adjustment for right motor
    driveIndividual(MOTOR_PWMB, MOTOR_BIN1, MOTOR_BIN2, v);
}

void driveIndividual(int pwm, int in1, int in2, double v)
{
    if (abs(v) < 1)
    {
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        return;
    }

    if (abs(v) > 255)
    {
        v = 255 * (v > 0 ? 1 : -1);
    }

    analogWrite(pwm, abs(v));
    digitalWrite(in1, v > 0 ? HIGH : LOW);
    digitalWrite(in2, v > 0 ? LOW : HIGH);
}