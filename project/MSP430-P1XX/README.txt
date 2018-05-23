Example projects for Olimex boards: MSP430-P1XX and MSP430-P249.
Compiled and tested with IAR for MSP 5.40.
Programmed with Olimex MSP430-JTAG-Tiny-V2.

Demos:
Blink - Blink the green LED on the board (P6.0)
Button - changes the state of the LED depending on the state of the button. The LED is: ON when you press the button; OFF - when you release it.
UART - Print initial message on the console (baudrate 9600) and then returns echo (next ASCII symbol) on the terminal.

Remember to select your board's microcontroller (and the respective configuration in the UART example for either 1XX or 2XX) in the project settings!