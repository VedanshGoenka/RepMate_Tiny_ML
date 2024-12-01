#include "buzzer.h"

void buzzerSetup() {
  pinMode(D8, OUTPUT); // Set pin D8 as output
  digitalWrite(D8, LOW); // Initialize the pin to LOW
}

void buzz(int frequency, long duration) {
  int period = 1000000L / frequency; // Calculate the period of the frequency
  long cycles = frequency * duration / 1000; // Calculate the number of cycles for the duration
  for (long i = 0; i < cycles; i++) {
    digitalWrite(D8, HIGH); // Set the pin to HIGH
    delayMicroseconds(period / 2); // Wait for half the period
    digitalWrite(D8, LOW); // Set the pin to LOW
    delayMicroseconds(period / 2); // Wait for half the period
  }
}

void testBuzzer() {
    digitalWrite(D8, HIGH);
    delay(1);
    digitalWrite(D8, LOW);
    delay(1);
}