#include "MotorSystem.h"
#include "Constants.h"

#include "PinManager.h"
#include <Arduino.h>

double currentLeftSpeed = 0;
double currentRightSpeed = 0;

bool reachMin = false;

void driveKinematic(double speed, double turn) {
    double vL = speed + turn;
    double vR = speed - turn;

    if (abs(vL - vR) > 510) {
        if (vL - vR > 0) {
            vL = 255;
            vR = -255;
        } else {
            vL = -255;
            vR = 255;
        }
    }

    if (abs(vL) > 255) {
        double shift = abs(vL) - 255;
        vR -= vL > 0 ? shift : -shift;
        vL = vL > 0 ? 255 : -255;
    }

    if (abs(vR) > 255) {
        double shift = abs(vR) - 255;
        vL -= vR > 0 ? shift : -shift;
        vR = vR > 0 ? 255 : -255;
    }

    driveHard(vL, vR);
}

// ���k������
void drive(double vL, double vR)
{
    driveLeft(vL);
    driveRight(vR);
    // driveLeft(currentLeftSpeed + min(vL - currentLeftSpeed, 10));
    // driveRight(currentRightSpeed + min(vR - currentRightSpeed, 10));
    // currentLeftSpeed += min(vR - currentLeftSpeed, 10);
    // currentRightSpeed += min(vR - currentRightSpeed, 10);
}

void driveHard(double vL, double vR)
{
    driveLeft(vL);
    driveRight(vR);
}

// �o�ӷ|delay�A��ĳ���n��
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

void driveIndividual(int pwm, int in1, int in2, double v) {
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

void back(int left_bound, int right_bound, int leftIR, int left_centerIR, int rightIR, int right_centerIR)
{
    if ((leftIR + left_centerIR > left_bound) && (rightIR + right_centerIR < right_bound))
        drive(NORMAL_SPEED, NORMAL_SPEED);
    else
        drive(-70, 70);
}

bool startPID(int left_bound, int right_bound, int leftIR, int left_centerIR, int rightIR, int right_centerIR, double weightedAvg)
{
    if (reachMin) {
        bool output = weightedAvg >= -0.5;
        if (output) reachMin = false;
        return output;
    }

    if (weightedAvg <= LEAVE_NODE_THRESHOLD) {
        reachMin = true;
    }

    return false;

    // return ((leftIR + left_centerIR > left_bound) && (rightIR + right_centerIR < right_bound));
}

