#include <Arduino.h>
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

const uint16_t PixelCount = 16; // make sure to set this to the number of pixels in your strip
const uint16_t PixelCount2 = 1; // make sure to set this to the number of pixels in your strip
const uint8_t PixelPin = 6;     // make sure to set this to the correct pin, ignored for Esp8266
const uint8_t PixelPin2 = 5;    // make sure to set this to the correct pin, ignored for Esp8266
const RgbColor CylonEyeColor(HtmlColor(0x7f0000));
const RgbColor HarshWhite(HtmlColor(0xddddff));
const RgbColor ClearPixel(HtmlColor(0x000000));
const int triggerPin = 7;

int buttonState = 0;
unsigned int chargeTime = 0;
const unsigned int maxCharge = 160000;
unsigned int chargeStep = 0;
unsigned int lastPixel = 0;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
// NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> barrel(PixelCount2, PixelPin2);
// for esp8266 omit the pin
//NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount);

void setup()
{

    chargeStep = maxCharge / PixelCount; // should be 2250 loops, ~2secs

    // initialize the pushbutton pin as an input:
    pinMode(triggerPin, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    strip.Begin(); // initialize the strip
    strip.Show();  // make sure it is visible
    // Serial.begin(9600);
}

void loop()
{

    buttonState = digitalRead(triggerPin);

    if (buttonState)
    {
        if (lastPixel > 0)
        {
            strip.ClearTo(HarshWhite);
            strip.Show();
            delay(10*lastPixel);
        }
        chargeTime = 0;
        lastPixel = 0;
        strip.ClearTo(ClearPixel);
        strip.Show();
    }
    else
    {
        chargeTime++;

        int rm = chargeTime % chargeStep;

        if (rm == 0)
        {
            for (unsigned int i = 0; i <= lastPixel; i++)
            {
                strip.SetPixelColor(i, CylonEyeColor);
            }
            lastPixel++;
            strip.Show();
        }
    }
}
