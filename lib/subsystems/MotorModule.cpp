//
// Created by matth on 2026/4/2.
//

#include "MotorModule.h"

#include <Arduino.h>

void MotorModule::setPinMode() {
    pinMode(pinPWM, OUTPUT);
    pinMode(pinIn1, OUTPUT);
    pinMode(pinIn2, OUTPUT);
}

void MotorModule::applySignedPWM(int pwm) const {
    if (abs(pwm) < 1) {
        stopModule();
        return;
    }

    pwm = constrain(pwm, -255, 255);
    analogWrite(pinPWM, abs(pwm));
    digitalWrite(pinIn1, pwm > 0 ? HIGH : LOW);
    digitalWrite(pinIn2, pwm > 0 ? LOW : HIGH);
}

void MotorModule::stopModule() const {
    analogWrite(pinPWM, 0);
    digitalWrite(pinIn1, LOW);
    digitalWrite(pinIn2, LOW);
}
