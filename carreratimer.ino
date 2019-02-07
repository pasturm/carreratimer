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
unsigned long lastTriggerTime1, lastTriggerTime2;
volatile unsigned long triggerTime1, triggerTime2;
volatile boolean triggered1, triggered2;
volatile boolean reset = false;
int lapNumber1=0, lapNumber2=0;  // lap counters
unsigned long lapTime1, lapTime2, clock1, clock2;
unsigned long lapRecord1 = 0, lapRecord2 = 0;
int i1, i2;  // counters
boolean first1 = true, first2 =  true;
int displayTime = 3;  // time (s) to display the lap time
int notes[] = {1047,1319,1568};  // frequencies for tone()

// string buffers for LCD
// https://www.baldengineer.com/arduino-lcd-display-tips.html
char line0[17];
char line1[17];
char fstr_lapTime1[7];
char fstr_lapTime2[7];
char fstr_lapRecord1[7];
char fstr_lapRecord2[7];

// interrupt service routines
void isr1()
{
	if (triggered1) return;  // wait until trigger is noticed in loop()
	triggerTime1 = millis();
	triggered1 = true;
}
void isr2()
{
	if (triggered2) return;
	triggerTime2 = millis();
	triggered2 = true;
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

//  lcd.setCursor(0, 0);
//  lcd.print(" 9    4.51    4s");
//  lcd.setCursor(0, 1);
//  lcd.print("11    4.28    4s");
//  delay(300000);
}

void loop()
{
	// if reed switch 1 is triggered
	if (triggered1)
	{
		// ignore if lap time is too short (debouncing)
		if ((triggerTime1 - lastTriggerTime1) < 2000.0)
			{
				triggered1 = false;
				return;
			}

		if (first1)  // first trigger to start the timer
		{
			lastTriggerTime1 = triggerTime1;
			triggered1 = false;
			first1 = false;
			return;
		}

		lapTime1 = (triggerTime1 - lastTriggerTime1);
		lapTime1 = min(lapTime1, 999990);  // maximum is 999.99 s
		lastTriggerTime1 = triggerTime1;
		triggered1 = false;  // re-arm for next time
		lapNumber1++;
		lapNumber1 %= 100;  // count up to 99
		if (lapTime1 < lapRecord1 || lapRecord1 == 0)
		{
			lapRecord1 = lapTime1;
		}
		i1 = 0;  // reset display time counter

		if (lapNumber1 % 10 == 0)  // play sound every 10 laps
		{
			for (int i = 3; i > -1; i--)
			{
				tone(11, notes[i], 200);
				delay(150);
			}
		}
	}

	// if reed switch 2 is triggered
	if (triggered2)
	{
		// ignore if lap time is too short (debouncing)
		if ((triggerTime2 - lastTriggerTime2) < 2000.0)
		{
			triggered2 = false;
			return;
		}

		if (first2)  // first trigger to start the timer
		{
			lastTriggerTime2 = triggerTime2;
			triggered2 = false;
			first2 = false;
			return;
		}

		lapTime2 = (triggerTime2 - lastTriggerTime2);
		lapTime2 = min(lapTime2, 999990);  // maximum is 999.99 s
		lastTriggerTime2 = triggerTime2;
		triggered2 = false;  // re-arm for next time
		lapNumber2++;
		lapNumber2 %= 100;  // count up to 99
		if (lapTime2 < lapRecord2 || lapRecord2 == 0)
		{
			lapRecord2 = lapTime2;
		}
		i2 = 0;  // reset display time counter

		if (lapNumber2 % 10 == 0)  // play sound every 10 laps
		{
			for (int i = 0; i < 3; i++)
			{
				tone(11, notes[i], 200);
				delay(150);
			}
		}
	}

	// reset lap counter and best time
	if (reset)
	{
		lapNumber1=0;
		lapNumber2=0;
		lapRecord1=0;
		lapRecord2=0;
		first1 = true;
		first2 = true;
		lapTime1 = 0;
		lapTime2 = 0;
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
	// lap clocks
	clock1 = (millis() - triggerTime1);
	clock2 = (millis() - triggerTime2);

	// generate display strings
	// line0
	// https://www.microchip.com/webdoc/AVRLibcReferenceManual/group__avr__stdlib_1ga060c998e77fb5fc0d3168b3ce8771d42.html
	dtostrf(lapRecord1/1000.0,6,2,fstr_lapRecord1);
	if (i1 < displayTime || first1)
	{
		dtostrf(lapTime1/1000.0,6,2,fstr_lapTime1);
		sprintf(line0, "%2d %6s %6s", lapNumber1, fstr_lapRecord1, fstr_lapTime1);
		i1++;
	}
	else
	{
		sprintf(line0, "%2d %6s %5ds", lapNumber1, fstr_lapRecord1, clock1/1000);
	}
	// line 1
	dtostrf(lapRecord2/1000.0,6,2,fstr_lapRecord2);
	if (i2 < displayTime || first2)
	{
		dtostrf(lapTime2/1000.0,6,2,fstr_lapTime2);
		sprintf(line1, "%2d %6s %6s", lapNumber2, fstr_lapRecord2, fstr_lapTime2);
		i2++;
	}
	else
	{
		sprintf(line1, "%2d %6s %5ds", lapNumber2, fstr_lapRecord2, clock2/1000);
	}

	lcd.setCursor(0,0);
	lcd.print(line0);
	lcd.setCursor(0,1);
	lcd.print(line1);
}
