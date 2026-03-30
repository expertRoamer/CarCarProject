#include "PinManager.h"
#include <Arduino.h>
#include "IRSensorSystem.h"

int values[5] = {0, 0, 0, 0, 0};
double w[5] = {-3, -1, 0, 1, 3};

double IR_Low[] = {46.78, 25.79, 29.77, 32.45, 27.80};
double IR_High[] = {653.20, 610.80, 594.36, 656.48, 581.25};

void readIRValues() {
    values[0] = linear(analogRead(IR_LEFT), IR_Low[0], IR_High[0]);
    values[1] = linear(analogRead(IR_LEFT_CENTER), IR_Low[1], IR_High[1]);
    values[2] = linear(analogRead(IR_CENTER), IR_Low[2], IR_High[2]);
    values[3] = linear(analogRead(IR_RIGHT_CENTER), IR_Low[3], IR_High[3]);
    values[4] = linear(analogRead(IR_RIGHT), IR_Low[4], IR_High[4]);
}

void printIRValues() {
    for (int i = 0; i < 5; i++) {
        Serial.print(values[i]);
        if (i < 4) {
            Serial.print(" ");
        }
    }
    Serial.println();
}

double getWeightedAvg() {
    double sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += values[i] * w[i];
    }
    return sum / (values[0] + values[1] + values[2] + values[3] + values[4]);
}

int getLeftIRValue() {
    return values[0];
}

int getLeftCenterIRValue() {
    return values[1];
}

int getCenterIRValue() {
    return values[2];
}

int getRightCenterIRValue() {
    return values[3];
}

int getRightIRValue() {
    return values[4];
}

void getAverage(int sum[5]) {
    for (int i = 0; i < 5; i++) {
        sum[i] += values[i];
    }
}

int linear(int x, double low, double high) {
    return 50 + (x - low) / (high - low) * (550);
}