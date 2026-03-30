#include <Arduino.h>
// RFID¼Ò²Õ
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

String path = "RBRBRBRB";
bool atNode = false;
bool first = true;
double turnLast = 0.0;

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

	SPI.begin();
	mfrc522 = new MFRC522(SS_PIN, RST_PIN);
	mfrc522->PCD_Init();

	BlueToothInit();
}

void loop()
{
	CardDectecting(mfrc522);
	BlueTooth();
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
