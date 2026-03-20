/*
 * Task 5A - AVR Control Flow Lab: Phase 1 (RJMP)
 */

// ── Pin aliases ──────────────────────────────────────────────
#define LED_PIN   PB5   // Digital 13  (PORTB bit 5)
#define BTN_PIN   PB0   // Digital  8  (PORTB bit 0) → PINB bit 0

void setup() {
  // Configure LED pin as output
  DDRB  |=  (1 << LED_PIN);   // PB5 = output
  PORTB &= ~(1 << LED_PIN);   // LED off

  // Configure button pin as input with internal pull-up
  // When not pressed  → PB0 is HIGH (pulled up)
  // When pressed      → PB0 is LOW  (shorted to GND)
  DDRB  &= ~(1 << BTN_PIN);   // PB0 = input
  PORTB |=  (1 << BTN_PIN);   // enable internal pull-up

  /*
   * ── Phase 1: RJMP tight polling loop
   */
  asm volatile(
    "wait_btn:       \n\t"   // local label – RJMP target
    "sbic %[pinb], 0 \n\t"   // skip next if PINB,0 == 0 (pressed)
    "rjmp wait_btn   \n\t"   // not pressed yet → loop back
    :                         // no output operands
    : [pinb] "I" (_SFR_IO_ADDR(PINB))  // input: PINB as I/O address constant
  );

  // ── Button pressed: flash LED for 200 ms ──────────────────
  PORTB |=  (1 << LED_PIN);  // LED ON
  delay(200);
  PORTB &= ~(1 << LED_PIN);  // LED OFF
  delay(100);
}

void loop() {
  // After the boot-wait phase the main loop just keeps the LED
  // toggling slowly so the demo is visually obvious.
  PORTB ^= (1 << LED_PIN);   // toggle
  delay(500);
}
