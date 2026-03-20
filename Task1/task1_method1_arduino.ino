

const int LED_PIN = 13; // Arduino pin 13 = PB5 on ATmega328P

void setup() {
  // Configure pin 13 as a digital output.

  pinMode(LED_PIN, OUTPUT);
}

void loop() {

  digitalWrite(LED_PIN, HIGH);
  delay(500); // Wait 500 ms (half-period for 1 Hz)


  digitalWrite(LED_PIN, LOW);
  delay(500); // Wait 500 ms → total period = 1000 ms = 1 Hz

}
