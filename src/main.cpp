#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include <avr/sleep.h>

#pragma region #region : CBI &SBI

#ifndef cbi
// Clear bits
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
// Set bits
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#pragma endregion

int pinNeo = 1;
int pinBlink = 0;
int pinButton = 2;
int pinPower = 4;

int leds = 1;
byte i = 1;
Adafruit_NeoPixel mx = Adafruit_NeoPixel(leds, 1, NEO_RGB + NEO_KHZ800);
bool pressed;
bool mode;
unsigned long t, p;

uint32_t Wheel(byte WheelPos);
bool system_sleep();

void setup()
{
  pinMode(pinBlink, OUTPUT);
  pinMode(pinNeo, OUTPUT);
  pinMode(pinButton, INPUT_PULLUP);
  pinMode(pinPower, OUTPUT);

  digitalWrite(pinPower, HIGH);

  // http://thewanderingengineer.com/2014/08/11/pin-change-interrupts-on-attiny85/
  sbi(GIMSK, PCIE); // Turn on Pin Change interrupt
  sbi(PCMSK, PCINT2);

  mx.begin();
  mx.setBrightness(32);
  mx.All(0);
  mx.Show(0);

  //blink();blink();blink();
  t = millis();
}

byte c = 0;
uint32_t colors[] = {
    Adafruit_NeoPixel::Rgb(255, 255, 255),
    Adafruit_NeoPixel::Rgb(240, 255, 64),
    Adafruit_NeoPixel::Rgb(0, 152, 255),
    Adafruit_NeoPixel::Rgb(255, 0, 0),
    Adafruit_NeoPixel::Rgb(0, 255, 0),
    Adafruit_NeoPixel::Rgb(0, 0, 255),
};

uint32_t color = colors[0];

void loop()
{
  bool justwoke = system_sleep();

  int btn = digitalRead(pinButton);

  if (!pressed && btn == LOW)
  {
    pressed = true;
    p = millis();
    if (!justwoke && mode)
    {
      c = (c + 1) % (sizeof(colors) / sizeof(uint32_t));
      color = colors[c];
    }
  }
  else if (pressed && btn == HIGH)
  {
    pressed = false;
  }

  if (pressed && millis() - p > 2000)
  {
    //blink();blink();
    p = millis();
    mode = !mode;
  }

  if (mode)
  {
    mx.All(color);
    mx.Show(100);
  }
  else
  {
    mx.All(Wheel(i));
    mx.Show(50);
    i += 2;
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
  if (WheelPos < 85)
  {
    return mx.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  else if (WheelPos < 170)
  {
    WheelPos -= 85;
    return mx.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  else
  {
    WheelPos -= 170;
    return mx.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void blink()
{
  digitalWrite(0, HIGH); // turn the LED on (HIGH is the voltage level)
  delay(100);            // wait for a second
  digitalWrite(0, LOW);  // turn the LED off by making the voltage LOW
  delay(100);            // wait for a second
}

// http://interface.khm.de/index.php/lab/experiments/sleep_watchdog_battery/
bool system_sleep()
{
  if (millis() - t < 10000)
    return false;

  mx.All(0);
  mx.Show(0);
  //blink(); blink();

  digitalWrite(pinPower, LOW);
  pinMode(pinNeo, INPUT);

  cbi(ADCSRA, ADEN);                   // Switch Analog to Digital converter OFF
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Set sleep mode
  sleep_mode();                        // System sleeps here

  pinMode(pinNeo, OUTPUT);
  digitalWrite(pinPower, HIGH);

  sbi(ADCSRA, ADEN); // Switch Analog to Digital converter ON
  return true;
}

ISR(PCINT0_vect)
{
  t = millis();
}
