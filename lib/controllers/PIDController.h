#ifndef PIDCONTROLLER_H
#define PIDCONTROLLER_H

#include "Constants.h"

class PIDController {
public:
    double Kp;
    double Ki;
    double Kd;

    PIDController(double Kp, double Ki, double Kd) : Kp(Kp), Ki(Ki), Kd(Kd) {}

    double calculate(double error) {
        if (firstRun) {
            lastError = error;
            firstRun = false;
        } else {
            integral += error * TIME_STEP;
        }

        double derivative = (error - lastError) / TIME_STEP;
        Serial.print("P: ");
        Serial.print(Kp * error);
        Serial.print(" I: ");
        Serial.print(Ki * integral);
        Serial.print(" D: ");
        Serial.println(Kd * derivative);
        lastError = error;
        return Kp * error + Ki * integral + Kd * derivative;
    }

    void reset() {
        integral = 0.0;
        lastError = 0.0;
        firstRun = true;
    }

private:
    bool firstRun = true;
    double lastError = 0.0;
    double integral = 0.0;
};

#endif // PIDCONTROLLER_H