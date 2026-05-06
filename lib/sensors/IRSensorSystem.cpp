#include "PinManager.h"
#include <Arduino.h>
#include "IRSensorSystem.h"

int values[5] = {0, 0, 0, 0, 0};
double w[5] = {-2, -1, 0, 1, 2};

unsigned long lastAtNode = 0;
double IR_Low[] = {28, 29, 28, 30, 31};
double IR_High[] = {393, 370, 394, 461, 501};

void readIRValues()
{
    values[0] = linear(analogRead(IR_LEFT), IR_Low[0], IR_High[0]);
    values[1] = linear(analogRead(IR_LEFT_CENTER), IR_Low[1], IR_High[1]);
    values[2] = linear(analogRead(IR_CENTER), IR_Low[2], IR_High[2]);
    values[3] = linear(analogRead(IR_RIGHT_CENTER), IR_Low[3], IR_High[3]);
    values[4] = linear(analogRead(IR_RIGHT), IR_Low[4], IR_High[4]);
}

void printRawValues()
{
    Serial.print(analogRead(IR_LEFT));
    Serial.print(", ");
    Serial.print(analogRead(IR_LEFT_CENTER));
    Serial.print(", ");
    Serial.print(analogRead(IR_CENTER));
    Serial.print(", ");
    Serial.print(analogRead(IR_RIGHT_CENTER));
    Serial.print(", ");
    Serial.print(analogRead(IR_RIGHT));
}

void printIRValues()
{
    String IRvalue = "";
    for (int i = 0; i < 5; i++)
    {
        IRvalue += String(values[i]);
        if (i < 4)
        {
            IRvalue += ", ";
        }
    }
    IRvalue += "";
    Serial.print(IRvalue);
}
void printIRValuesBT()
{
    String IRvalue = "";
    for (int i = 0; i < 5; i++)
    {
        IRvalue += String(values[i]);
        if (i < 4)
        {
            IRvalue += ", ";
        }
    }
    IRvalue += "";
    Serial3.println(IRvalue);
}

double getWeightedAvg()
{
    double sum = 0;
    for (int i = 0; i < 5; i++)
    {
        sum += values[i] * w[i];
    }
    return sum / (values[0] + values[1] + values[2] + values[3] + values[4]);
}

int getLeftIRValue()
{
    return values[0];
}

int getLeftCenterIRValue()
{
    return values[1];
}

int getCenterIRValue()
{
    return values[2];
}

int getRightCenterIRValue()
{
    return values[3];
}

int getRightIRValue()
{
    return values[4];
}

void getAverage(int sum[5])
{
    for (int i = 0; i < 5; i++)
    {
        sum[i] += values[i];
    }
}

int linear(int x, double low, double high)
{
    return 50 + (x - low) / (high - low) * (550);
}

bool atNodeIR()
{
    int threshold = 250;
    bool leftthree = getLeftIRValue() > threshold && getLeftCenterIRValue() > threshold && getCenterIRValue() > threshold;
    bool centerthree = getCenterIRValue() > threshold && getLeftCenterIRValue() > threshold && getRightCenterIRValue() > threshold;
    bool rightthree = getRightCenterIRValue() > threshold && getCenterIRValue() > threshold && getRightIRValue() > threshold;

    if ((millis() - lastAtNode >= 200 || lastAtNode == 0) && (leftthree || centerthree || rightthree))
    {
        lastAtNode = millis();
        return true;
    }

    return false;
}
