#include "RFIDSystem.h"
#include <Arduino.h>

// ¨®¨®¶Çµ¹¹q¸£¥ÎSerial3
// ¹q¸£¶Çµ¹¨®¨®¥ÎSerial
void CardDectecting(MFRC522 *mfrc522)
{
    // 1. ÀË¬d¬O§_¦³·s¥d¤ù
    if (!mfrc522->PICC_IsNewCardPresent())
    {
        return;
    }

    // 2. ¹Á¸ÕÅª¨ú¥d¤ù¸ê®Æ
    if (!mfrc522->PICC_ReadCardSerial())
    {
        return;
    }

    Serial.println(F("**Card Detected!**"));

    // --- ï¿½N UID ï¿½zï¿½Lï¿½Å¤ï¿½ï¿½Ç°e ---
    String uidString = "UID:";
    for (byte i = 0; i < mfrc522->uid.size; i++)
    {
        // ï¿½Nï¿½ì¤¸ï¿½ï¿½ï¿½à¦¨ 16 ï¿½iï¿½ï¿½Aï¿½pï¿½Gï¿½pï¿½ï¿½ 0x10 ï¿½É­ï¿½ 0 ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½[
        if (mfrc522->uid.uidByte[i] < 0x10)
            uidString += "0";
        uidString += String(mfrc522->uid.uidByte[i], HEX);

        // if (i < mfrc522->uid.size - 1)
        // uidString += " "; // ï¿½ì¤¸ï¿½Õ¤ï¿½ï¿½ï¿½ï¿½[ï¿½Å®ï¿½
    }
    uidString.toUpperCase(); // ï¿½à¦¨ï¿½jï¿½gï¿½ï¿½Kï¿½\Åª

    // ï¿½zï¿½Lï¿½Å¤ï¿½ï¿½oï¿½eï¿½ï¿½ï¿½qï¿½ï¿½
    // Serial3.println(F("**Card Detected!**"));
    Serial3.println(uidString);

    // ï¿½Pï¿½É¦bï¿½qï¿½ï¿½ï¿½Ç¦Cï¿½ï¿½Ê±ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ü¡Aï¿½ï¿½Kï¿½ï¿½ï¿½ï¿½
    Serial.print("Sending to Bluetooth: ");
    Serial.println(uidString);
    // ---------------------------------------

    mfrc522->PICC_HaltA();      // ï¿½ï¿½ï¿½dï¿½ï¿½ï¿½iï¿½Jï¿½ï¿½ï¿½ï¿½Ò¦ï¿?
    mfrc522->PCD_StopCrypto1(); // ï¿½ï¿½ï¿½ï¿½[ï¿½Kï¿½ï¿½ï¿½ï¿½
}
