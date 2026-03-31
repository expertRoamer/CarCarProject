#include <Arduino.h>
#include "Constants.h"
#include "BluetoothSystem.h"

long baudRates[] = {9600, 19200, 38400, 57600, 115200, 4800, 2400, 1200, 230400};
bool moduleReady = false;

bool waitForResponse(const char *expected, unsigned long timeout)
{
    unsigned long start = millis();
    Serial3.setTimeout(timeout);
    String response = Serial3.readString();
    if (response.length() > 0)
    {
        Serial.print("HM10 Response: ");
        Serial.println(response);
    }
    return (response.indexOf(expected) != -1);
}
void sendATCommand(const char *command)
{
    Serial3.print(command);
    waitForResponse("", 1000);
}

void BlueToothInit()
{
    Serial.begin(115200); // Debug Monitor (USB)
    while (!Serial)
        ;
    Serial.println("Initializing HM-10...");

    // 1. Automatic Baud Rate Detection
    for (int i = 0; i < 9; i++)
    {
        Serial.print("Testing baud rate: ");
        Serial.println(baudRates[i]);

        Serial3.begin(baudRates[i]);
        Serial3.setTimeout(100);
        delay(100);

        // 2. Force Disconnection
        // Sending "AT" while connected forces the module to disconnect [2].
        Serial3.print("AT");

        if (waitForResponse("OK", 800))
        {
            Serial.println("HM-10 detected and ready.");
            moduleReady = true;
            break;
        }
        else
        {
            Serial3.end();
            delay(100);
        }
    }

    if (!moduleReady)
    {
        Serial.println("Failed to detect HM-10. Check 3.3V VCC and wiring.");
        return;
    }

    // 3. Restore Factory Defaults
    Serial.println("Restoring factory defaults...");
    sendATCommand("AT+RENEW"); // Restores all setup values
    delay(500);

    // 4. Set Custom Name via Macro
    Serial.print("Setting name to: ");
    Serial.println(CARCAR_NAME);
    String nameCmd = "AT+NAME" + String(CARCAR_NAME);
    sendATCommand(nameCmd.c_str()); // Max length is 12

    // 5. Enable Connection Notifications
    Serial.println("Enabling notifications...");
    sendATCommand("AT+NOTI1"); // Notify when link is established/lost

    // 6. Get the Bluetooth MAC Address
    Serial.println("Querying Bluetooth Address");
    sendATCommand("AT+ADDR?");

    // 7. Restart the module to apply changes
    Serial.println("Restarting module...");
    sendATCommand("AT+RESET"); // Restart the module
    delay(1000);
    Serial3.begin(9600); // Now the module would use baudrate 9600

    Serial.println("Initialization Complete.");
}
String BlueTooth()
{
    String command = "";

    // 1. CarCar to PC: Forward HM-10 responses to the Serial Monitor
    if (Serial3.available())
    {
        command = Serial3.readString();
        command.trim(); // 【重要】去除掉 \r 或 \n，否則 if ("F") 會判斷失敗

        // 偵錯用：在電腦螢幕看到收到了什麼
        Serial.print("Bluetooth Received: [");
        Serial.print(command);
        Serial.println("]");
    }

    // 2. PC to CarCar: Read user input and truncate line endings
    if (Serial.available())
    {
        static String pcInputBuffer = "";
        while (Serial.available())
        {
            char c = Serial.read();
            if (c == '\r' || c == '\n')
            {
                if (pcInputBuffer.length() > 0)
                {
                    Serial3.print(pcInputBuffer); // 傳送給藍牙模組
                    Serial.print("\n[PC Command Sent to HM-10: ");
                    Serial.print(pcInputBuffer);
                    Serial.println("]");

                    // 如果想讓電腦輸入的指令能控制車子，可以取消下面這行的註解
                    // command = pcInputBuffer;

                    pcInputBuffer = "";
                }
            }
            else
            {
                pcInputBuffer += c;
            }
        }
    }

    return command; // 回傳收到的藍牙指令（如果沒有則是空字串）
}