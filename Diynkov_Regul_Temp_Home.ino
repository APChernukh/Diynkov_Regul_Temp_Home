/*
 Name:    Diyankov_Regul_Temp_Home.ino
 Created: 18.04.2019 20:01:19
 Author:  solop
*/

#include <iarduino_OLED_txt.h>
#include <Wire.h>
#include <OneWire.h>

const int pOtpin = A0;          // ��� ����������� ������������
const int pRelepin = 4;         // ����������� ���� 1 �������� ������� ��������� � 3-�� �������� Arduino
const int lRelepin = 2;         // ����������� ���� 2 �������� ������� ��������� � 2-�� �������� Arduino
const int pbUttonpin = 6;       // ��� ������ +
const int mbUttonpin = 7;       // ��� ������ -
OneWire ds(8);                  // ������ ����������� ����� 8 � ����

const int potMax = 2;    // ������������ �������� �������������
const int potMin = 7;     // ����������� �������� �������������
int sensorValue = 0;
int outputValue = 0;
int sensorUstan = 0;
int tUstan = 0;
int tHo;
int tDelta[]{ 11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30 };
const int tUstanMax = tDelta[19];
const int tUstanMin = tDelta[0];

/*--- ����� ������� --- ���� ����� ������������������ ����� �� ���� ����� ���� ---*/
iarduino_OLED_txt myOLED(0x78);                           // ��������� ������ myOLED, �������� ����� ������� �� ���� I2C: 0x78 (���� ��������� ��� RW=0).
//iarduino_OLED_txt myOLED(0x3C);                         // ��������� ������ myOLED, �������� ����� ������� �� ���� I2C: 0x3C (���� ��������� ��� RW=0).
//iarduino_OLED_txt myOLED(0x3D);                         // ��������� ������ myOLED, �������� ����� ������� �� ���� I2C: 0x3D (���� ��������� ��� RW=0).
extern uint8_t SmallFontRus[];
extern uint8_t BigNumbers[];
//  myOLED.setCoding(TXT_UTF8);                         // ��������� ��������� ������ � ������. ���� �� ������� �� ������������ ������� �������, �� ...
                                                        // ���������������� ������� setCoding � �������� �������� TXT_UTF8, �� TXT_CP866 ��� TXT_WIN1251.

// the setup function runs once when you press reset or power the board
void setup() {

	Serial.begin(9600);
	sensorValue = analogRead(pOtpin) / 51;               // �������� ������� ��������
	Serial.println(sensorValue);                        // ������� ��������� �� �������
	delay(1);                                            // ��������� �������� ��� ������������ ������ �����������

	pinMode(pRelepin, OUTPUT);
	pinMode(lRelepin, OUTPUT);
	digitalWrite(pRelepin, HIGH);
	digitalWrite(lRelepin, HIGH);

	pinMode(pbUttonpin, INPUT);
	pinMode(mbUttonpin, INPUT);

	tUstan = tDelta[11];

	myOLED.begin();
	myOLED.clrScr();                                    // ������� ��� � ������
	myOLED.setFont(SmallFontRus);                       // �������������� ������� �����
	myOLED.print(F("��������� �����������"), OLED_C, 0);   // ��������� �����������
	myOLED.print(F("�������� 18 04 19"), OLED_C, 27);      // �������� 03 01 19
	myOLED.print(F("����� ����� ����� �"), OLED_C, 37);    // ����� ����� ����� �
	myOLED.print(F("���������"), OLED_C, 50);              // ���������
	delay(10000);
}

// the loop function runs over and over again until power down or reset
void loop() {

	tHome();
	uGolustanovki();
	bUtton();
	sensorValue = analogRead(pOtpin) / 51;                              //��������� �������� � �������������
	Serial.println(sensorValue);                        // ������� ��������� �� �������
	delay(1);                                            // ��������� �������� ��� ������������ ������ �����������
	oLed();
	delay(1);


	if (sensorValue > sensorUstan)                                      //���� �������� ������ ��������� ������
	{
		fOrward();                              //�������� �� ��������� ���������������� ����������� �������� �������
	}
	else if (sensorValue < sensorUstan)                                 //���� �������� ������ ��������� �����
	{
		rEverse();                              //�������� �� ��������� ���������������� ����������� �������� �������
	}
	else                                                //���� �������� � �������� ���������
	{
		sTop();                                        //������������� ���������
	}
}

