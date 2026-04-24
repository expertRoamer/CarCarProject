// testing point-wise transmission
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#include "PinManager.h"
#include "MotorSystem.h"
#include "IRSensorSystem.h"
#include "RFIDSystem.h"
#include "Constants.h"
#include "BluetoothSystem.h"
#include "PIDController.h"

MFRC522 *mfrc522;

PIDController IR_PID(30, 0, 80); // 40

// String path = "LBRBLBRB";
String path = ""; // path will be sent by bluetooth

bool atNode = false;
bool first = true;
double turn = 0.0;
unsigned long previousMillis = 0UL; // For time counting
unsigned long currentMillis = 0UL;
int num = 0;
int sum[5] = {};

bool BLUETOOTH_MODE = false; // ï¿½Î¨Ó¤ï¿½ï¿½ï¿½ï¿½Ò¦ï¿½

void setup()
{
	pinMode(MOTOR_PWMA, OUTPUT);
	pinMode(MOTOR_AIN1, OUTPUT);
	pinMode(MOTOR_AIN2, OUTPUT);

	pinMode(MOTOR_PWMB, OUTPUT);
	pinMode(MOTOR_BIN1, OUTPUT);
	pinMode(MOTOR_BIN2, OUTPUT);

	pinMode(IR_LEFT, INPUT);
	pinMode(IR_LEFT_CENTER, INPUT);
	pinMode(IR_CENTER, INPUT);
	pinMode(IR_RIGHT_CENTER, INPUT);
	pinMode(IR_RIGHT, INPUT);

	Serial.begin(9600);
	Serial3.begin(9600);

	SPI.begin();
	mfrc522 = new MFRC522(SS_PIN, RST_PIN);
	mfrc522->PCD_Init();

	BlueToothInit(); // Keep Commented out Unless Initializing A Brand New Module
	while (path.length() < 3)
	{
		String cmd = BlueTooth();
		for (int i = 0; i < cmd.length(); i++)
		{
			char c = cmd.charAt(i);
			if (c == 'F' || c == 'B' || c == 'L' || c == 'R')
			{
				path += c;
				Serial.print("Received initial step: ");
				Serial.println(c);
			}
		}
		delay(10);
	}
}

void loop()
{
	/*Execute any moment*/
	CardDectecting(mfrc522);
	String cmd = BlueTooth(); // bluetooth will send the next move of CarCar to it

	// Append only valid directional commands
	for (int i = 0; i < cmd.length(); i++)
	{
		char c = cmd.charAt(i); // §ì¥X³æ¤@¦r¤¸

		if (c == 'F' || c == 'B' || c == 'L' || c == 'R')
		{
			path += c;
		}
	}
	/*Execute any moment*/

	currentMillis = millis(); // get current time
	/*Execute every 10ms*/
	if (currentMillis - previousMillis >= TIME_STEP)
	{
		previousMillis = currentMillis; // update  time

		printIRValues();
		void loop()
		{
			// CardDectecting(mfrc522);
			// String cmd = ""; // ï¿½ï¿½ï¿½ï¿½ï¿½Ó¦ï¿½ï¿½Å¤ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½O
			String cmd = BlueTooth(); // ï¿½ï¿½ï¿½ï¿½ï¿½Ó¦ï¿½ï¿½Å¤ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½O

			if (!isRunning)
				return;

			printIRValues();
			Serial.println("");

			if (!BLUETOOTH_MODE)
			{
				readIRValues();

				Serial.print(", ");
				Serial.print(atNode);
				Serial.print(", ");
				Serial.print(getWeightedAvg());
				Serial.print(", ");
				if (atNode)
				{
					if (path[0] != 'B' && (getCenterIRValue() + getLeftCenterIRValue() + getRightCenterIRValue()) > 200 && getLeftIRValue() < 100 && getRightIRValue() < 100)
					{
						atNode = false;
						path.remove(0, 1);
					}
					else if (path[0] == 'B' && startPID(250, 250, getLeftIRValue(), getLeftCenterIRValue(), getRightIRValue(), getRightCenterIRValue(), getWeightedAvg()))
					{
						atNode = false;
						path.remove(0, 1);
					}
					else
					{
						runPath();
					}
				}
				else
				{
					if (getLeftIRValue() > 200 && getRightIRValue() > 200)
					{
						atNode = true;
					}
					else
					{
						double turn = IR_PID.calculate(getWeightedAvg());
						driveKinematic(NORMAL_SPEED, turn);
						turnLast = turn;
					}
				}
				if (cmd == "BT")
				{
					BLUETOOTH_MODE = true;
					drive(0, 0);
					Serial.println("****Switched to BLUETOOTH mode.****");
				}
			}
			else // ï¿½Å¤ï¿½ï¿½ï¿½Ê¼Ò¦ï¿?
			{
				if (cmd == "F")
					drive(NORMAL_SPEED, NORMAL_SPEED); // ï¿½eï¿½i
				else if (cmd == "B")
					drive(-NORMAL_SPEED, -NORMAL_SPEED); // ï¿½ï¿½h
				else if (cmd == "L")
					drive(-NORMAL_SPEED, NORMAL_SPEED); // ï¿½ï¿½ï¿½ï¿½
				else if (cmd == "R")
					drive(NORMAL_SPEED, -NORMAL_SPEED); // ï¿½kï¿½ï¿½
				else if (cmd == "S")
					drive(0, 0);
				else if (cmd == "AUTO")
				{
					BLUETOOTH_MODE = false;
					Serial.println("****Switched to AUTO mode.****");
				}
			}

			Serial.println(turnLast);
			delay(TIME_STEP);
		}

		void runPath()
		{
			char command = path.charAt(0);

			if (command == 'F')
			{
				driveKinematic(NORMAL_SPEED, 0);
			}
			else if (command == 'L')
			{
				drive(0, 150);
				turningDir = -1;
			}
			else if (command == 'R')
			{
				drive(150, 0);
				turningDir = 1;
			}
			else if (command == 'B')
			{
				back(150, 150, getCenterIRValue(), getLeftCenterIRValue(), getRightCenterIRValue(), getRightIRValue());
				turningDir = -1;
			}
			else
			{
				isRunning = false;
				drive(0, 0);
			}
		}