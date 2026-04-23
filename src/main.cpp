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

PIDController IR_PID(45, 0, 40); // 80

String path = "BBBBBBBB";
bool atNode = false;
bool first = true;
double turnLast = 0.0;

void runPath();
int num = 0;
int sum[5] = {};

bool BLUETOOTH_MODE = false; // �ΨӤ����Ҧ�

void setup() {
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

void loop() {
	CardDectecting(mfrc522);
	// String cmd = ""; // �����Ӧ��Ť������O
	String cmd = BlueTooth(); // �����Ӧ��Ť������O
	printIRValues();
	if (!BLUETOOTH_MODE) // �۰ʼҦ�
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
				double turn = IR_PID.calculate(0.0, getWeightedAvg());
				driveKinematic(NORMAL_SPEED, -turn);
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
	else // �Ť���ʼҦ�
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

	Serial.println(turnLast);
	delay(TIME_STEP);
}

void runPath()
{
	char command = path.charAt(0);
	// Serial.println(command);
	if (command == 'F')
	{
		driveKinematic(NORMAL_SPEED, 0);
	}
	else if (command == 'L')
	{
		drive(-40, 150);
	}
	else if (command == 'R')
	{
		drive(200, -25);
	}
	else if (command == 'B')
	{
		back(150, 150, getCenterIRValue(), getLeftCenterIRValue(), getRightCenterIRValue(), getRightIRValue());
	}
}
