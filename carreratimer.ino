// Carrera race track timer
//
// Lap timing for a Carrera slot car racing track using an Arduino Uno
// and two reed switches to detect the magnet of the cars.
// The lap counts, current lap times and lap records are displayed on a 16x2 LCD.
// Lap counters and timers can be reset with a pushbutton.
//
// Author: Patrick Sturm
// January 2019

#include <LiquidCrystal.h>

// initialize the LCD
LiquidCrystal lcd(9, 8, 7, 6, 5, 4);

// initialize variables
unsigned long lastTriggerTime[2];  // arrays of lenght 2 for the two cars
volatile unsigned long triggerTime[2];
volatile boolean triggered[2];
volatile boolean reset = false;
int lapNumber[2] = {0, 0};  // lap counter
unsigned long lapTime[2], clock[2];
unsigned long lapRecord[2] = {0, 0};
int ii[2];  // counters
boolean first[2] = {true,  true};
int displayTime = 3;  // time (s) to display the lap time
int notes[] = {1047,1319,1568};  // frequencies for tone()

// string buffers for LCD
// https://www.baldengineer.com/arduino-lcd-display-tips.html
char line[2][17];
char fstr_lapTime[2][7];
char fstr_lapRecord[2][7];

// interrupt service routines
void isr1()
{
	if (triggered[0]) return;  // wait until trigger is noticed in loop()
	triggerTime[0] = millis();
	triggered[0] = true;
}
void isr2()
{
	if (triggered[1]) return;
	triggerTime[1] = millis();
	triggered[1] = true;
}
// pin change interrupt (as there are only 2 external interrupts on the Uno)
// http://gammon.com.au/interrupts
// https://thewanderingengineer.com/2014/08/11/arduino-pin-change-interrupts/
ISR(PCINT0_vect)  // Port B, PCINT0 - PCINT7
{
	if (reset) return;
	reset = true;
}

void setup()
{
	pinMode(2, INPUT_PULLUP);  // interrupt pin for reed switch 1
	pinMode(3, INPUT_PULLUP);  // interrupt pin for reed switch 2
	attachInterrupt(digitalPinToInterrupt(2), isr1, FALLING);
	attachInterrupt(digitalPinToInterrupt(3), isr2, FALLING);
	pinMode(10, INPUT_PULLUP);  // pin for push putton
	PCMSK0 |= 0b00000100;    // mask to turn on pin D10, PCINT3. Alternative: PCMSK0 |= bit (PCINT2);
	PCIFR  |= bit (PCIF0);   // clear all interrupt flags
	PCICR |= 0b00000001; // enable pin change interrupts for D8 to D13. Alternative: PCICR  |= bit (PCIE0);
	pinMode(11, OUTPUT);  // pin for piezo buzzer

	lcd.begin(16, 2);  // set up the number of columns and rows on the LCD
	lcd.print("Rundenz");
	lcd.print((char)0x84);  // 0x84 = ä, 0x8E = Ä, 0x94 = ö, 0x99 = Ö, 0x81 = ü, 0x9A = Ü
	lcd.print("hler");
	lcd.setCursor(0, 1);  // set the cursor to column 0, line 1
	lcd.print("und Zeitmessung");

	// switch LED_BUILTIN off
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);

	delay(3000);  // show starting message for 3 s
}

void loop()
{
	for (int i = 0; i < 2; i++)  // loop over the cars
	{
		// if reed switch is triggered
		if (triggered[i])
		{
			// ignore if lap time is too short (debouncing)
			if ((triggerTime[i] - lastTriggerTime[i]) < 2000.0)
			{
				triggered[i] = false;
				return;
			}

			if (first[i])  // first trigger to start the timer
			{
				lastTriggerTime[i] = triggerTime[i];
				triggered[i] = false;
				first[i] = false;
				return;
			}

			lapTime[i] = (triggerTime[i] - lastTriggerTime[i]);
			lapTime[i] = min(lapTime[i], 999990);  // maximum is 999.99 s
			lastTriggerTime[i] = triggerTime[i];
			triggered[i] = false;  // re-arm for next time
			lapNumber[i]++;
			lapNumber[i] %= 100;  // count up to 99
			if (lapTime[i] < lapRecord[i] || lapRecord[i] == 0)
			{
				lapRecord[i] = lapTime[i];
			}
			ii[i] = 0;  // reset display time counter

			if (lapNumber[i] % 10 == 0)  // play sound every 10 laps
			{
				for (int j = 0; j < 3; j++)
				{
					if (i == 0)
					{
						tone(11, notes[2-j], 200);
					}
					else
					{
						tone(11, notes[j], 200);
					}
					delay(150);
				}
			}
		}
	}

	// reset lap counter and best time
	if (reset)
	{
		for (int j = 0; j < 2; j++)
		{
			lapNumber[j] = 0;
			lapRecord[j] = 0;
			first[j] = true;
			lapTime[j] = 0;
		}
		reset = false;
	}

	// update LCD
	updateDisplay();

	// wait for 1 s
	delay(1000);

}  // end of loop

// update display function
void updateDisplay()
{
	for (int i = 0; i < 2; i++)  // loop over the the two lines of the LCD
	{
		// lap clocks
		clock[i] = (millis() - triggerTime[i]);

		// generate display strings
		dtostrf(lapRecord[i]/1000.0, 6, 2, fstr_lapRecord[i]);
		// https://www.microchip.com/webdoc/AVRLibcReferenceManual/group__avr__stdlib_1ga060c998e77fb5fc0d3168b3ce8771d42.html
		if (ii[i] < displayTime || first[i])
		{
			dtostrf(lapTime[i]/1000.0, 6, 2, fstr_lapTime[i]);
			sprintf(line[i], "%2d %6s %6s", lapNumber[i], fstr_lapRecord[i], fstr_lapTime[i]);
			ii[i]++;
		}
		else
		{
			sprintf(line[i], "%2d %6s %5ds", lapNumber[i], fstr_lapRecord[i], clock[i]/1000);
		}

		lcd.setCursor(0, i);
		lcd.print(line[i]);
	}
}
