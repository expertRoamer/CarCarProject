#include <Arduino.h>
#include <SPI.h> 
#include <MFRC522.h>

#include "PinManager.h"
#include "MotorSystem.h"

// IR value
double left;
double left_center;
double center;
double right;
double right_center;
MFRC522 *mfrc522;

double before_error;
double Kd = -0.005;

int normal_speed = 150;

double speed();
int maxIR();
void MoveFor(double, double, double);

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
  
	Serial.begin(9600);
	// 藍芽
	// SPI.begin(); 
	
	// mfrc522 = new MFRC522(SS_PIN, RST_PIN); 
	// // 請系統去要一塊記憶體空間，後面呼叫它的建構函式
	// // 將(SS, RST) 當成參數傳進去初始化。
	// mfrc522->PCD_Init(); 
	// /* 初始化MFRC522讀卡機PCD_Init 模組。
	// -> 表示：透過記憶體位置，找到mfrc522 這物件，再翻其內容。*/
	// Serial.println(F("Read UID on a MIFARE PICC:")); 
}

void loop() { 
	// MoveForward(200,200);
	Serial.print(analogRead(IR_LEFT));
	Serial.print(" ");
	Serial.print(analogRead(IR_LEFT_CENTER));
	Serial.print(" ");
	Serial.print(analogRead(IR_CENTER));
	Serial.print(" ");
	Serial.print(analogRead(IR_RIGHT_CENTER));
	Serial.print(" ");
	Serial.println(analogRead(IR_RIGHT));

	Serial.println(normal_speed * (1 - speed()));
	Serial.print(" ");
	Serial.println(normal_speed * (1 + speed()));
	Serial.print(" ");
	Serial.println(maxIR());

	left = analogRead(IR_LEFT);
	left_center = analogRead(IR_LEFT_CENTER);
	center = analogRead(IR_CENTER);
	right_center = analogRead(IR_RIGHT_CENTER);
	right = analogRead(IR_RIGHT);

	// MoveFor(normal_speed * (1 - speed()), normal_speed * (1 + speed()) , 5);

	before_error = speed();

	drive(100.0, -100.0, 5000);
	delay(2000);
}

double speed() {
	double difference;
	double error = 0;
	double P = 0.004;
	if (maxIR() == 0) {
		error = 4 * 0.2;
	}
	if (maxIR() == 1) {
		error = 0.6;
	}
	if (maxIR() == 3) {
		error = -1 * 0.6;
	}
	if (maxIR() == 4) {
		error = -4 * 0.2;
	}
	difference = (error - before_error) / 0.01;
	//return P * error;
	return error + difference * Kd;
	//positive:left decrease etc
}

int maxIR() {
	if (left >= center && left >= right && left >= left_center && left >= right_center) {
		return 0;
	}
	if (left_center >= left && left_center >= right && left_center >= center && left_center >= right_center) {
		return 1;
	}
	if (center >= left && center >= right && center >= left_center && center >= right_center) {
		return 2;
	}
	if (right_center >= left && right_center >= right && right_center >= left_center && right_center >= center) {
		return 3;
	}
	if (right >= left && right >= center && right >= left_center && right >= right_center) {
		return 4;
	}

	return 2;
}

double difference() {
	
}