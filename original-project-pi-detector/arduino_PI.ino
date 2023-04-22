// Arduino Pulse Induction Metal Detector

// Pin assignments
byte txPin = 8;          // Assign pin 8 to TX output
byte mainSamplePin = 9;  // Assign pin 9 to main sample pulse
byte efeSamplePin = 10;  // Assign pin 10 to EFE sample pulse
byte audioPin = 11;      // Assign pin 11 to audio chopper
byte boostPin = 12;      // Assign pin 12 to boost switch
byte delayPin = A0;      // Assign delay pot to A0

// Program constants
const float normalPower = 50E-6;       // Normal TX-on time (50us)
const float boostPower = 100E-6;       // Boost TX-on time (100us)
const float clockCycle = 62.5E-9;      // Time for one clock cycle (1/16MHz)
const unsigned long maxCount = 65535;  // Value of 2^16 - 1
const byte readDelayLimit = 100;       // Wait 100 TX periods (100ms) before reading delay pot
const byte mosfetOn = HIGH;            // Mosfet turns on when transmitter input high
const byte mosfetOff = LOW;            // Mosfet turns off when transmitter input low
const byte syncDemodOn = LOW;          // Sample gate turns on when input high
const byte syncDemodOff = HIGH;        // Sample gate turns off when input low

// Detector timings
float txOn = normalPower;        // TX-on time using normal power mode
float defMainDelay = 10E-6;      // Default main sample delay (10us)
float mainDelay = defMainDelay;  // Main sample pulse delay
float mainSample = 50E-6;        // Main sample pulse width (50us)
float efeDelay = 240E-6;         // EFE sample pulse delay (240us)
float efeSample = mainSample;    // EFE sample pulse width (same as main sample)
float txPeriod = 1E-3;           // TX period (1ms)

// Timing offsets
float txOnOffset = 3E-6;         // TX-on pulse offset (3us)
float mainDelayOffset = 4.2E-6;  // Main delay pulse offset (4.2us)
float mainSampleOffset = 3E-6;   // Main sample pulse offset (3us)
float efeDelayOffset = 12E-6;    // EFE delay pulse offset (12us)
float efeSampleOffset = 4E-6;    // EFE sample pulse offset (4us)
float txPeriodOffset = 30E-6;    // TX period offset (30us)

// Program variables
float temp1, temp2, temp3, temp4, temp5, temp6;  // Intermediate calculation variables
word txOnCount;                                  // TX pulse
word mainDelayCount;                             // Main sample delay
word mainSampleCount;                            // Main sample pulse
word efeDelayCount;                              // EFE sample delay
word efeSampleCount;                             // EFE sample pulse
word txPeriodCount;                              // TX period
word delayVal = 0;                               // Delay pot value
boolean readDelayPot = false;                    // Delay pot read (true or false)
byte intState = 0;                               // Interrupt state machine
byte readDelayCounter = 0;                       // Read delay pot counter

void setup() {
  pinMode(txPin, OUTPUT);           // Set TX pin to output mode
  pinMode(mainSamplePin, OUTPUT);   // Set main sample pin to output mode
  pinMode(efeSamplePin, OUTPUT);    // Set EFE sample pin to output mode
  pinMode(boostPin, INPUT_PULLUP);  // Set Boost switch pin to input mode with pullup resistor
  calcTimerValues();                // Calculate all timer values
  noInterrupts();                   // Disable interrupts
  TCCR1A = 0;                       // Initialize Timer1 registers
  TCCR1B = 0;
  TIMSK0 = 0;                       // Clear Timer0 mask register to eliminate jitter
  TCNT1 = txOnCount;                // Load Timer1 with TX-on count
  TCCR1B |= (1 << CS10);            // No prescaling for Timer1
  TIMSK1 |= (1 << TOIE1);           // Enable Timer1 overflow interrupt
  interrupts();                     // Enable interrupts
  analogWrite(audioPin, 127);       // Set audioPin with 50% duty cycle PWM
}

void calcTimerValues() {
  if (digitalRead(boostPin) == HIGH) {                   // Get boost switch position
    txOn = normalPower;                                  // Set TX-on to 50us if HIGH
  } else {
    txOn = boostPower;                                   // Set TX-on to 100us if LOW
  }
  temp1 = (txOn - txOnOffset) / clockCycle;
  txOnCount = maxCount - int(temp1);                     // TX-on count for Timer1
  temp2 = (mainDelay - mainDelayOffset) / clockCycle;
  mainDelayCount = maxCount - int(temp2);                // Main sample delay count for Timer1
  temp3 = (mainSample - mainSampleOffset) / clockCycle;
  mainSampleCount = maxCount - int(temp3);               // Main sample pulse count for Timer1
  temp4 = (efeDelay - efeDelayOffset) / clockCycle;
  temp4 -= temp3 + temp2;
  efeDelayCount = maxCount - int(temp4);                 // EFE sample delay count for Timer1
  temp5 = (efeSample - efeSampleOffset) / clockCycle;
  efeSampleCount = maxCount - int(temp5);                // EFE sample pulse count for Timer 1
  temp6 = (txPeriod - txPeriodOffset) / clockCycle;
  temp6 -= temp1 + temp2 + temp3 + temp4 + temp5;
  txPeriodCount = maxCount - int(temp6);                 // TX period count for Timer1
}

ISR(TIMER1_OVF_vect) {
  switch (intState) {
    case 0:
      TCNT1 = txOnCount;                           // Load Timer1 with TX-ON count
      digitalWrite(txPin, mosfetOn);               // Turn on Mosfet
      intState = 1;
      break;
    case 1:
      TCNT1 = mainDelayCount;                      // Load Timer1 with main sample delay count
      digitalWrite(txPin, mosfetOff);              // Turn off Mosfet
      intState = 2;
      break;
    case 2:
      TCNT1 = mainSampleCount;                     // Load Timer1 with main sample pulse count
      digitalWrite(mainSamplePin, syncDemodOn);    // Turn on main sample gate
      intState = 3;
      break;
    case 3:
      TCNT1 = efeDelayCount;                       // Load Timer1 with EFE sample delay count
      digitalWrite(mainSamplePin, syncDemodOff);   // Turn off main sample gate
      if (readDelayPot == false) {                 // Check if read delay pot flag is false
        readDelayCounter++;                        // Increment read delay counter
        if (readDelayCounter >= readDelayLimit) {  // Check if read delay counter has reached limit
          readDelayPot = true;                     // Enable read of delay pot
          readDelayCounter = 0;                    // Clear read delay counter
        }
      }
      intState = 4;
      break;
    case 4:
      TCNT1 = efeSampleCount;                      // Load Timer1 with EFE sample pulse count
      digitalWrite(efeSamplePin, syncDemodOn);     // Turn on EFE sample gate
      intState = 5;
      break;
    case 5:
      TCNT1 = txPeriodCount;                       // Load Timer1 with TX period count
      digitalWrite(efeSamplePin, syncDemodOff);    // Turn off EFE sample gate
      intState = 0;
      break;
    default:
      intState = 0;
      break;
  }
}

void loop() {
  if (readDelayPot == true) {
    delayVal = analogRead(delayPin);                   // Read the delay pot
    mainDelay = defMainDelay + delayVal * clockCycle;  // Offset main sample delay
    calcTimerValues();                                 // Calculate new timer values
    readDelayPot = false;                              // Set read delay pot flag to false
  }
}
