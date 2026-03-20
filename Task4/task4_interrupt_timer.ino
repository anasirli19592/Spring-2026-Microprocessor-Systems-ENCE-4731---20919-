/*
 * Task 4: Interrupt-Driven 10ms Pulse Generator
 * Microprocessor Systems Lab - Lecture 4
 */

#include <avr/io.h>
#include <avr/interrupt.h>

/* ── Shared state between ISRs ── */
volatile uint8_t countdown = 0;   // ms remaining; 0 = output is OFF

/* ──────────────────────────────────────────────────────────────
 * ISR: External Interrupt 0  (D2 / PD2 falling edge)
 * Triggered when the button is pressed (pull-up → line goes LOW)
 * ────────────────────────────────────────────────────────────── */
ISR(INT0_vect)
{
    /* Turn output pin HIGH immediately */
    PORTD |= (1 << PD5);

    /* Arm the 10 ms countdown */
    countdown = 10;

    /* Reset timer count so first tick is exactly 1 ms from now */
    TCNT1 = 0;
}


//   ISR: Timer1 Compare Match A  (fires every 1 ms)
//  Counts down and turns output LOW when countdown reaches zero
ISR(TIMER1_COMPA_vect)
{
    if (countdown > 0)           // only act when a pulse is active
    {
        countdown--;             // decrement the millisecond counter

        if (countdown == 0)      // 10 ticks elapsed → end the pulse
        {
            PORTD &= ~(1 << PD5);   // set output LOW
        }
    }
}


// setup()  - one-time hardware initialisation

void setup(void)
{
    /* ── I/O direction ── */
    DDRD &= ~(1 << PD2);        // PD2 (D2) = input  (button)
    PORTD |=  (1 << PD2);       // enable internal pull-up on PD2

    DDRD |=  (1 << PD5);        // PD5 (D5) = output (pulse / LED)
    PORTD &= ~(1 << PD5);       // start LOW

    /* ── Timer1 – CTC mode, 1 ms interval ──
     *   WGM13:WGM10 = 0100  → CTC, TOP = OCR1A
     *   CS12:CS10   = 011   → prescaler = 64
     *   TCCR1A: COM1A1=0, COM1A0=0, WGM11=0, WGM10=0
     *   TCCR1B: ICNC1=0, ICES1=0, WGM13=0, WGM12=1, CS12=0, CS11=1, CS10=1
     */
    TCCR1A = 0x00;               // normal port operation; WGM11,WGM10 = 00
    TCCR1B = (1 << WGM12)        // CTC mode (WGM12 = 1)
           | (1 << CS11)         // prescaler 64: CS12=0, CS11=1, CS10=1
           | (1 << CS10);

    OCR1A  = 249;                // count 0..249 → 250 ticks × 4 µs = 1 ms
    TCNT1  = 0;                  // reset counter

    /* Enable Timer1 compare-match A interrupt */
    TIMSK1 |= (1 << OCIE1A);    // set OCIE1A bit in Timer/Counter1 Interrupt Mask

    /* ── External Interrupt 0 (INT0) on PD2 – falling edge ──
     *   ISC01=1, ISC00=0 --> falling edge triggers INT0
     *   EICRA bits [1:0] control INT0 sense
     */
    EICRA = (EICRA & 0xFC)       // clear ISC01, ISC00
          | (1 << ISC01);        // ISC01=1, ISC00=0 → falling edge
    EIMSK |= (1 << INT0);        // enable INT0

    /* ── Enable global interrupts ── */
    sei();                       // sets I-bit in SREG; no ISR can fire before this
}


// loop()  - intentionally empty
// All behaviour is handled by the two ISRs above

void loop(void)
{
    /* empty - CPU idles here between interrupts */
}
