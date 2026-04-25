#ifndef PIDCONTROLLER_H
#define PIDCONTROLLER_H

#include "Constants.h"

class PIDController
{
public:
    double Kp;
    double Ki;
    double Kd;

    PIDController(double Kp, double Ki, double Kd) : Kp(Kp), Ki(Ki), Kd(Kd) {}

    double calculate(double setpoint, double measurement, double time)
    {
        double error = setpoint - measurement;

        if (firstRun)
        {
            lastError = error;
            firstRun = false;
        }
        else
        {
            integral += error * time;
        }

        double derivative = (error - lastError) / time;
        // Serial.print(Kp * error);
        // Serial.print(", ");
        // Serial.print(Ki * integral);
        // Serial.print(", ");
        // Serial.print(Kd * derivative);
        lastError = error;
        return Kp * error + Ki * integral + Kd * derivative;
    }

    void reset()
    {
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