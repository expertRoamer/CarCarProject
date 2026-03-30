#include "RFIDSystem.h"

void CardDectecting(MFRC522 *mfrc522)
{
    if (!mfrc522->PICC_IsNewCardPresent())
    {
        return;
    } // PICC_IsNewCardPresent()：是否感應到新的卡片?
    if (!mfrc522->PICC_ReadCardSerial())
    {
        return;
    } // PICC_ReadCardSerial()：是否成功讀取資料?
    Serial.println(F("**Card Detected:**"));
    mfrc522->PICC_DumpDetailsToSerial(&(mfrc522->uid)); // 讀出UID
    mfrc522->PICC_HaltA();                              // 讓同一張卡片進入停止模式(只顯示一次)
    mfrc522->PCD_StopCrypto1();                         // 停止Crypto1
}
