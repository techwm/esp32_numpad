#include "DigitControl.h"

DigitControl::DigitControl(
    int pin1, int pin2, int pin3, int pin4,
    int pin5, int pin6, int pin7, int pin8)
{
    _pins = new int[pin1, pin2, pin3, pin4,
                      pin5, pin6, pin7, pin8];
}

void DigitControl::begin(void)
{
    // for (int i = 0; i < 8; i++)
    // {
    //     // Serial.println(this->_pins[i]);
    //     pinMode(this->_pins[i], OUTPUT);
    // }

    // for (int i = 0; i < 8; i++)
    // {
    //     digitalWrite(this->_pins[i], HIGH);
    //     // Serial.println(this->_pins[i]);
    // }
}