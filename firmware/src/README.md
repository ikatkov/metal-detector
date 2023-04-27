The ATtiny85 has an internal oscilator that runs at 8MHz, this clock is divided by 8 (`CKDIV8`) so by default ATtiny85 runs at 1MHz.
You can divide by 1 to make ATtiny85 run at 8MHz, you can also use internal PLL (unique to ATTiny85) to make it run on 16MHz
You can't make it run faster without an exteral oscilator.
Using external oscilator will consume two pins. 

# The 8MHz Internal Clock Configuration
8MHz operation is achieved with fuse settings for:

* internal 8 MHz RC oscillator (CKSEL=0010)
* no prescaler (CKDIV8=0)

# Checking The Existing Configuration
The ATtiny85 can use an external clock, but by default it uses an internal oscillator. The internal oscillator runs at 8 MHz, prescaled to 1 MHz by default.

The clock settings are in the fuses. 

```
avrdude -p attiny85  -b 19200 -cusbtiny -U lfuse:r:-:i

avrdude: AVR device initialized and ready to accept instructions

Reading | ################################################## | 100% 0.01s

avrdude: Device signature = 0x1e930b (probably t85)
avrdude: reading lfuse memory:

Reading | ################################################## | 100% 0.00s

avrdude: writing output file "<stdout>"
:01000000629D
:00000001FF

avrdude done.  Thank you.
```

Use http://www.engbedded.com/fusecalc to confirm that E:FF, H:DF, L:62 are factory defaults: 8 MHz internal oscillator with CKDIV8 prescaler: so it is running at 1 MHz.

# Configure For 8MHz
```
avrdude -p attiny85  -b 19200 -cusbtiny -U lfuse:w:0xe2:m
```

# Configure For 16MHz
```
avrdude -p attiny85  -b 19200 -cusbtiny -U lfuse:w:0xf1:m
```

Upload the following and see if it blinks once per second. One second ON and one second OFF.
```
#include "Arduino.h"

void setup()
{
    pinMode(PB4, OUTPUT);
    digitalWrite(PB4, LOW);
}

void loop()
{
    digitalWrite(PB4, HIGH);
    delay(1000);
    digitalWrite(PB4, LOW);
    delay(1000);
}
```


# Restore to Factory Defaults (if needed)
```
avrdude -p attiny85  -b 19200 -cusbtiny -U efuse:w:0xff:m -U hfuse:w:0xdf:m -U lfuse:w:0x62:m
```