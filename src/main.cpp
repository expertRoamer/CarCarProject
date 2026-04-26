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

PIDController IR_PID(50, 0, 2000); // Previous :(60, 0, 100) turn : (50, 0, 100) 1000
// PIDController TURN_PID(50, 0, 200); 60 200

String path = "FFLFBFRRLRALLFRAF"; // A: clockwise 180; B: counterclockwise 180
// FFLFBFRRLRALLFRAF
// String path = ""; // Point-wise
bool atNode = false;
bool first = true;

int initialRun = 0;
bool startReturn = false;

double turn = 0.0;
int fullRotate = 1000, halfRotate = 500;

void runPath();
int num = 0;
int sum[5] = {};

bool isRunning = true;
bool translationalBrake = false;
bool rotationalBrake = false;
int turningDir = 1; // 1: left turn brake; -1: right turn brake
unsigned long timestamp = 0;
unsigned long lastTime = 0;
unsigned long startTime = 0;

bool lockRFID = false;

bool BLUETOOTH_MODE = false; //

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

	// BlueToothInit();

	// Serial.begin(115200);
	Serial.begin(9600);
	Serial3.begin(9600);

	SPI.begin();
	mfrc522 = new MFRC522(SS_PIN, RST_PIN);
	mfrc522->PCD_Init();

	// Takes in initial 3 commands
	while (path.length() < 3)
	{
		String cmd = BlueTooth();
		for (unsigned int i = 0; i < cmd.length(); i++)
		{
			char c = cmd.charAt(i);
			if (c == 'F' || c == 'B' || c == 'L' || c == 'R' || c == 'A')
			{
				path += c;
				// Serial.print("Received initial step: ");
				// Serial.println(c);
			}
		}
		delay(10);
	}

	initialRun = 0;
	path.remove(0,1);
	Serial3.println("NX");
	drive(NORMAL_SPEED, NORMAL_SPEED);
	startTime = millis();
}

void loop()
{
	while (Serial3.available() > 0)
	{
		char c = (char)Serial3.read();

		// Append available path commend
		if (c == 'F' || c == 'B' || c == 'L' || c == 'R' || c == 'A')
		{
			path += c;
		}
	}

	if (millis() - startTime < 200) return;

	// if (initialRun == 0) {
	// 	if (!atNodeIR()) {
	// 		initialRun = 2;
	// 		return;
	// 	}
	//
	// 	drive(NORMAL_SPEED, NORMAL_SPEED);
	// }
	// // printRawValues();
	// // Serial.println("");
	// if (initialRun != 2) return;


	if (path.length() == 0 || (path.length() > 0 && path[0] == 'B') || (path.length() > 0 && path[0] == 'A'))
	{
		if (CardDectecting(mfrc522) && !lockRFID)
		{
			lockRFID = true;
		}
	}

	// printRawValues();
	// Serial.println("");

	// if (!lockRFID)
	// {
	// 	if (CardDectecting(mfrc522))
	// 	{
	// 		lockRFID = true;
	// 	}
	// }
	// CardDectecting(mfrc522);

	if (!isRunning)
	{
		drive(0, 0);
		return;
	}

	// if (!BLUETOOTH_MODE)
	// {
	readIRValues();

	// Serial.print(", ");
	// Serial.print(atNode);
	// Serial.print(", ");
	// Serial.print(getWeightedAvg());
	// Serial.print(", ");

	if (translationalBrake)
	{
		if (millis() - timestamp < 100 )
		{
			drive(-255, -255);
		}
		else
		{
			drive(-10, -10);
			// isRunning = false;
			translationalBrake = false;
			runPath();
		}
	}
	else if (rotationalBrake)
	{
		if (millis() - timestamp < 50)
		{
			switch (turningDir)
			{
			case 1:
				drive(-255, 255);
				break;
			case -1:
				drive(255, -255);
				break;
			}
		}
		else
		{
			rotationalBrake = false;
			atNode = false;
			Serial3.println("NX");
			path.remove(0, 1);

			switch (turningDir) {
				case 1:
					drive(-10, 10);
					break;
				case -1:
					drive(10, -10);
					break;
			}

			// isRunning = false;
			lockRFID = false;
		}
	}
	else if (atNode)
	{
		int waitTime = 100;
		if (path[0] == 'A' || path[0] == 'B') waitTime = 800;
		else if (path[0] == 'L' || path[0] == 'R') waitTime = 500;

		if (getLeftIRValue() < 100 && getRightIRValue() < 100 && max(max(getRightCenterIRValue(), getCenterIRValue()), getLeftCenterIRValue()) > 200  && millis() - timestamp > waitTime)
		{
			if (path[0] == 'F')
			{
				driveKinematic(NORMAL_SPEED, -turn);

				Serial3.println("NX");
				path.remove(0, 1);
				atNode = false;
				lockRFID = false;
			}
			else if (!rotationalBrake)
			{
				rotationalBrake = true;
				timestamp = millis();
				drive(0, 0);
			}
		}
	}
	else
	{
		if (atNodeIR())
		{
			atNode = true;
			if (path[0] == 'F')
			{
				runPath();
			}
			else if (!translationalBrake)
			{
				translationalBrake = true;
				timestamp = millis();
				drive(10, 10);
			}
		}
		else
		{
			// if (path.length() > 1 && path[1] == 'F') {
			// 	driveKinematic(255, -turn);
			// } else {
			// 	driveKinematic(NORMAL_SPEED, -turn);
			// }
			driveKinematic(NORMAL_SPEED, -turn);
		}
	}

	// if (cmd == "BT")
	// {
	// 	BLUETOOTH_MODE = true;
	// 	drive(0, 0);
	// 	Serial.println("****Switched to BLUETOOTH mode.****");
	// }
	// }
	// else
	// {
	// if (cmd == "F")
	// 	drive(NORMAL_SPEED, NORMAL_SPEED); // �e�i
	// else if (cmd == "B")
	// 	drive(-NORMAL_SPEED, -NORMAL_SPEED); // ��h
	// else if (cmd == "L")
	// 	drive(-NORMAL_SPEED, NORMAL_SPEED); // ����
	// else if (cmd == "R")
	// 	drive(NORMAL_SPEED, -NORMAL_SPEED); // �k��
	// else if (cmd == "S")
	// 	drive(0, 0);
	// else if (cmd == "AUTO")
	// {
	// 	BLUETOOTH_MODE = false;
	// 	// Serial.println("****Switched to AUTO mode.****");
	// }
	// }

	// Serial.println(turn);
	// delay(TIME_STEP);
	turn = IR_PID.calculate(0.0, getWeightedAvg(), millis() - lastTime);
	// Serial.println(millis() - lastTime);
	lastTime = millis();
}

void runPath()
{
	char command = path.charAt(0);
	// Serial.println(command);
	if (command == 'F')
	{
		driveKinematic(255, 0);
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
		drive(-70, 70);
		turningDir = -1;
	}
	else if (command == 'A')
	{
		drive(70, -70);
		turningDir = 1;
	}
	else
	{
		// isRunning = false;
		drive(0, 0);
	}
}
