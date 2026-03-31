#include "RFIDSystem.h"
#include <Arduino.h>

void CardDectecting(MFRC522 *mfrc522)
{
    // 1. 檢查是否有新卡片
    if (!mfrc522->PICC_IsNewCardPresent())
    {
        return;
    }

    // 2. 嘗試讀取卡片資料
    if (!mfrc522->PICC_ReadCardSerial())
    {
        return;
    }

    Serial.println(F("**Card Detected!**"));

    // --- 將 UID 透過藍牙傳送 ---
    String uidString = "UID:";
    for (byte i = 0; i < mfrc522->uid.size; i++)
    {
        // 將位元組轉成 16 進位，如果小於 0x10 補個 0 比較美觀
        if (mfrc522->uid.uidByte[i] < 0x10)
            uidString += "0";
        uidString += String(mfrc522->uid.uidByte[i], HEX);

        if (i < mfrc522->uid.size - 1)
            uidString += " "; // 位元組之間加空格
    }
    uidString.toUpperCase(); // 轉成大寫方便閱讀

    // 透過藍牙發送給手機/電腦
    Serial3.println(uidString);

    // 同時在電腦序列埠監控視窗顯示，方便除錯
    // Serial.print("Sending to Bluetooth: ");
    Serial.println(uidString);
    // ---------------------------------------

    mfrc522->PICC_HaltA();      // 讓卡片進入停止模式
    mfrc522->PCD_StopCrypto1(); // 停止加密驗證
}
