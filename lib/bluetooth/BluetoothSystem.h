#ifndef BLUETOOTHSYSTEM_H
#define BLUETOOTHSYSTEM_H
#include <Arduino.h>

bool waitForResponse(const char *expected, unsigned long timeout);

void sendATCommand(const char *command);

void BlueToothInit();

void BlueTooth();
#endif // BLUETOOTHSYSTEM_H