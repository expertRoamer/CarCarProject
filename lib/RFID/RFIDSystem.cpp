#include "RFIDSystem.h"
#include <Arduino.h>

// �����ǵ��q����Serial3
// �q���ǵ�������Serial
void CardDectecting(MFRC522 *mfrc522)
{
    // 1. �ˬd�O�_���s�d��
    if (!mfrc522->PICC_IsNewCardPresent())
    {
        return;
    }

    // 2. ����Ū���d�����
    if (!mfrc522->PICC_ReadCardSerial())
    {
        return;
    }

    Serial.println(F("**Card Detected!**"));

    // --- �N UID �z�L�Ť��ǰe ---
    String uidString = "UID:";
    for (byte i = 0; i < mfrc522->uid.size; i++)
    {
        // �N�줸���ন 16 �i��A�p�G�p�� 0x10 �ɭ� 0 ������[
        if (mfrc522->uid.uidByte[i] < 0x10)
            uidString += "0";
        uidString += String(mfrc522->uid.uidByte[i], HEX);

        // if (i < mfrc522->uid.size - 1)
            // uidString += " "; // �줸�դ����[�Ů�
    }
    uidString.toUpperCase(); // �ন�j�g��K�\Ū

    // �z�L�Ť��o�e���q��
    // Serial3.println(F("**Card Detected!**"));
    Serial3.println(uidString);

    // �P�ɦb�q���ǦC��ʱ�������ܡA��K����
    Serial.print("Sending to Bluetooth: ");
    Serial.println(uidString);
    // ---------------------------------------

    mfrc522->PICC_HaltA();      // ���d���i�J����Ҧ�
    mfrc522->PCD_StopCrypto1(); // ����[�K����
}
