// Fuses needs to be set to
// lfuse f1 - 16Mhz internal clock, no Divide clock by 8 internally
// hfuse df - default
// avrdude -p attiny85  -b 19200 -cusbtiny -U lfuse:w:0xf1:m
// avrdude -p attiny85  -b 19200 -cusbtiny -U hfuse:w:0xdf:m
#include "Arduino.h"

;                         // RESET is Attiny85 pin4
#define delayPin A3       // Attiny85 pin2
#define audioPin PB4      // Attiny85 pin3
;                         // GND is Attiny85 pin4
#define efeSamplePin PB0  // Attiny85 pin5
#define mainSamplePin PB1 // Attiny85 pin6
#define txPin PB2         // Attiny85 pin7
;                         // VCC is Attiny85 pin8

// Default Detector Timings, has to be multiple of 4us
#define txPulseMicros 52    // Main pulse width
#define mainDelayMicros 12  // Main sample pulse delay
#define mainSampleMicros 52 // Main sample pulse width
#define efeDelayMicros 240  // EFE sample pulse delay
#define efeSampleMicros 52  // EFE sample pulse width
#define txCycleMicros 1000  // total cycle width 1ms
#define readDelayLimit 100  // Wait 100 cycle periods (100ms) before reading delay pot
#define MICROS_ELAPSED 4    // 4us elapsed per each interrupt call

uint8_t intState;
uint8_t readDelayCounter = 0;
boolean readDelayPot = false;
uint8_t varMainDelayMicros = mainDelayMicros;

void setup()
{
    // chip specific adjustment to local oscilator
    OSCCAL = 75;

    pinMode(txPin, OUTPUT);         // Set TX pin to output mode
    pinMode(mainSamplePin, OUTPUT); // Set main sample pin to output mode
    pinMode(efeSamplePin, OUTPUT);  // Set EFE sample pin to output mode
    pinMode(delayPin, INPUT);

    // initialize the timer0
    cli();
    TCCR0A = (1 << WGM01);              // Put timer0 in CTC mode and clear other bits
    TCCR0B = (1 << CS01) | (1 << CS00); // Timer0 prescaling 64 and clearing other bits
    TIMSK |= (1 << OCIE0A);             // enable timer1 compare interrupt
    OCR0A = 0;                          // set the timer0 count  - Count 0 cycles before calling ISR interrupt i.e. each 64cycles i.e. 4us
    sei();
    pinMode(audioPin, OUTPUT);
    analogWrite(audioPin, 127); // Set audioPin with 50% duty cycle PWM
}

void loop()
{
}

// called every 4uS, 64CPU cycles in each 4us
ISR(TIM0_COMPA_vect)
{
    switch (intState)
    {
    case 0:
        OCR0A = txPulseMicros / MICROS_ELAPSED;
        PORTB |= (1 << txPin); // Turn on Mosfet
        intState++;
        break;
    case 1:
        OCR0A = varMainDelayMicros / MICROS_ELAPSED;
        PORTB &= ~(1 << txPin); // Turn off Mosfet
        intState++;
        break;
    case 2:
        OCR0A = mainSampleMicros / MICROS_ELAPSED;
        PORTB &= ~(1 << mainSamplePin); // Turn on mainSample
        intState++;
        break;
    case 3:
        OCR0A = efeDelayMicros / MICROS_ELAPSED;
        PORTB |= (1 << mainSamplePin); // Turn off mainSample
        intState++;
        break;
    case 4:
        OCR0A = efeSampleMicros / MICROS_ELAPSED;
        PORTB &= ~(1 << efeSamplePin); // Turn on efeSample
        intState++;
        break;
    case 5:
        // ~24us (magic number) were spent on this case() code block execution
        uint32_t total = txPulseMicros + varMainDelayMicros + mainSampleMicros + efeDelayMicros + efeSampleMicros + 19;
        OCR0A = (txCycleMicros - total) / MICROS_ELAPSED;
        PORTB |= (1 << efeSamplePin); // Turn off efeSample
        intState = 0;

        readDelayCounter++;
        if (readDelayCounter >= readDelayLimit)
        {
            varMainDelayMicros = mainDelayMicros + analogRead(delayPin) / 10;
            readDelayCounter = 0;
        }

        break;
    }
}
