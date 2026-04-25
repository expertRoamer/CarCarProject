#include "RFIDSystem.h"
#include <Arduino.h>

String uids[20];
unsigned long timestamps[20];
int i = 0;
// String CARD = "45764D73";

bool CardDectecting(MFRC522 *mfrc522)
{
    if (!mfrc522->PICC_IsNewCardPresent())
    {
        return false;
    }

    if (!mfrc522->PICC_ReadCardSerial())
    {
        return false;
    }
    // Serial.println(F("**Card Detected!**"));

    String uidString = "UID:";
    for (byte i = 0; i < mfrc522->uid.size; i++)
    {
        if (mfrc522->uid.uidByte[i] < 0x10)
            uidString += "0";
        uidString += String(mfrc522->uid.uidByte[i], HEX);

        // if (i < mfrc522->uid.size - 1)
        // uidString += " "; // �줸�դ����[�Ů�
    }
    uidString.toUpperCase(); // �ন�j�g��K�\Ū

    // Serial3.println(F("**Card Detected!**"));
    Serial3.println(uidString);

    // Serial.print("Sending to Bluetooth: ");
    // Serial.println(uidString);

    // if (uidString == CARD)
    // {
    //     for (int j = 0; j < i; j++)
    //     {
    //         Serial.print(uidString[j]);
    //         Serial.print(", ");
    //         Serial.println(timestamps[j]);
    //     }
    // }
    // else
    {
        uids[i] = uidString;
        timestamps[i] = millis();
        i++;
    }

    // ---------------------------------------

    mfrc522->PICC_HaltA();      // ���d���i�J����Ҧ�?
    mfrc522->PCD_StopCrypto1(); // ����[�K����

    return true;
}
