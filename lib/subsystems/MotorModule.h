//
// Created by matth on 2026/4/2.
//

#ifndef CARCARPROJECT_MOTORMODULE_H
#define CARCARPROJECT_MOTORMODULE_H

#include "IConnectedDevice.h"

class MotorModule : IConnectedDevice {
public:
    MotorModule(const int pinPWM, const int pinIn1, const int pinIn2)
        : pinPWM(pinPWM), pinIn1(pinIn1), pinIn2(pinIn2) {}

    void setPinMode() override;

    void applySignedPWM(int pwm) const;

    void stopModule() const;

private:
    const int pinPWM, pinIn1, pinIn2;
};


#endif //CARCARPROJECT_MOTORMODULE_H