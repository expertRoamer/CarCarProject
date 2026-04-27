#ifndef RFIDSYSTEM_H
#define RFIDSYSTEM_H
#include <SPI.h>
#include <MFRC522.h>

bool CardDectecting(MFRC522 *);

void resendUID();

#endif // RFIDSYSTEM_H