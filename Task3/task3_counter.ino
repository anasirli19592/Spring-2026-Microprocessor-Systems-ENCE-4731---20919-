/*
 * Task 3 - Microprocessor Systems Lab
 */

#include <avr/io.h>
#include <util/delay.h>

void eeprom_wait() {
    while (EECR & (1 << EEPE)); // EEPE (bit 1) stays 1 while write is in progress
}

// Write one byte to EEPROM at given address
void eeprom_write_byte(uint16_t address, uint8_t data) {
    eeprom_wait();
    EEAR = address;         // Set EEPROM Address Register
    EEDR = data;            // Set EEPROM Data Register
    EECR |= (1 << EEMPE);  // Master Write Enable (must be set first)
    EECR |= (1 << EEPE);   // Start EEPROM write
}

// Read one byte from EEPROM at given address
uint8_t eeprom_read_byte(uint16_t address) {
    eeprom_wait();
    EEAR = address;         // Set EEPROM Address Register
    EECR |= (1 << EERE);   // Start EEPROM read (EERE = bit 0)
    return EEDR;            // Return data from EEPROM Data Register
}

// ----------------------------------------------------------------
// Main Program
// ----------------------------------------------------------------

int main(void) {
    // --- Serial (USART) Initialization at 9600 baud ---
    // Baud rate = 9600, F_CPU = 16MHz
    // UBRR = F_CPU / (16 * baud) - 1 = 16000000 / (16 * 9600) - 1 = 103
    UBRR0H = 0;
    UBRR0L = 103;
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);  // Enable TX and RX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 data bits, 1 stop bit

    // --- Load counter from EEPROM on power-up ---
    // This allows the counter to continue from where it was last saved
    uint8_t counter = eeprom_read_byte(0x00);

    // Load the EEPROM value into register R24 using inline AVR assembly
    // R24 is the working register for the counter; address 0x0018 in register file
    asm volatile (
        "sts 0x0018, %0 \n\t"  // Store 'counter' variable value into R24
        :
        : "r" (counter)
    );

    // --- Print startup message via Serial ---
    // Manually send each character over USART
    const char* msg = "Counter started. S=Save, R=Reset\r\n";
    for (uint8_t i = 0; msg[i] != '\0'; i++) {
        while (!(UCSR0A & (1 << UDRE0))); // Wait until TX buffer is empty
        UDR0 = msg[i];
    }

    // Main Loop: Increment R24 every 1 second

    while (1) {
        // --- 1-second delay ---
        // At 16 MHz: 1 second = 16,000,000 CPU cycles
        // _delay_ms(1000) expands to a tight loop consuming ~16,000,000 cycles
        _delay_ms(1000);

        // --- Increment R24 using AVR assembly ---
        // INC instruction: 1 cycle, operates directly on the register
        // R24 is at address 0x0018 in the AVR register file
        asm volatile (
            "lds r24, 0x0018 \n\t"  // Load R24 from its own address (2 cycles)
            "inc r24         \n\t"  // Increment R24 by 1 (1 cycle)
            "sts 0x0018, r24 \n\t"  // Store incremented value back (2 cycles)
            ::: "r24"
        );

        // Read R24 back into C variable for EEPROM/Serial use
        asm volatile (
            "lds %0, 0x0018 \n\t"
            : "=r" (counter)
        );

        // --- Print current counter value over Serial ---
        // Format: "Counter: XXX\r\n"
        const char* prefix = "Counter: ";
        for (uint8_t i = 0; prefix[i] != '\0'; i++) {
            while (!(UCSR0A & (1 << UDRE0)));
            UDR0 = prefix[i];
        }

        // Print counter as decimal digits
        uint8_t temp = counter;
        char digits[4] = {0};
        uint8_t d = 0;
        if (temp == 0) {
            digits[d++] = '0';
        } else {
            while (temp > 0) {
                digits[d++] = '0' + (temp % 10);
                temp /= 10;
            }
        }
        // Print in correct order (digits are stored reversed)
        for (int8_t k = d - 1; k >= 0; k--) {
            while (!(UCSR0A & (1 << UDRE0)));
            UDR0 = digits[k];
        }
        while (!(UCSR0A & (1 << UDRE0))); UDR0 = '\r';
        while (!(UCSR0A & (1 << UDRE0))); UDR0 = '\n';

        // --- Check for incoming serial command ---
        if (UCSR0A & (1 << RXC0)) {  // Check if a byte has been received
            uint8_t cmd = UDR0;      // Read the received byte

            if (cmd == 'S' || cmd == 's') {
                // --- STORE: Save current counter to EEPROM address 0x00 ---
                eeprom_write_byte(0x00, counter);

                const char* saved = "Saved to EEPROM.\r\n";
                for (uint8_t i = 0; saved[i] != '\0'; i++) {
                    while (!(UCSR0A & (1 << UDRE0)));
                    UDR0 = saved[i];
                }
            }
            else if (cmd == 'R' || cmd == 'r') {
                // --- RESET: Zero the counter in R24 and write 0 to EEPROM ---
                asm volatile (
                    "clr r24         \n\t"  // Clear R24 (set to 0), 1 cycle
                    "sts 0x0018, r24 \n\t"  // Store 0 back to R24 address, 2 cycles
                    ::: "r24"
                );
                counter = 0;
                eeprom_write_byte(0x00, 0);

                const char* reset_msg = "Counter reset to 0.\r\n";
                for (uint8_t i = 0; reset_msg[i] != '\0'; i++) {
                    while (!(UCSR0A & (1 << UDRE0)));
                    UDR0 = reset_msg[i];
                }
            }
        }
    }

    return 0;
}
