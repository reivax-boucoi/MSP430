Short description of MSP5510-STK_Full_Example demo program

The demo example demonstrates the following modules:

1. USB devices: 1 CDC,  1 MSD with 2 LUNs

	- CDC USB device returns echo. When you plug in USB cable a new virtual COM port it will appear in your Device Manager list. Use your favourite terminal program (like Hyper terminal or Putty) for Virtual COM port opening.

	- MSDevice USB contains 1 Mass storage device emulated in MSP430F5510 program memmory and 1 Mass storage device in SD card. You can see them in your My computer list. Note that when you plug in SD card you may wait up to 10-15 sec for SD card initialization

2. LCD1x9 writing and reading

	- Pressing BUT1 will measure and display the battery voltage level in mV. If you have no battery in BAT connector you will see something about 4314 mV, because there is on board battery charger and it is trying to charge battery with its maximum voltage level. 

	- Pressing of BUT2 will show OLIMEX on the display

3. SD card reading and writing

	- Find and choose the new Mass storage Device in your My computer list and you will be able to write or read data from the SD card. 

4. Battery measurement and display using A0 input
	
5. LED blinking

	-Greed LED blink using Timer1 interrupt and the external 32768 crystal clock source
	
6. BUT1 and BUT2 reading

	- Pressing of BUT 1 and BUT2 will change LCD text and blinking LED period. Note that the buttons have to be hold pressed for 1s at least.

Note:

MSP5510-STK board has embedded USB HID bootloader and you can download the firmware using the Texas Instrumment tool(..MSP430 USB Firmware Upgrade Example\BSL_USB_GUI.exe without the need for JTAG device).

The output firmware file is generated in ..\MSP-5510STK_Full_Example\MSP-5510STK_Example\Debug\Exe directory. For MSP5510_STK_Full example project it is MSP-5510STK_Example.txt