#include <Arduino.h>
#include "Constants.h"
#include "BluetoothSystem.h"

// ESP32 to PC uses Serial3
// PC to ESP32 uses Serial
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
        // Sending "AT" while connected forces the module to disconnect.
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
    sendATCommand("AT+RENEW");
    delay(500);

    // 4. Set Custom Name via Macro
    Serial.print("Setting name to: ");
    Serial.println(CARCAR_NAME);
    String nameCmd = "AT+NAME" + String(CARCAR_NAME);
    sendATCommand(nameCmd.c_str());

    // 5. Enable Connection Notifications
    Serial.println("Enabling notifications...");
    sendATCommand("AT+NOTI1");

    // 6. Get the Bluetooth MAC Address
    Serial.println("Querying Bluetooth Address");
    sendATCommand("AT+ADDR?");

    // 7. Restart the module to apply changes
    Serial.println("Restarting module...");
    sendATCommand("AT+RESET");
    delay(1000);
    Serial3.begin(9600); // Now the module uses baudrate 9600

    Serial.println("Initialization Complete.");
}

String BlueTooth()
{
    String command = "";

    // 1. ESP32 to PC: Forward HM-10 responses to the Serial Monitor
    while (Serial3.available())
    {
        char c = (char)Serial3.read();
        command += c;
    }

    if (command.length() > 0)
    {
        command.trim();

        // Debug: See what was received on the Serial Monitor
        // Serial.print("Bluetooth Received: [");
        // Serial.print(command);
        // Serial.println("]");
    }
    // 2. PC to ESP32: Read user input and truncate line endings
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
                    Serial3.print(pcInputBuffer); // Send to Bluetooth module
                    // Serial.print("\n[PC Command Sent to HM-10: ");
                    // Serial.print(pcInputBuffer);
                    // Serial.println("]");

                    pcInputBuffer = "";
                }
            }
            else
            {
                pcInputBuffer += c;
            }
        }
    }

    return command; // Return the received Bluetooth command (or empty string if none)
}