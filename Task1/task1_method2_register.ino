

#include <avr/io.h>      // Provides DDRB, PORTB, DDB5, PORTB5
#include <util/delay.h>  // Provides _delay_ms() — cycle-accurate

void setup() {

  DDRB |= (1 << DDB5);
}

void loop() {

  PORTB |= (1 << PORTB5);
  _delay_ms(500); // Busy-wait 500 ms (cycle-counted by compiler)


  PORTB &= ~(1 << PORTB5);
  _delay_ms(500); // Busy-wait 500 ms → total period = 1000 ms

}
