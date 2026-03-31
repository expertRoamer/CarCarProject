#ifndef BLUETOOTHSYSTEM_H
#define BLUETOOTHSYSTEM_H

bool waitForResponse(const char *expected, unsigned long timeout);

void sendATCommand(const char *command);

void BlueToothInit();

String BlueTooth();

#endif // BLUETOOTHSYSTEM_H