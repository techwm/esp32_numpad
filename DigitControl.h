#ifndef DigitControl_H
#define DigitControl_H
#include <Arduino.h>

// char characters[] = {
//     '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
//     'A', 'C', 'D', 'E', 'F', 'H', 'L', 'N', 'R', 'U'};

class DigitControl
{
public:
  DigitControl(
      int pin1, int pin2, int pin3, int pin4,
      int pin5, int pin6, int pin7, int pin8);
  void begin(void);

private:
  int *_pins;
};

#endif