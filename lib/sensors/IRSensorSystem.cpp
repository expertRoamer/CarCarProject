#include "PinManager.h"
#include <Arduino.h>

int values[5] = {0, 0, 0, 0, 0};
double runningSum[5] = {0, 0, 0, 0, 0};
double w[5] = {-3, -1, 0, 1, 3};
int count = 0;

void readIRValues() {
    values[0] = analogRead(IR_LEFT);
    values[1] = analogRead(IR_LEFT_CENTER);
    values[2] = analogRead(IR_CENTER);
    values[3] = analogRead(IR_RIGHT_CENTER);
    values[4] = analogRead(IR_RIGHT);
    count++;
    for (int i = 0; i < 5; i++) {
        runningSum[i] += values[i];
    }
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

void printAvg() {
    for (int i = 0; i < 5; i++) {
        Serial.print(runningSum[i] / count);
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