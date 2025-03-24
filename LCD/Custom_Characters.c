#include "xc.h"
#include <stdint.h>

// CONFIGURATION BITS
#pragma config FNOSC = FRC  // Internal Fast RC oscillator

// Simple delay function (approximate, not precise)
void delay_ms(uint16_t ms) {
    while (ms--) {
        uint16_t i = 1000;  // 4 cycles per iteration, so 1000 x 4 = 4000 cycles (1 ms)
        
        while (i--) {
            asm("repeat #3");  // Repeat NOP 3 times (4 cycles total)
            asm("nop");
        }
    }
}

void delay_us(unsigned int microseconds) {
    while (microseconds--) {
        asm("repeat #12");  // 12 instruction cycles
        asm("nop");         // Do nothing, just waste time
    }
}

// Define LCD control pins (connected to PORTD)
#define RS LATDbits.LATD0  // Register Select pin (RS)
#define EN LATDbits.LATD1  // Enable pin (EN)
#define D4 LATDbits.LATD2  // Data line 4
#define D5 LATDbits.LATD3  // Data line 5
#define D6 LATDbits.LATD4  // Data line 6
#define D7 LATDbits.LATD5  // Data line 7

// Function to pulse the Enable pin (lets LCD know data is ready)
void LCD_EnablePulse() {
    EN = 1;
    delay_us(1);  // Small delay (Enable pulse width)
    EN = 0;
    delay_us(1);  // Small delay (Enable pulse width)
}

// Send 4 bits of data to the LCD
void LCD_Send4Bits(unsigned char data) {
    D4 = (data >> 0) & 0x01;  // Send bit 0
    D5 = (data >> 1) & 0x01;  // Send bit 1
    D6 = (data >> 2) & 0x01;  // Send bit 2
    D7 = (data >> 3) & 0x01;  // Send bit 3
    LCD_EnablePulse();
}

// Send a command to the LCD
void LCD_Command(unsigned char cmd) {
    RS = 0;  // Command mode
    LCD_Send4Bits(cmd >> 4);  // Send higher nibble
    LCD_Send4Bits(cmd & 0x0F);  // Send lower nibble
    delay_ms(2);  // Wait for command to finish
}

// Send a single character to the LCD
void LCD_Char(unsigned char data) {
    RS = 1;  // Data mode
    LCD_Send4Bits(data >> 4);  // Send higher nibble
    LCD_Send4Bits(data & 0x0F);  // Send lower nibble
}

// Send a string (text) to the LCD
void LCD_String(const char *str) {
    while (*str) {  // Loop until null character
        LCD_Char(*str);  // Send each character
        str++;  // Move to the next character
    }
}

// Set the cursor to a specific row and column
void LCD_SetCursor(unsigned char row, unsigned char col) {
    unsigned char address;

    if (row == 1) {
        address = 0x80 + col;  // First line
    } else if (row == 2) {
        address = 0xC0 + col;  // Second line
    }

    LCD_Command(address);  // Send cursor position command
}

// Initialize the LCD
void LCD_Init() {
    delay_ms(20);  // Wait after power on

    LCD_Send4Bits(0x03);  // Initialization step 1
    delay_ms(5);

    LCD_Send4Bits(0x03);  // Initialization step 2
    delay_ms(5);

    LCD_Send4Bits(0x03);  // Initialization step 3
    delay_ms(5);

    LCD_Send4Bits(0x02);  // Set to 4-bit mode
    delay_ms(5);

    LCD_Command(0x28);  // 4-bit, 2 line, 5x8 font
    LCD_Command(0x0C);  // Display ON, Cursor OFF
    LCD_Command(0x06);  // Entry mode, cursor moves right
    LCD_Command(0x01);  // Clear display
    delay_ms(5);
}

void LCD_CreateChar(unsigned char location, const uint8_t *pattern) {
    location &= 0x07;  // Limit location to 0-7 (8 custom chars)
    LCD_Command(0x40 | (location << 3));  // Set CGRAM address (location * 8)

    for (uint8_t i = 0; i < 8; i++) {
        LCD_Char(pattern[i]);  // Write each byte (row of pixels)
    }
}


int main(void) {
    // Set LCD pins as output
    TRISDbits.TRISD0 = 0;
    TRISDbits.TRISD1 = 0;
    TRISDbits.TRISD2 = 0;
    TRISDbits.TRISD3 = 0;
    TRISDbits.TRISD4 = 0;
    TRISDbits.TRISD5 = 0;

    LATD = 0x00;

    LCD_Init();
    
    uint8_t ESAB[8][8] =
    {
        {
            0b00001, // Row 1, Col 7
            0b00010,
            0b00100,
            0b00111,
            0b00000,
            0b00111,
            0b00100,
            0b00110
        },
        {
            0b11111, // Row 1, Col 8
            0b00000,
            0b00000,
            0b11111,
            0b00000,
            0b01110,
            0b01000,
            0b01110
        },
        {
            0b11111, // Row 1, Col 9
            0b00000,
            0b00000,
            0b11111,
            0b00000,
            0b11101,
            0b10101,
            0b11101
        },
        {
            0b11110, //Row 1, Col 10
            0b00010,
            0b00100,
            0b11000,
            0b00000,
            0b10000,
            0b01000,
            0b11000
        },
        {
            0b00100, // Row 2, Col 7
            0b00111,
            0b00000,
            0b00111,
            0b01000,
            0b10000,
            0b11111,
            0b00000
        },
        {
            0b00010, // Row 2, Col 8
            0b01110,
            0b00000,
            0b11111,
            0b00000,
            0b00000,
            0b11111,
            0b00000
        },
        {
            0b10101, // Row 2, Col 9
            0b10101,
            0b00000,
            0b11111,
            0b00000,
            0b00000,
            0b11111,
            0b00000
        },
        {
            0b01000, // Row 2, Col 10
            0b10000,
            0b00000,
            0b11000,
            0b01000,
            0b10000,
            0b00000,
            0b00000
        }
    };
    
    for(uint8_t k = 0; k<8; k++)
    {
        LCD_CreateChar(k, ESAB[k]);
    }
    
    LCD_SetCursor(1,6);
    
    for(uint8_t l = 0; l<4; l++)
    {
        LCD_Char(l);
    }
    
    LCD_SetCursor(2,6);
    
     for(uint8_t m = 4; m<8; m++)
    {
        LCD_Char(m);
    }

    while (1) {
 
    }

    return 0;
}