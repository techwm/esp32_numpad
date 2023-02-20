
#include <BleKeyboard.h>
#include "BuzzerControl.h"
// #include "DigitControl.h"

int longPressDelay = 350; // customizable keyboard values
int spamSpeed = 15;
int cClock = 0;
int cPulse = 0;
int lastActive = 0;
int lastPulseCheck = 0;
int pulseCount = 0;
int profile = 1;
bool bleConnected = false;
bool numlock = true;
int buzzerPin = 21;
int enterPin = 34;
int segmentPins[8] = {19, 18, 5, 17, 16, 4, 2, 15};
char profiles[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};

// declaring inputs and outputs
int inputs[] = {23, 26, 27, 14, 12, 13};
#define inCount sizeof(inputs) / sizeof(inputs[0])
int outputs[] = {32, 33, 25, 22, 34}; // pull down by 100K ohm resistors
#define outCount sizeof(outputs) / sizeof(outputs[0])

#define TIME_TO_SLEEP 120 /* Time ESP32 will go to sleep (in seconds) */
#define totalProfile sizeof(profiles) / sizeof(profiles[0])
#define seconds() (millis() / 1000)

#define KEY_NA 0x00
#define KEY_DELETE 0xD4
#define KEY_BACKSPACE 0xB2
#define KEY_DELETE 0xD4

// Numeric keypad
#define KEY_NUM_LOCK 0xDB
#define KEY_KP_SLASH 0xDC
#define KEY_KP_ASTERISK 0xDD
#define KEY_KP_MINUS 0xDE
#define KEY_KP_PLUS 0xDF
#define KEY_KP_ENTER 0xE0
#define KEY_KP_1 0xE1
#define KEY_KP_2 0xE2
#define KEY_KP_3 0xE3
#define KEY_KP_4 0xE4
#define KEY_KP_5 0xE5
#define KEY_KP_6 0xE6
#define KEY_KP_7 0xE7
#define KEY_KP_8 0xE8
#define KEY_KP_9 0xE9
#define KEY_KP_0 0xEA
#define KEY_KP_DOT 0xEB

int alphaBits[20][7] = {
    {0, 1, 1, 1, 1, 1, 1}, // 0
    {0, 0, 0, 1, 0, 0, 1}, // 1
    {1, 0, 1, 1, 1, 1, 0}, // 2
    {1, 0, 1, 1, 0, 1, 1},
    {1, 1, 0, 1, 0, 0, 1},
    {1, 1, 1, 0, 0, 1, 1},
    {1, 1, 1, 0, 1, 1, 1},
    {0, 0, 1, 1, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 0, 0, 1}, // 9
    {1, 1, 1, 1, 1, 0, 1},
    {0, 1, 1, 0, 1, 1, 0},
    {1, 0, 0, 1, 1, 1, 1},
    {1, 1, 1, 0, 1, 1, 0},
    {1, 1, 1, 0, 1, 0, 0},
    {1, 1, 0, 1, 1, 0, 1}, // H
    {0, 1, 0, 0, 1, 1, 0},
    {1, 0, 0, 0, 1, 0, 1},
    {1, 0, 0, 0, 1, 0, 0},
    {0, 1, 0, 1, 1, 1, 1},
};
char characters[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'A', 'C', 'D', 'E', 'F', 'H', 'L', 'N', 'R', 'U'};

#define totalChar sizeof(characters) / sizeof(characters[0])

char sysLayout[outCount][inCount] = {
    {'A', 'L', '7', '4', '1', 'p'},
    {'B', '/', '8', '5', '2', '0'},
    {'C', '*', '9', '6', '3', '.'},
    {'D', '-', '+', 'y', 'u', 'i'},
    {'E', 'f', 'g', 'h', 'j', 'k'}};

int layout1[outCount][inCount] = {
    {KEY_NA, KEY_NUM_LOCK, KEY_KP_7, KEY_KP_4, KEY_KP_1, KEY_NA},
    {KEY_NA, KEY_KP_SLASH, KEY_KP_8, KEY_KP_5, KEY_KP_2, KEY_KP_0},
    {KEY_BACKSPACE, KEY_KP_ASTERISK, KEY_KP_9, KEY_KP_6, KEY_KP_3, KEY_KP_DOT},
    {KEY_DELETE, KEY_KP_MINUS, KEY_KP_PLUS, KEY_NA, KEY_NA, KEY_NA},
    {KEY_KP_ENTER, KEY_NA, KEY_NA, KEY_NA, KEY_NA, KEY_NA}};

