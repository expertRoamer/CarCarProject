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

PIDController IR_PID(50, 0, 1000); // Previous :(60, 0, 100) turn : (50, 0, 100)
// PIDController TURN_PID(50, 0, 200); 60 200

String path = "FFLFBFRRLRALLFRAF"; // A: clockwise 180; B: counterclockwise 180
bool atNode = false;
bool first = true;

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

bool lockRFID = false;

bool BLUETOOTH_MODE = false; // �ΨӤ����Ҧ�

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

	Serial.begin(9600);
	Serial3.begin(9600);

	SPI.begin();
	mfrc522 = new MFRC522(SS_PIN, RST_PIN);
	mfrc522->PCD_Init();
}

void loop()
{
	if (path.length() == 0 || path[0] == 'B')
	{
		if (CardDectecting(mfrc522) && !lockRFID)
		{
			lockRFID = true;
		}
	}

	// CardDectecting(mfrc522);
	// String cmd = ""; // �����Ӧ��Ť������O
	String cmd = BlueTooth(); // �����Ӧ��Ť������O

	if (!isRunning)
	{
		drive(0, 0);
		return;
	}

	// printRawV]alues();
	// Serial.println("");

	if (!BLUETOOTH_MODE)
	{
		readIRValues();

		// Serial.print(", ");
		// Serial.print(atNode);
		// Serial.print(", ");
		// Serial.print(getWeightedAvg());
		// Serial.print(", ");

		if (translationalBrake)
		{
			if (millis() - timestamp < 100)
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
				Serial3.println("STEP_DONE*");
				path.remove(0, 1);
				drive(10, -10);
				// isRunning = false;
			}
		}
		else if (atNode)
		{
			if (getLeftIRValue() < 100 && getRightIRValue() < 100 && max(max(getRightCenterIRValue(), getCenterIRValue()), getLeftCenterIRValue()) > 100 && (millis() - timestamp) > ((path[0] == 'B') ? 800 : 400))
			{
				if (path[0] == 'F')
				{
					Serial3.println("STEP_DONE*");
					path.remove(0, 1);
					lockRFID = false;
					atNode = false;
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
					drive(0, 0);
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

		if (cmd == "BT")
		{
			BLUETOOTH_MODE = true;
			drive(0, 0);
			Serial.println("****Switched to BLUETOOTH mode.****");
		}
	}
	else
	{
		if (cmd == "F")
			drive(NORMAL_SPEED, NORMAL_SPEED); // �e�i
		else if (cmd == "B")
			drive(-NORMAL_SPEED, -NORMAL_SPEED); // ��h
		else if (cmd == "L")
			drive(-NORMAL_SPEED, NORMAL_SPEED); // ����
		else if (cmd == "R")
			drive(NORMAL_SPEED, -NORMAL_SPEED); // �k��
		else if (cmd == "S")
			drive(0, 0);
		else if (cmd == "AUTO")
		{
			BLUETOOTH_MODE = false;
			Serial.println("****Switched to AUTO mode.****");
		}
	}

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
		drive(15, 150);
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
		isRunning = false;
		drive(0, 0);
	}
}
