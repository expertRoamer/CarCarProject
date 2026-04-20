#ifndef IRSENSOR_SYSTEM_H
#define IRSENSOR_SYSTEM_H




void readIRValues();

void printIRValues();

double getWeightedAvg();

int getLeftIRValue();

int getLeftCenterIRValue();

int getCenterIRValue();

int getRightCenterIRValue();

int getRightIRValue();

void getAverage(int[]);

int linear(int, double, double);

void correction();

void printRawValues();

#endif // IRSENSOR_SYSTEM_H