int keyDown[outCount][inCount];
bool keyLong[outCount][inCount];

BleKeyboard bleKeyboard;
BuzzerControl buzzerCtrl(buzzerPin);
// DigitControl digitCtrl(19, 18, 5, 17, 16, 4, 2, 15);

void setup()
{
  Serial.begin(115200);

  for (int i = 0; i < 8; i++)
  {
    pinMode(segmentPins[i], OUTPUT);
  }

  for (int i = 0; i < outCount; i++)
  { // declaring all the outputs and setting them high
    if (i == outCount - 1)
    {
      break;
    }
    pinMode(outputs[i], OUTPUT);
    digitalWrite(outputs[i], HIGH);
  }
  for (int i = 0; i < inCount; i++)
  { // declaring all the inputs and activating the internal pullup resistor
    pinMode(inputs[i], INPUT_PULLUP);
  }

  // put your setup code here, to run once:
  pinMode(enterPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  bleKeyboard.begin();

  // ESP_EXT1_WAKEUP_ANY_HIGH
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_34, 1);
  buzzerCtrl.play_start_sound();

  lastActive = seconds();
  lastPulseCheck = millis();
  pulseCount = 0;
}

void loop()
{
  int idle_time = seconds() - lastActive;
  if (idle_time >= TIME_TO_SLEEP)
  {
    go_to_sleep();
  }

  if (bleKeyboard.isConnected())
  {
    bleConnected = true;
    set_dot(numlock);
    show_character(profiles[profile - 1]);
    pulseCount = 0;
  }
  else
  {
    if (bleConnected == true)
    {
      show_character('*');
      bleConnected = false;
      pulseCount = 0;
    }

    if (millis() - lastPulseCheck >= 1500)
    {
      toggle_dot();
      lastPulseCheck = millis();
      buzzerCtrl.play_pulse_sound();
      Serial.print("connectd: ");
      Serial.println(bleConnected);
      pulseCount = pulseCount + 1;
    }

    if (pulseCount > 10)
    {
      go_to_sleep();
    }
  }

  for (int i = 0; i < outCount; i++)
  {
    if (i == outCount - 1)
    {
      if (digitalRead(enterPin) == HIGH)
      {
        keyPressed(i, 0);
      }
      else if (keyDown[4][0] != 0)
      {
        resetKey(i, 0);
      }
      break;
    }
    digitalWrite(outputs[i], LOW); // setting one row low
    delayMicroseconds(5);          // giving electronics time to settle down

    for (int j = 0; j < inCount; j++)
    {
      if (digitalRead(inputs[j]) == LOW)
      {
        keyPressed(i, j); // calling keyPressed function if one of the inputs reads low
      }
      else if (keyDown[i][j] != 0)
      { // resetting the key if it is not pressed any more
        resetKey(i, j);
      }
    }
    digitalWrite(outputs[i], HIGH);
    delayMicroseconds(500); // setting the row high and waiting 0.5ms until next cycle
  }
}

void keyPressed(int row, int col)
{

  if (keyDown[row][col] == 0)
  { // if the function is called for the first time for this key

    send_key2(row, col);
    //    digitalWrite(buzzerPin, HIGH);
    //    delayMicroseconds(3500);
    //    digitalWrite(buzzerPin, LOW);
  }
  else if (keyLong[row][col] && keyDown[row][col] > spamSpeed)
  { // if the key has been held long enough to warrant another keystroke set
    send_key2(row, col);
    keyDown[row][col] = 1;
  }
  else if (keyDown[row][col] > longPressDelay)
  { // if the key has been held for longer that longPressDelay, it switches into spam mode
    keyLong[row][col] = true;
  }

  keyDown[row][col]++;
}

void resetKey(int row, int col)
{ // resetting the variables after key is released
  keyDown[row][col] = 0;
  keyLong[row][col] = false;
}

