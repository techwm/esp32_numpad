#include "BuzzerControl.h"

BuzzerControl::BuzzerControl(int pin) {
  pinMode(pin, OUTPUT);
  _pin = pin;
}

void BuzzerControl::playMelody(int melody[], int count, int tempo, float note_ratio, float pause_ratio) 
{
  //  int tempo = 144;
  int wholenote = (60000 * 4) / tempo;
  int divider = 0, noteDuration = 0;
  int notes = count;

  // iterate over the notes of the melody.
  // Remember, the array is twice the number of notes (notes + durations)
  for (int i = 0; i < notes * 2 ; i = i + 2) {

    // calculates the duration of each note
    divider = melody[i + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(this->_pin, melody[i], noteDuration * note_ratio);

    // Wait for the specief duration before playing the next note.
    delay(noteDuration * pause_ratio);

    // stop the waveform generation before the next note.
    noTone(this->_pin);
  }
}

void BuzzerControl::play_start_sound() {
  int melody[] = {
    NOTE_E6, 8, NOTE_G6, 8, NOTE_E7, 8,
    NOTE_C7, 8, NOTE_D7, -8, NOTE_G7, 8,
  };
  this->playMelody(melody, sizeof(melody) / sizeof(melody[0]), 200, 0.4, 0.7);
}

void BuzzerControl::play_pulse_sound() {
  int melody[] = {
    NOTE_C6, 8, NOTE_C7, 16, NOTE_C8, 16,
  };
  this->playMelody(melody, sizeof(melody) / sizeof(melody[0]), 200, 0.7, 0.2);
}
