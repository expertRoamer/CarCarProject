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

PIDController IR_PID(80, 0.0, 0.0);

String path = "RBRBRBRBRBRBRBRBRBRBRBRBRBRBRBRBRBRBRBRBRBRBRBRB";
bool atNode = false;
bool first = true;
double turnLast = 0.0;

void runPath();
int num = 0;
int sum[5] = {};

bool BLUETOOTH_MODE = false; // 用來切換模式

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

	SPI.begin();
	mfrc522 = new MFRC522(SS_PIN, RST_PIN);
	mfrc522->PCD_Init();

	BlueToothInit();
}
// 車車傳給電腦用Serial3
// 電腦傳給車車用Serial
void loop()
{
	CardDectecting(mfrc522);
	String cmd = BlueTooth(); // 接收來自藍牙的指令

	if (!BLUETOOTH_MODE) // 自動模式
	{
		readIRValues();

		if (atNode)
		{
			if ((getCenterIRValue() + getLeftCenterIRValue() + getRightCenterIRValue()) > 200 && getLeftIRValue() < 100 && getRightIRValue() < 100)
			{
				atNode = false;
				path.remove(0, 1);
			}
			else if (path[0] == 'B' && startPID(150, 150, getLeftIRValue(), getLeftCenterIRValue(), getRightIRValue(), getRightCenterIRValue()))
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
			Serial3.println("****Switched to BLUETOOTH mode.****");
		}
	}
	else // 藍牙手動模式
	{

		if (cmd == "F")
		{
			Serial3.println("---Driving Forward---");
			drive(NORMAL_SPEED, NORMAL_SPEED); // 前進
		}
		else if (cmd == "B")
		{
			Serial3.println("---Driving Backward---");
			drive(-NORMAL_SPEED, -NORMAL_SPEED); // 後退
		}
		else if (cmd == "L")
		{
			Serial3.println("---Driving Leftward---");
			drive(-NORMAL_SPEED, NORMAL_SPEED); // 左轉
		}
		else if (cmd == "R")
		{
			Serial3.println("---Driving Rightward---");
			drive(NORMAL_SPEED, -NORMAL_SPEED); // 右轉
		}
		else if (cmd == "S")
		{
			Serial3.println("---Stopping---");
			drive(0, 0);
		}
		else if (cmd == "AUTO")
		{
			BLUETOOTH_MODE = false;
			Serial.println("****Switched to AUTO mode.****");
			Serial3.println("****Switched to AUTO mode.****");
		}
	}
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
		drive(0, 255);
	}
	else if (command == 'R')
	{
		drive(255, 0);
	}
	else if (command == 'B')
	{
		back(150, 150, getCenterIRValue(), getLeftCenterIRValue(), getRightCenterIRValue(), getRightIRValue());
	}
}