void fOrward() {
	digitalWrite(lRelepin, HIGH);                      //�� ����������� ���� 1 �������� ������� ������ ������� ������� �������
	digitalWrite(pRelepin, LOW);                       //�� ����������� ���� 2 �������� ������� ������ ������ ������� �������
	Serial.print("Forward");
	Serial.println("");
}
void rEverse() {
	digitalWrite(lRelepin, LOW);                      //�� ����������� ���� 1 �������� ������� ������ ������ ������� �������
	digitalWrite(pRelepin, HIGH);                     //�� ����������� ���� 2 �������� ������� ������ ������� ������� �������
	Serial.print("Reverse");
	Serial.println("");
}
void sTop() {
	digitalWrite(lRelepin, LOW);                      //�� ����������� ���� 1 �������� ������� ������ ������ ������� �������
	digitalWrite(pRelepin, LOW);                      //�� ����������� ���� 2 �������� ������� ������ ������ ������� �������
	Serial.print("STOP");
	Serial.println("");
}
int tHome() {
	byte i;
	byte present = 0;
	byte type_s;
	byte data[12];
	byte addr[8];


	if (!ds.search(addr)) {
		ds.reset_search();
		return;
	}


	if (OneWire::crc8(addr, 7) != addr[7]) {
		return;
	}
	switch (addr[0]) {
	case 0x10:
		type_s = 1;
		break;
	case 0x28:
		type_s = 0;
		break;
	case 0x22:
		type_s = 0;
		break;
	default:
		return;
	}

	ds.reset();
	ds.select(addr);
	ds.write(0x44, 1);

	delay(1000);    //�������� 1 �������
	present = ds.reset();
	ds.select(addr);
	ds.write(0xBE);

	for (i = 0; i < 9; i++) {
		data[i] = ds.read();
	}

	int16_t raw = (data[1] << 8) | data[0];
	if (type_s) {
		raw = raw << 3;
		if (data[7] == 0x10) {
			raw = (raw & 0xFFF0) + 12 - data[6];
		}
	}
	else {
		byte cfg = (data[4] & 0x60);

		if (cfg == 0x00) raw = raw & ~7;
		else if (cfg == 0x20) raw = raw & ~3;
		else if (cfg == 0x40) raw = raw & ~1;
	}
	tHo = (long)raw / 16.0; //����������� ����
	Serial.print("  temperatura Home = ");  // ���� ����� ������� �� �������� �����, �� ��������������� 4 �������
	Serial.print(tHo);
	Serial.print(" Celsius ");
	Serial.println("");
	return tHo;
}
int uGolustanovki() {
	if ((tUstan - tHo) >= 3) {
		sensorUstan = potMax;
	}
	else if ((tUstan - tHo) < 3 && (tUstan - tHo) > 1) {
		sensorUstan = 3;
	}
	else if ((tUstan - tHo) < 2 && (tUstan - tHo) > 0) {
		sensorUstan = 4;
	}
	else if ((tUstan - tHo) < 1 && (tUstan - tHo) > -1) {
		sensorUstan = 5;
	}
	else if ((tUstan - tHo) < 0 && (tUstan - tHo) > -2) {
		sensorUstan = 6;
	}
	else if ((tUstan - tHo) <= -2) {
		sensorUstan = potMin;
	}
	Serial.print(F("Sensor Ustan: "));
	Serial.print(sensorUstan);
	Serial.println("");

	return sensorUstan;
}
int bUtton() {
	if (tUstan < tDelta[19] && tUstan >= tUstanMin) {
		if (digitalRead(pbUttonpin)) {
			while (digitalRead(pbUttonpin)) { delay(50); }              // ��� ���� �� �� �������� ������
			++tUstan;
			Serial.print(F("������������� �������� �����������: \n"));
			Serial.print(tUstan);
		}
	}
	if (tUstan > tDelta[0] && tUstan <= tUstanMax) {
		if (digitalRead(mbUttonpin)) {
			while (digitalRead(mbUttonpin)) { delay(50); }              // ��� ���� �� �� �������� ������
			--tUstan;
			Serial.print(F("������������� �������� �����������: \n"));
			Serial.print(tUstan);
		}
	}
	return tUstan;
}
int oLed() {
	myOLED.clrScr();                                    // ������� ��� � ������

	myOLED.setFont(SmallFontRus);
	myOLED.print(F("�����������"), 5, 5);
	myOLED.print(F(" � ����"), 5, 15);
	myOLED.setFont(BigNumbers);
	myOLED.print(String(tHo), 90, 5);

	myOLED.setFont(SmallFontRus);
	myOLED.print(F("���������"), 5, 38);
	myOLED.print(F("�����������"), 5, 48);
	myOLED.setFont(BigNumbers);
	myOLED.print(String(tUstan), 90, 35);
}
