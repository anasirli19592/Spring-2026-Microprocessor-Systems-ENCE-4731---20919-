/*
 * Task 5B - AVR Control Flow Lab: Phase 2 (JMP)
 * Hardware: Arduino Uno (ATmega328P)
 */

#define LED_PIN  PB5
#define BTN_PIN  PB0

// ── Debounce helper
// Returns true on a fresh LOW edge (button press), debounced.
static bool buttonPressed() {
  static uint8_t prev = 1;
  uint8_t cur = (PINB >> BTN_PIN) & 1;   // 1 = not pressed, 0 = pressed
  if (cur == 0 && prev == 1) {
    delay(20);                            // debounce
    prev = cur;
    return true;
  }
  prev = cur;
  return false;
}

// ── Mode functions (run forever, never return)

void __attribute__((noreturn)) modeA_entry() {
  // Slow blink: 500 ms ON, 500 ms OFF
  while (true) {
    PORTB |=  (1 << LED_PIN); delay(500);
    PORTB &= ~(1 << LED_PIN); delay(500);
  }
}

void __attribute__((noreturn)) modeB_entry() {
  // Double blink: two 100 ms pulses, 700 ms pause
  while (true) {
    PORTB |=  (1 << LED_PIN); delay(100);
    PORTB &= ~(1 << LED_PIN); delay(100);
    PORTB |=  (1 << LED_PIN); delay(100);
    PORTB &= ~(1 << LED_PIN); delay(700);
  }
}

void __attribute__((noreturn)) modeC_entry() {
  // Fast strobe: 50 ms ON, 50 ms OFF
  while (true) {
    PORTB |=  (1 << LED_PIN); delay(50);
    PORTB &= ~(1 << LED_PIN); delay(50);
  }
}

void setup() {
  // ── I/O init
  DDRB  |=  (1 << LED_PIN);
  PORTB &= ~(1 << LED_PIN);
  DDRB  &= ~(1 << BTN_PIN);
  PORTB |=  (1 << BTN_PIN);   // pull-up

  // ── Phase 1: RJMP boot-wait (same as Task 5A)
  asm volatile(
    "wait_btn2:      \n\t"
    "sbic %[pinb], 0 \n\t"
    "rjmp wait_btn2  \n\t"
    : : [pinb] "I" (_SFR_IO_ADDR(PINB))
  );
  // Confirm first press
  PORTB |= (1 << LED_PIN); delay(200); PORTB &= ~(1 << LED_PIN); delay(100);

  // ── Phase 2: count presses in a 2-second window
  uint8_t presses = 1;          // first press already detected above
  unsigned long start = millis();
  while (millis() - start < 2000) {
    if (buttonPressed()) {
      presses++;
      if (presses > 3) presses = 3;   // cap at 3
      // Acknowledge with a quick blink
      PORTB |= (1 << LED_PIN); delay(80); PORTB &= ~(1 << LED_PIN);
    }
  }

  /*
   * ── JMP to the selected mode
   */
  if (presses == 1) {
    asm volatile("jmp %x[addr]" : : [addr] "i" (modeA_entry));
  } else if (presses == 2) {
    asm volatile("jmp %x[addr]" : : [addr] "i" (modeB_entry));
  } else {
    asm volatile("jmp %x[addr]" : : [addr] "i" (modeC_entry));
  }
}

void loop() {
  // Never reached – all paths end in an infinite mode loop.
}
