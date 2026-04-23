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

PIDController IR_PID(150, 0.0, 0.); // 80

// String path = "FRBRBRB";
String path = ""; // path will be sent by bluetooth

bool atNode = false;
bool first = true;
double turnLast = 0.0;
unsigned long previousMillis = 0UL; // For time counting
unsigned long currentMillis = 0UL;
int num = 0;
int sum[5] = {};

void runPath();

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

	// BlueToothInit(); // Keep Commented out Unless Initializing A Brand New Module
	while (path.length() < 3) // Take in three steps first
	{
		String cmd = BlueTooth();
		if (cmd == "F" || cmd == "B" || cmd == "L" || cmd == "R")
		{
			path += cmd;
			Serial.print("Received initial step: ");
			Serial.println(cmd);
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
	if (cmd == "F" || cmd == "B" || cmd == "L" || cmd == "R")
	{
		path += cmd;
	}
	/*Execute any moment*/

	currentMillis = millis(); // get current time
	/*Execute every 10ms*/
	if (currentMillis - previousMillis >= TIME_STEP)
	{
		previousMillis = currentMillis; // update  time

		printIRValues();
		readIRValues();
		if (atNode)
		{
			if (path.length() == 0)
			{
				drive(0, 0);
				delay(TIME_STEP);
				return;
			}

			// Check if intersection clear condition is met (for F, L, R)
			if ((getCenterIRValue() + getLeftCenterIRValue() + getRightCenterIRValue()) > 200 && getLeftIRValue() < 100 && getRightIRValue() < 100)
			{
				atNode = false;
				Serial3.println("STEP_DONE"); // Send ACK to Python
				path.remove(0, 1);			  // Remove completed step
			}
			// Check if leaving node (for Backward)
			else if (path[0] == 'B' && startPID(150, 150, getLeftIRValue(), getLeftCenterIRValue(), getRightIRValue(), getRightCenterIRValue()))
			{
				atNode = false;
				Serial3.println("STEP_DONE");
				// if (path.length() > 0)
				// {
				path.remove(0, 1);
				// }
				// else
				// {
				// 	runPath();
				// }
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
			// if (cmd == "BT")  //bluetooth mode will not be needed
			// {
			// 	BLUETOOTH_MODE = true;
			// 	drive(0, 0);
			// 	Serial.println("****Switched to BLUETOOTH mode.****");
			// }
		}
		else
		{
			if (getLeftIRValue() > 200 && getRightIRValue() > 200) // Entering a Node
			{
				atNode = true;
			}
			else // Regular PID
			{
				double turn = IR_PID.calculate(getWeightedAvg());

				double currentSpeed = (path.length() > 0 && path[0] == 'F') ? NORMAL_SPEED : NORMAL_SPEED * 0.7;
				driveKinematic(currentSpeed, turn);

				turnLast = turn;
			}
		}
		// delay(TIME_STEP);
	}
}

void runPath()
{
	if (path.length() == 0)
		return; // Extra safety guard

	char command = path.charAt(0);

	if (command == 'F')
	{
		driveKinematic(NORMAL_SPEED, 0);
	}
	else if (command == 'L')
	{
		drive(-50, 255);
	}
	else if (command == 'R')
	{
		drive(255, -50);
	}
	else if (command == 'B')
	{
		back(150, 150, getCenterIRValue(), getLeftCenterIRValue(), getRightCenterIRValue(), getRightIRValue());
	}
}