void send_key2(int row, int col)
{
  char text = sysLayout[row][col];
  Serial.println(text);

  if (text == 'L')
  {
    toggle_numlock();
    return;
  }

  if (!numlock)
  {
    digitalWrite(buzzerPin, HIGH);
    delayMicroseconds(4000);
    digitalWrite(buzzerPin, LOW);
    if (text == 'C')
    {
      prevProfile();
      return;
    }
    else if (text == 'D')
    {
      nextProfile();
      return;
    }
  }
  else
  {
    digitalWrite(buzzerPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(buzzerPin, LOW);
  }

  if (profile == 1)
  {
    if (text == 'A')
    {
      bleKeyboard.write(KEY_MEDIA_CALCULATOR);
    }
    else if (text == 'B')
    {
      bleKeyboard.write(KEY_MEDIA_MUTE);
    }
    else
    {
      bleKeyboard.write(layout1[row][col]);
    }
    return;
  }
  else if (profile == 2)
  {
    //    bleKeyboard.write(layout2[row][col]);
    return;
  }

  //  bleKeyboard.write(text);
}

void send_key(char text)
{
  lastActive = seconds();
  Serial.println(text);
  if (bleKeyboard.isConnected())
  {
    if (text == 'A')
    {
      bleKeyboard.write(KEY_MEDIA_CALCULATOR);
      return;
    }
    if (text == 'B')
    {
      bleKeyboard.write(KEY_MEDIA_MUTE);
      return;
    }
    if (text == 'C')
    {
      if (numlock)
      {
        bleKeyboard.write(KEY_BACKSPACE);
      }
      else
      {
        prevProfile();
      }
      return;
    }
    if (text == 'D')
    {
      nextProfile();
      return;
    }
    if (text == 'E')
    {
      bleKeyboard.write(KEY_RETURN);
      return;
    }
    if (text == 'L')
    {
      toggle_numlock();
      return;
    }
    bleKeyboard.write(text);
  }
}

void prevProfile()
{
  profile = profile - 1;
  if (profile <= 1)
  {
    profile = 1;
    warn_buzzer(4000, 10000, 2);
  }
}

void nextProfile()
{
  profile = profile + 1;
  if (profile >= totalProfile)
  {
    profile = totalProfile;
    warn_buzzer(4000, 10000, 2);
  }
}

void callback()
{
}

void set_dot(bool state)
{
  digitalWrite(segmentPins[7], state);
}

void clicky_sound()
{
  warn_buzzer(7000, 0, 1);
}

void toggle_dot()
{
  digitalWrite(segmentPins[7], !digitalRead(segmentPins[7]));
}

void toggle_numlock()
{
  clicky_sound();
  numlock = !numlock;
}

void go_to_sleep()
{
  Serial.println("Going to sleep");
  for (int i = 0; i < outCount; i++)
  {
    if (i == outCount - 1)
    {
      break;
    }
    digitalWrite(outputs[i], LOW);
  }
  for (int i = 0; i < inCount; i++)
  {
    pinMode(inputs[i], LOW);
  }
  esp_deep_sleep_start();
};

void show_character(char character)
{
  if (character == '*')
  {
    for (int j = 0; j < 7; j++)
    {
      digitalWrite(segmentPins[j], LOW);
    }
    return;
  }

  for (int i = 0; i < totalChar; i++)
  {
    if (character == characters[i])
    {
      for (int j = 0; j < 7; j++)
      {
        digitalWrite(segmentPins[j], alphaBits[i][j]);
      }
    }
  }
}

void warn_buzzer(int vol, int freq, int count)
{
  int min_vol = 30;      // min=20
  int min_freq = 100000; // min=2000
  int min_count = 1;     // min=1

  if (vol < min_vol)
  {
    vol = min_vol;
  }
  if (freq < min_freq)
  {
    freq = min_freq;
  }
  if (count < min_count)
  {
    count = min_count;
  }

  for (int i = 1; i < count; i++)
  {
    digitalWrite(buzzerPin, HIGH);
    delayMicroseconds(vol);
    digitalWrite(buzzerPin, LOW);
    delayMicroseconds(freq);
  }

  digitalWrite(buzzerPin, HIGH);
  delayMicroseconds(vol);
  digitalWrite(buzzerPin, LOW);
}
