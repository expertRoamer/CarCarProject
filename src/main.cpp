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

String path = "LBRBLBRB";
bool atNode = false;
bool first = true;
double turn = 0.0;

void runPath();
int num = 0;
int sum[5] = {};

bool isRunning = true;
bool translationalBrake = false;
bool rotationalBrake = false;
int turningDir = 1; // i: clockwise; -1: counterclockwise
unsigned long timestamp = 0;
unsigned long lastTime = 0;

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
	// CardDectecting(mfrc522);
	// String cmd = ""; // �����Ӧ��Ť������O
	String cmd = BlueTooth(); // �����Ӧ��Ť������O

	if (!isRunning) return;

	printIRValues();
	Serial.println("");

	if (!BLUETOOTH_MODE) {
		readIRValues();

		// Serial.print(", ");
		// Serial.print(atNode);
		// Serial.print(", ");
		// Serial.print(getWeightedAvg());
		// Serial.print(", ");

		if (translationalBrake) {
			if (millis() - timestamp < 100) {
				drive(-255, -255);
			} else {
				drive(0, 0);
				// isRunning = false;
				translationalBrake = false;
				runPath();
			}
		} else if (rotationalBrake) {
			if (millis() - timestamp < 50) {
				drive(-255 * turningDir, 255 * turningDir);
			} else {
				rotationalBrake = false;
				atNode = false;
				path.remove(0, 1);
				// drive(0, 0);
				// isRunning = false;
			}
		} else if (atNode) {
			if (getLeftIRValue() < 100 && getRightIRValue() < 100 && max(max(getCenterIRValue(), getLeftCenterIRValue()), getRightCenterIRValue()) > 100 && millis() - timestamp > 500) {
				if (!rotationalBrake) {
					rotationalBrake = true;
					timestamp = millis();
					drive(0, 0);
				}
			}
		} else {
			if (atNodeIR()) {
				atNode = true;
				if (!translationalBrake) {
					translationalBrake = true;
					timestamp = millis();
					drive(0, 0);
				}
			} else {
				turn = IR_PID.calculate(0.0, getWeightedAvg());
				driveKinematic(NORMAL_SPEED, -turn);
			}
		}

		if (cmd == "BT") {
			BLUETOOTH_MODE = true;
			drive(0, 0);
			Serial.println("****Switched to BLUETOOTH mode.****");
		}
	} else {
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
	// Serial.println(millis() - lastTime);
	// lastTime = millis();
}

void runPath() {
	char command = path.charAt(0);
	// Serial.println(command);
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
	} else {
		isRunning = false;
		drive(0, 0);
	}
}
