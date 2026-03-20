/*
 * Task 5C - AVR Control Flow Lab: Phase 3 (IJMP)
 * Hardware: Arduino Uno (ATmega328P)
 */

#include <avr/pgmspace.h>   // pgm_read_word_near

#define LED_PIN  PB5
#define BTN_PIN  PB0

// ── Global state
volatile uint8_t g_action   = 0;   // current action index (0-3)
volatile uint8_t g_mode     = 1;   // selected mode (1=A, 2=B, 3=C)
volatile bool    g_newPress = false;

// ── Debounce helper
static bool buttonEdge() {
  static uint8_t prev = 1;
  uint8_t cur = (PINB >> BTN_PIN) & 1;
  if (cur == 0 && prev == 1) { delay(20); prev = 0; return true; }
  if (cur == 1 && prev == 0) { prev = 1; }
  return false;
}

// ── Action handler prototypes
void action0_normal()   ;
void action1_inverted() ;
void action2_dim()      ;
void action3_reset()    ;

// ── Jump table in program memory
// Each entry is the 16-bit word address of the handler.
// IJMP loads the Z register with one of these addresses.
typedef void (*ActionFn)(void);
const ActionFn actionTable[4] PROGMEM = {
  action0_normal,
  action1_inverted,
  action2_dim,
  action3_reset
};

// ── Action implementations
void action0_normal() {
  // Normal: follow the mode's default ON period
  uint16_t onMs  = (g_mode == 1) ? 500 : (g_mode == 2) ? 100 : 50;
  uint16_t offMs = (g_mode == 1) ? 500 : (g_mode == 2) ? 100 : 50;
  PORTB |=  (1 << LED_PIN); delay(onMs);
  PORTB &= ~(1 << LED_PIN); delay(offMs);
  if (g_mode == 2) {                // Mode B: second pulse
    PORTB |=  (1 << LED_PIN); delay(onMs);
    PORTB &= ~(1 << LED_PIN); delay(700);
  }
}

void action1_inverted() {
  // Inverted: LED is OFF during the "on" period
  uint16_t onMs  = (g_mode == 1) ? 500 : (g_mode == 2) ? 100 : 50;
  uint16_t offMs = (g_mode == 1) ? 500 : (g_mode == 2) ? 700 : 50;
  PORTB &= ~(1 << LED_PIN); delay(onMs);
  PORTB |=  (1 << LED_PIN); delay(offMs);
}

void action2_dim() {
  // Dim simulation: very short ON, long OFF (low duty cycle)
  PORTB |=  (1 << LED_PIN); delay(30);
  PORTB &= ~(1 << LED_PIN); delay(300);
}

void action3_reset() {
  // Reset: snap back to action 0 immediately
  g_action = 0;
}

// ── IJMP dispatcher
/*
 * Reads the handler address from the PROGMEM jump table, loads it
 */
static void dispatchAction(uint8_t idx) {
  // Read handler word-address from flash table
  uint16_t addr = pgm_read_word_near(&actionTable[idx]);

  /*
   * Load Z register pair and execute IJMP:
   *   ZH (r31) ← high byte of word address
   *   ZL (r30) ← low  byte of word address
   *   IJMP     → PC ← Z
   *
   * "r" constraint lets the compiler allocate any general-purpose
   * register; we then move them into r31/r30 explicitly.
   */
  asm volatile(
    "movw r30, %[a] \n\t"   // ZL:ZH ← addr (movw copies a register pair)
    "ijmp           \n\t"   // PC ← Z  (Indirect Jump)
    :
    : [a] "r" (addr)
    : "r30", "r31"           // clobbers Z pair
  );
}

// ── Setup
void setup() {
  DDRB  |=  (1 << LED_PIN);
  PORTB &= ~(1 << LED_PIN);
  DDRB  &= ~(1 << BTN_PIN);
  PORTB |=  (1 << BTN_PIN);

  // Phase 1: RJMP boot-wait
  asm volatile(
    "wait_btn3:      \n\t"
    "sbic %[pinb], 0 \n\t"
    "rjmp wait_btn3  \n\t"
    : : [pinb] "I" (_SFR_IO_ADDR(PINB))
  );
  PORTB |= (1 << LED_PIN); delay(200); PORTB &= ~(1 << LED_PIN); delay(100);

  // Phase 2: count presses (2-second window)
  g_mode = 1;
  unsigned long start = millis();
  while (millis() - start < 2000) {
    if (buttonEdge()) {
      g_mode++;
      if (g_mode > 3) g_mode = 3;
      PORTB |= (1 << LED_PIN); delay(80); PORTB &= ~(1 << LED_PIN);
    }
  }
}

// ── Main loop – Phase 3: IJMP action dispatch
void loop() {
  // Cycle action on button press
  if (buttonEdge()) {
    g_action = (g_action + 1) % 4;
  }

  // IJMP dispatch to current action handler
  dispatchAction(g_action);
}
