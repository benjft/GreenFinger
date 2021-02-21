#include <Arduino.h>
#include <EEPROM.h>

#include "Greenhouse.h"

#define EEPROM_DEFAULT 0xff

#define RESET_PIN 12
#define PIN_TEST A3

#define N_ANALOG 8
const uint8_t ANALOG_PINS[N_ANALOG] {A0, A1, A2, A3, A4, A5, A6, A7};

#define N_ZONES 5
Zone zones[N_ZONES];

int pumpTime = 30; // pump time in seconds. Will pump at most this at a time.

void loadFromEEPROM() {
	uint8_t hasSave = EEPROM.read(0);
	if (hasSave == EEPROM_DEFAULT) {
		return;
	}

	int address = 1;
	for (size_t i = 0; i < N_ZONES; i++) {
		EEPROM.get(address, zones[i]);
		Zone zone = zones[i];
		
		uint8_t sensorPin = zone.getSensorPin();
		digitalWrite(sensorPin, LOW);
		pinMode(sensorPin, INPUT);

		uint8_t pumpPin = zone.getPumpPin();
		digitalWrite(pumpPin, LOW);
		pinMode(pumpPin, OUTPUT);

		address += sizeof(Zone);
	}
}

void saveToEEPROM() {
	EEPROM.write(0, 0); // overwrite initial byte to say we have a save.
	
	int address = 1;
	for (size_t i = 0; i < N_ZONES; i++) {
		EEPROM.put(address, zones[i]);
		address += sizeof(Zone);
	}
}

void setup() {
	// reset needs to be high to stay on. Pull low to reset.
	digitalWrite(RESET_PIN, HIGH);
	Serial.begin(115200);

	loadFromEEPROM();
	
	pinMode(RESET_PIN, OUTPUT);

	while (!Serial) {} // pause until serial ready
}

void reset() {
	// reset needs to be high to stay on. Pull low to reset.
	digitalWrite(RESET_PIN, LOW);
	delay(1000);
}

bool state = false;
void loop() {
	for (size_t z = 0; z < N_ZONES; z++) {
		zones[z].loop();
	}

//   int v = analogRead(ANALOG_PINS[1]);
//   Serial.println(v);
  delay(500);
}

void serialSet(String s) {
	int propLen = s.indexOf(' ');
	String prop = s.substring(0, propLen);
	s = s.substring(propLen+1); // +1 to skip space

	int zoneNum = s.toInt();
	s = s.substring(String(zoneNum).length() + 1); // +1 to skip space

	Zone *zone = &zones[zoneNum];
	if (prop == "pump") {
		int pin = s.toInt();
		zone->setPumpPin(pin);
	} else if (prop == "sensor") {
		int pin = s.toInt();
		zone->setSensorPin(ANALOG_PINS[pin]);
	} else if (prop == "time") {
		int time = s.toInt();
		zone->setPumpTime(time);
	} else {
		int value;
		if (s.length() == 0) {
			value = zone->getRawValue();
     		Serial.print("set to raw: ");
			Serial.println(value);
		} else {
			value = s.toInt();
		}
		if (prop == "low") {
			zone->setLowValue(value);
		} else if (prop == "high") {
			zone->setHighValue(value);
		}
	}
}

void serialGet(String s) {
	int propLen = s.indexOf(' ');
	String prop = s.substring(0, propLen);
	s = s.substring(propLen+1); // +1 to skip space

	int zoneNum = s.toInt();
	s = s.substring(String(zoneNum).length() + 1); // +1 to skip space

	Zone *zone = &zones[zoneNum];
	if (prop == "pump") {
		Serial.println(zone->getPumpPin());
	} else if (prop == "sensor") {
		// convert absolute pin to analoge pin number
		uint8_t pin = zone->getSensorPin();
		for (size_t i = 0; i < N_ANALOG; i++) {
			if (ANALOG_PINS[i] == pin) {
				pin = i;
				break;
			}
		}
		Serial.println(pin, DEC);
	} else if (prop == "low") {
		Serial.println(zone->getLowValue());
	} else if (prop == "high") {
		Serial.println(zone->getHighValue());
	} else if (prop == "time") {
		Serial.println(zone->getPumpTime());
	} else if (prop == "raw") {
    	Serial.println(zone->getRawValue());
	} else if (prop == "mapped") {
    	Serial.println(zone->getMappedValue());
	}
}

void serialEvent() {
	while (Serial.available()) {
		String next = Serial.readStringUntil('\n');
		next.toLowerCase();
		if (next == "save") {
			saveToEEPROM();
		} else if (next.startsWith("set ")) {
			String substr = next.substring(4);
			serialSet(substr);
		} else if (next.startsWith("get ")) {
			String substr = next.substring(4);
			serialGet(substr);
		} else if (next == "reset") {
			reset();
		}
	}
}
