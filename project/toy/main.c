#include <msp430.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include "libTimer.h"
#include "buzzer.h"
#include "led.h"

#define BUTTONS 15
#define SW1 BIT0       // Button1 (P2.0)
#define SW2 BIT1       // Button2 (P2.1)
#define SW3 BIT2       // Button3 (P2.2)
#define SW4 BIT3       // Button4 (P2.3)

// Rectangle state
int rect_width = screenWidth / 2;    // Initial rectangle width
int rect_height = screenHeight / 2;  // Initial rectangle height
short buzzer_frequency = 2500;       // Initial buzzer frequency (2 kHz)

int radius;
int current_shape = 0;  // 0 - Rectangle, 1 - Circle
int current_color = 0;  // 0 - Black, 1 - Purple, 2 - Blue, 3 - Green, 4 - Yellow, 5 - Orange, 6 - Red

// Function prototypes
void drawRectangle(short color);
void drawCircle(short color);
void drawShape(short color);
void buttonInit();
void timerInit();
void startBuzzer(short frequency);


// Button initialization
void buttonInit() {
    P2REN |= BUTTONS;       // Enable resistor for P2
    P2OUT |= BUTTONS;       // Set resistor to pull-up
    P2DIR &= ~BUTTONS;      // Set P2 as input
    P2IE |= BUTTONS;        // Enable interrupts for P2
    P2IES |= BUTTONS;       // Interrupt on falling edge
    P2IFG &= ~BUTTONS;      // Clear interrupt flags
}

// Function to start the buzzer for a short duration
void startBuzzer(short frequency) {
    buzzer_set_period(frequency);  // Set buzzer frequency
    TA1CCTL0 = CCIE;               // Enable timer interrupt
}

// Simple delay function
void delay(unsigned int count) {
    while (count-- > 0) {
        volatile unsigned int i;
        i = 1000; // Adjust this for the delay speed
        while (i--);
    }
}


// Timer initialization for buzzer duration
void timerInit() {
    TA1CTL = TASSEL_2 + MC_1;      // SMCLK, Up mode
    TA1CCR0 = 1000000;              // ~1 second at 1 MHz SMCLK
    TA1CCTL0 = 0;                  // Disable timer interrupt by default
}

// Function to draw the current shape
void drawShape(short color) {
    clearScreen(COLOR_WHITE);  // Clear the screen before drawing the new shape
    if (current_shape == 0) {
        drawRectangle(color);  // Draw rectangle if current shape is 0
    } else if (current_shape == 1) {
        drawCircle(color);  // Draw circle if current shape is 1
    }
}

// Draw rectangle
void drawRectangle(short color) {
    int rect_x = (screenWidth - rect_width) / 2;
    int rect_y = (screenHeight - rect_height) / 2;
    fillRectangle(rect_x, rect_y, rect_width, rect_height, color);  // Draw rectangle
}

// Draw circle
void drawCircle(short color) {
    int radius = (rect_width < rect_height) ? rect_width / 2 : rect_height / 2;
    int center_x = screenWidth / 2;
    int center_y = screenHeight / 2;
    fillCircle(center_x, center_y, radius, color);  // Draw circle
}

// Function to handle the "TOO BIG" state
void handleTooBig() {
    clearScreen(COLOR_BLACK);                  // Fill the screen with black
    drawString11x16((screenWidth / 2) - 40,      // Center the text horizontally
                  screenHeight / 2 - 8,            // Center the text vertically
                  "TOO BIG", COLOR_RED, COLOR_BLACK); // Display "TOO BIG" in red
    delay(100); 
    or_sr(0x10);                       // Stop the program
}

// Function to handle the "TOO SMALL" state
void handleTooSmall() {
    clearScreen(COLOR_WHITE);                  // Fill the screen with white
    drawString11x16((screenWidth / 2) - 45,      // Center the text horizontally
                  screenHeight / 2 - 8,            // Center the text vertically
                  "TOO SMALL", COLOR_RED, COLOR_WHITE); // Display "TOO SMALL" in red
    delay(100); 
    or_sr(0x10);                       // Stop the program
}

// Main function
void main() {
    configureClocks();  // Set up the clock system
    lcd_init();         // Initialize the LCD
    buttonInit();       // Initialize buttons
    timerInit();        // Initialize the timer
    buzzer_init();      // Initialize the buzzer
    P1DIR |= LEDS;      // Set LED pins as output

    clearScreen(COLOR_WHITE); // Clear the screen
    drawShape(current_color);   // Draw the initial shape with black color

    __enable_interrupt();    // Enable global interrupts

    while (1) {  // Main loop, graphics and sound updates are interrupt-driven
	P1OUT |= LEDS;
	__bis_SR_register(LPM3_bits + GIE); // Enter LPM3 with interrupts enabled
	P1OUT &= ~LEDS;
    }
}

