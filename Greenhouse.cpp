#include "Greenhouse.h"

#define UNSET_PIN 0xff

Zone::Zone() : pumpPin(UNSET_PIN), sensorPin(UNSET_PIN),
               lowValue(-1), highValue(-1) {}


void Zone::setPumpPin(const uint8_t &pin) {
    this->pumpPin = pin;
    digitalWrite(pin, LOW);
    pinMode(pin, OUTPUT);
}

void Zone::setSensorPin(const uint8_t &pin) {
    this->sensorPin = pin;
    digitalWrite(pin, LOW);
    pinMode(pin, INPUT);
}

void Zone::setLowValue(const int &val) {
    this->lowValue = val;
}

void Zone::setHighValue(const int &val) {
    this->highValue = val;
}

void Zone::setPumpTime(const int &time) {
    this->pumpTime = time;
}

uint8_t Zone::getPumpPin() const {
    return this->pumpPin;
}

uint8_t Zone::getSensorPin() const { 
    return this->sensorPin;
}

int Zone::getLowValue() const {
    return this->lowValue;
}

int Zone::getHighValue() const {
    return this->highValue;
}

int Zone::getRawValue() const {
    if (this->sensorPin == UNSET_PIN) {
        return -1; // error
    }

    return analogRead(this->sensorPin);
}

int Zone::getPumpTime() const {
    return this->pumpTime;
}

int clamp(const int &v, const int &a, const int &b) {
    if (a > b) {
        return constrain(v, b, a);
    }

    if (v < a) {
        return a;
    } else if (v > b) {
        return b;
    }

    return v;
}

int Zone::getMappedValue(const int &low, const int &high) const {
    const int raw = this->getRawValue();
    if (raw == -1) {
        return -1; // error
    }

    const int mapped = map(raw, this->lowValue, this->highValue, low, high);

    return clamp(mapped, low, high);
}

void Zone::loop() {
    if (this->sensorPin == UNSET_PIN || this->pumpPin == UNSET_PIN) {
        return;
    }
    auto sens = this->getMappedValue();
    if (sens > 75) {
        this->pumping = false;
        this->triggered = false;
        this->since = 0;
    } else if (sens < 25) {
        if (!triggered) {
            pumping = true;
            since = millis() / 1000;
        }
        this->triggered = true;
    }

    if (this->triggered) {
        unsigned long uptime = millis() / 1000;
        unsigned long dt = uptime - this->since;
        char buff[30];
        sprintf(buff, "P:%d %d S:%d T:%d", this->pumpPin, pumping, sens, dt);
        Serial.println(buff);
        if (dt >= this->pumpTime) {
            this->pumping = !this->pumping;
            this->since = millis() / 1000;
        }

        if (this->pumping) {
            digitalWrite(this->pumpPin, HIGH);
        } else {
            digitalWrite(this->pumpPin, LOW);
        }
    }
}
