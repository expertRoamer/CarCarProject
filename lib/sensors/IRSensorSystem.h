#ifndef IRSENSOR_SYSTEM_H
#define IRSENSOR_SYSTEM_H

void readIRValues();

void printIRValues();

void printAvg();

double getWeightedAvg();

int getLeftIRValue();

int getLeftCenterIRValue();

int getCenterIRValue();

int getRightCenterIRValue();

int getRightIRValue();

#endif // IRSENSOR_SYSTEM_H