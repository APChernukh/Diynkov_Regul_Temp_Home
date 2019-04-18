/*
 Name:    Diyankov_Regul_Temp_Home.ino
 Created: 18.04.2019 20:01:19
 Author:  solop
*/

#include <iarduino_OLED_txt.h>
#include <Wire.h>
#include <OneWire.h>

const int pOtpin = A0;          // пин подключения потециометра
const int pRelepin = 4;         // управляющий вход 1 драйвера моторов подключен к 3-му контакту Arduino
const int lRelepin = 2;         // управляющий вход 2 драйвера моторов подключен к 2-му контакту Arduino
const int pbUttonpin = 6;       // пин кнопки +
const int mbUttonpin = 7;       // пин кнопки -
OneWire ds(8);                  // датчик температуры вывод 8 в доме

const int potMax = 2;    // максимальное значение потенциометра
const int potMin = 7;     // минимальное значение потенциометра
int sensorValue = 0;
int outputValue = 0;
int sensorUstan = 0;
int tUstan = 0;
int tHo;
int tDelta[]{ 11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30 };
const int tUstanMax = tDelta[19];
const int tUstanMin = tDelta[0];

/*--- Адрес дисплея --- если нужно тораскоментировать любую из двух строк ниже ---*/
iarduino_OLED_txt myOLED(0x78);                           // Объявляем объект myOLED, указывая адрес дисплея на шине I2C: 0x78 (если учитывать бит RW=0).
//iarduino_OLED_txt myOLED(0x3C);                         // Объявляем объект myOLED, указывая адрес дисплея на шине I2C: 0x3C (если учитывать бит RW=0).
//iarduino_OLED_txt myOLED(0x3D);                         // Объявляем объект myOLED, указывая адрес дисплея на шине I2C: 0x3D (если учитывать бит RW=0).
extern uint8_t SmallFontRus[];
extern uint8_t BigNumbers[];
//  myOLED.setCoding(TXT_UTF8);                         // Указываем кодировку текста в скетче. Если на дисплее не отображается Русский алфавит, то ...
                                                        // раскомментируйте функцию setCoding и замените параметр TXT_UTF8, на TXT_CP866 или TXT_WIN1251.

// the setup function runs once when you press reset or power the board
void setup() {

	Serial.begin(9600);
	sensorValue = analogRead(pOtpin) / 51;               // получаем текущее значение
	Serial.println(sensorValue);                        // выводим результат на монитор
	delay(1);                                            // небольшая задержка для стабильности вывода результатов

	pinMode(pRelepin, OUTPUT);
	pinMode(lRelepin, OUTPUT);
	digitalWrite(pRelepin, HIGH);
	digitalWrite(lRelepin, HIGH);

	pinMode(pbUttonpin, INPUT);
	pinMode(mbUttonpin, INPUT);

	tUstan = tDelta[11];

	myOLED.begin();
	myOLED.clrScr();                                    // Стираем все с экрана
	myOLED.setFont(SmallFontRus);                       // Инициализируем русский шрифт
	myOLED.print(F("Регулятор температуры"), OLED_C, 0);   // Регулятор температуры
	myOLED.print(F("Прошивка 18 04 19"), OLED_C, 27);      // Прошивка 03 01 19
	myOLED.print(F("Пусть будет тепло и"), OLED_C, 37);    // Пусть будет тепло и
	myOLED.print(F("КОМФОРТНО"), OLED_C, 50);              // КОМФОРТНО
	delay(10000);
}

// the loop function runs over and over again until power down or reset
void loop() {

	tHome();
	uGolustanovki();
	bUtton();
	sensorValue = analogRead(pOtpin) / 51;                              //считываем значение с потенциометра
	Serial.println(sensorValue);                        // выводим результат на монитор
	delay(1);                                            // небольшая задержка для стабильности вывода результатов
	oLed();
	delay(1);


	if (sensorValue > sensorUstan)                                      //если значение больше двигаемся вперед
	{
		fOrward();                              //движемся со скоростью пропорциональной переданному значению функции
	}
	else if (sensorValue < sensorUstan)                                 //если значение меньше двигаемся назад
	{
		rEverse();                              //движемся со скоростью пропорциональной переданному значению функции
	}
	else                                                //если значение в заданном диапазоне
	{
		sTop();                                        //останавливаем двигатель
	}
}

void fOrward() {
	digitalWrite(lRelepin, HIGH);                      //на управляющий вход 1 драйвера моторов подаем высокий уровень сигнала
	digitalWrite(pRelepin, LOW);                       //на управляющий вход 2 драйвера моторов подаем низкий уровень сигнала
	Serial.print("Forward");
	Serial.println("");
}
void rEverse() {
	digitalWrite(lRelepin, LOW);                      //на управляющий вход 1 драйвера моторов подаем низкий уровень сигнала
	digitalWrite(pRelepin, HIGH);                     //на управляющий вход 2 драйвера моторов подаем высокий уровень сигнала
	Serial.print("Reverse");
	Serial.println("");
}
void sTop() {
	digitalWrite(lRelepin, LOW);                      //на управляющий вход 1 драйвера моторов подаем низкий уровень сигнала
	digitalWrite(pRelepin, LOW);                      //на управляющий вход 2 драйвера моторов подаем низкий уровень сигнала
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

	delay(1000);    //задержка 1 секунды
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
	tHo = (long)raw / 16.0; //температура Дома
	Serial.print("  temperatura Home = ");  // если нужно увидеть на мониторе порта, то раскоментируйте 4 строчки
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
			while (digitalRead(pbUttonpin)) { delay(50); }              // ждём пока мы не отпустим кнопку
			++tUstan;
			Serial.print(F("Установленное значение температуры: \n"));
			Serial.print(tUstan);
		}
	}
	if (tUstan > tDelta[0] && tUstan <= tUstanMax) {
		if (digitalRead(mbUttonpin)) {
			while (digitalRead(mbUttonpin)) { delay(50); }              // ждём пока мы не отпустим кнопку
			--tUstan;
			Serial.print(F("Установленное значение температуры: \n"));
			Serial.print(tUstan);
		}
	}
	return tUstan;
}
int oLed() {
	myOLED.clrScr();                                    // Стираем все с экрана

	myOLED.setFont(SmallFontRus);
	myOLED.print(F("Температура"), 5, 5);
	myOLED.print(F(" в ДОМЕ"), 5, 15);
	myOLED.setFont(BigNumbers);
	myOLED.print(String(tHo), 90, 5);

	myOLED.setFont(SmallFontRus);
	myOLED.print(F("Настройка"), 5, 38);
	myOLED.print(F("температуры"), 5, 48);
	myOLED.setFont(BigNumbers);
	myOLED.print(String(tUstan), 90, 35);
}
