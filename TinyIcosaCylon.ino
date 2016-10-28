
#include <EEPROM.h>
//#include <avr/interrupt.h>
//#include <avr/power.h>
#include <avr/sleep.h>
//#include <avr/io.h>

// Utility macros
#define adc_disable() (ADCSRA &= ~(1<<ADEN)) // disable ADC (before power-off)
#define adc_enable()  (ADCSRA |=  (1<<ADEN)) // re-enable ADC

// const static byte high_pins [] = {0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4};
// const static byte low_pins  [] = {1,2,3,4,0,2,3,4,0,1,3,4,0,1,2,4,0,1,2,3};

const static byte high_pins [] = {0,1,1,2,2,3,3,4,4,0,0,2,2,4,4,1,1,3,3,0};
const static byte low_pins  [] = {1,0,2,1,3,2,4,3,0,4,2,0,4,2,1,4,3,1,0,3};

const static byte led_count = sizeof(high_pins) / sizeof(high_pins[0]);

void pulse(byte led, int pause) {
  led = led % led_count;
  byte pin1 = high_pins[led];
  byte pin2 = low_pins[led];
  digitalWrite(pin1,HIGH);
  digitalWrite(pin2,LOW);
  pinMode(pin1,OUTPUT);
  pinMode(pin2,OUTPUT);
  delayMicroseconds(pause);
  pinMode(pin1,INPUT);
  pinMode(pin2,INPUT);
  digitalWrite(pin1,LOW);
  digitalWrite(pin2,LOW);
}

void randomBlink(int on, int off) {
  byte pin = random(led_count);
  pulse(pin,on);
  delay(off);
}

void cylon(int on, int off) {
  static char pin = 0;
  static char dir = 1;
  pulse(pin, on);
  pin += dir;
  if (pin <= 0) {
    pin = 0;
    dir = 1;
  }
  if (pin >= led_count) {
    pin = led_count - 1;
    dir = -1;
  }
  delay(off);
}

void rotate(int d, int on, int off) {
  static int pin = 0;
  pulse(pin,on);
  pin += d;
  if (pin < 0) pin = led_count - 1;
  if (pin >= led_count) pin = 0;
  delay(off);
}

void fade(int pin) {
  for(int on = 1; on < 1000; ++on) {
    int off = 2000 - on;
    pulse(pin,on);
    delayMicroseconds(off);
  }
  for(int on = 1000; on > 0; --on) {
    int off = 2000 - on;
    pulse(pin,on);
    delayMicroseconds(off);
  }
}

void randomFade() {
  byte pin = random(led_count);
  fade(pin);
}

void multiFade(int pos) {
  pos += 100;
  for(int p = pos - 50; p < pos + 50; p += 5) {
    int o = (60 - abs(pos - p));
    pulse(p / 5, o * o / 4);
  }
}

void rotateFade() {
  static int c = 50;
  multiFade(c);
  c = (c + 1) % 100;
}

void breathe() {
  static int d = 5;
  static int dir = 1;
  for(int pin = 0; pin < 20; ++pin) pulse(pin, d);
  delayMicroseconds((250 * 20) - (d * 20) + 1);
  if (dir > 0) {
     if ( d < 250 ) ++d;
     else {
       dir = -1;
       --d;
     }
  } else {
    if ( d > 1 ) --d;
    else {
      dir = 1;
      ++d;
    }
  }
}

void sleepNow() {
  DDRB  = B000000; // set all 5 IO to INPUT
  PORTB = B100000; // reset pullup enabled, all other output LOW
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Choose our preferred sleep mode:
  sleep_enable();                      // Set sleep enable (SE) bit:
  sleep_mode();                        // Put the device to sleep:
  sleep_disable();                     // Upon waking up, sketch continues from this point.
}

void twinkle() {
    randomBlink(3000,20); 
}
    
void scanRight() {
  rotate(-1,8000,20);
}
    
void scanLeft() {
  rotate(1,8000,20);
}
    
void cylonMode() {
  cylon(8000, 20);
}
    
void allOn() {
  rotate(1,500,0);
}


typedef void(*ModePtr)();
ModePtr ModePtrs[] = {
  sleepNow, 
  cylonMode, 
  scanLeft, 
  scanRight, 
  twinkle, 
  allOn, 
  // randomFade, 
  rotateFade, 
  breathe,
};
const static byte mode_count = sizeof(ModePtrs) / sizeof(ModePtrs[0]);
static byte mode = 0;

void setup() {
  adc_disable();
  // we are using the reset pin to change modes
  if (bit_is_set(MCUSR, EXTRF)) { // reset button
    mode = EEPROM.read(1) % mode_count; // retrieve mode from EEPROM 
    EEPROM.write(1, (mode + 1) % mode_count); // store the next mode into the EEPROM
  } else {
    mode = 0;
    EEPROM.write(1, 1); // store the next mode into the EEPROM
  }
}

void loop() {
  ModePtrs[mode]();
}


