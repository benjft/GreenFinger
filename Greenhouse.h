#ifndef GREENHOUSE_H
#define GREENHOUSE_H

#include <Arduino.h>

class Zone {
private:
    uint8_t pumpPin, sensorPin;

    int lowValue, highValue, pumpTime;
    unsigned long since;
    bool triggered;
    bool pumping;
    
public:
    Zone();
    
    void setPumpPin(const uint8_t &pin);
    void setSensorPin(const uint8_t &pin);
    void setLowValue(const int &val);
    void setHighValue(const int &val);
    void setPumpTime(const int &time);

    uint8_t getPumpPin() const;
    uint8_t getSensorPin() const;
    int getLowValue() const;
    int getHighValue() const;
    int getPumpTime() const;

    int getRawValue() const;
    int getMappedValue(const int &low = 0, const int &high = 100) const;

    void loop();
};

#endif