// Interrupt Service Routine for Port 2
void __interrupt_vec(PORT2_VECTOR) Port_2() {
    if (P2IFG & SW1) {       // Button 1 (Increase size)
        P2IFG &= ~SW1;
	
        P1OUT |= LED_RED;    // Turn on red LED
        //led_off(0); 
	P1OUT &= ~LED_GREEN;

        if (rect_width < screenWidth || rect_height < screenHeight) {
            rect_width += 8;
            rect_height += 8;

            // Ensure shape does not exceed screen dimensions
            if (rect_width > screenWidth) rect_width = screenWidth;
            if (rect_height > screenHeight) rect_height = screenHeight;
	    
            clearScreen(COLOR_WHITE);
            drawShape(current_color);  // Redraw the shape with the current color
            buzzer_frequency += 200;
            startBuzzer(buzzer_frequency);
           
            if (rect_width == screenWidth && rect_height == screenHeight) {
                handleTooBig(); // Handle the "TOO BIG" condition
            }
        }
    }

    if (P2IFG & SW2) {       // Button 2 (Decrease size)
        P2IFG &= ~SW2;
	
        P1OUT &= ~LED_RED;   // Turn off red LED
	//led_off(0); 
	P1OUT &= ~LED_GREEN;
	
        if (rect_width > 0 || rect_height > 0) {
            rect_width -= 8;
            rect_height -= 8;

            // Ensure shape does not become negative
            if (rect_width < 0) rect_width = 0;
            if (rect_height < 0) rect_height = 0;

            clearScreen(COLOR_WHITE);
            drawShape(current_color);  // Redraw the shape with the current color

            if (buzzer_frequency > 200) buzzer_frequency -= 200;
            startBuzzer(buzzer_frequency);

            if (rect_width <= 0 && rect_height <= 0) {
                handleTooSmall();  // Handle the "TOO SMALL" condition
            }
        }
    }

    if (P2IFG & SW3) {       // Button 3 (Cycle through shapes)
        P2IFG &= ~SW3;
	P1OUT ^= LED_RED;      // Toggle red LED
	    
	//led_off(0); 
	P1OUT &= ~LED_GREEN;
	    
        // Toggle between rectangle (0) and circle (1)
        if (current_shape == 0) {
            current_shape = 1;  // Change to Circle
        } else {
            current_shape = 0;  // Change to Rectangle
        }
        
        drawShape(current_color);  // Redraw the new shape with the current color
	
        buzzer_frequency = 4000;  // Optionally reset buzzer frequency on shape change
        startBuzzer(buzzer_frequency);  // Play buzzer when the shape changes
    }
	
    if (P2IFG & SW4) {       // Button 4 (Change shape color)
        P2IFG &= ~SW4;
	//led_off(0);
	P1OUT &= ~LED_GREEN;
	
        // Simplified color change logic
        if (current_color < 6) {
            current_color++;  // Increment color
        } else {
            current_color = 0; // Reset to first color
        }

        // Set the color based on the current_color value
        switch (current_color) {
            case 0:
            drawShape(COLOR_BLACK);  // Draw with black color
                break;
            case 1:
                drawShape(COLOR_PURPLE);  // Draw with purple color
                break;
            case 2:
                drawShape(COLOR_BLUE);    // Draw with blue color
                break;
            case 3:
                drawShape(COLOR_GREEN);   // Draw with green color
                break;
            case 4:
                drawShape(COLOR_YELLOW);  // Draw with yellow color
                break;
            case 5:
                drawShape(COLOR_ORANGE);  // Draw with orange color
                break;
            case 6:
                drawShape(COLOR_RED);     // Draw with red color
                break;
        }
        buzzer_frequency = 1000;  // Optionally reset buzzer frequency on color change
        startBuzzer(buzzer_frequency);  // Play buzzer when the color changes

        P1OUT &= ~LEDS;    // Ensure both LEDs are initially off
        int count = 0;
        while (count < 5) {
            P1OUT ^= LED_RED;         // Toggle red LED
            P1OUT ^= LED_GREEN;       // Toggle green LED
            delay(100);               // Delay to control blink speed
            count++;
        }
    }
}

// Timer1_A0 ISR for buzzer duration
void __interrupt_vec(TIMER1_A0_VECTOR) Timer1_A0() {
    buzzer_set_period(0);    // Turn off the buzzer
    TA1CCTL0 = 0;            // Disable timer interrupt
}
