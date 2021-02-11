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

const unsigned long maxCharge = 160000;
const unsigned long idleTimeout = 16000;
unsigned int chargeStep = 0;
unsigned int lastPixelCharged = 0;
int buttonState = 0;
unsigned int chargeTime = 0;
unsigned long idleTime = 0;

int measurementValue = 0;
float voltage;
unsigned int VoltageSampleTime = 0;
unsigned int VoltageSampleFrequency = 0;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
// NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> barrel(PixelCount2, PixelPin2);

NeoPixelAnimator animations(2); // only ever need 2 animations

uint16_t lastPixel = 0; // track the eye position
int8_t moveDir = 1;     // track the direction of movement

// uncomment one of the lines below to see the effects of
// changing the ease function on the movement animation
AnimEaseFunction moveEase =
    NeoEase::QuarticInOut;

void FadeAll(uint8_t darkenBy)
{
    RgbColor color;
    for (uint16_t indexPixel = 0; indexPixel < strip.PixelCount(); indexPixel++)
    {
        color = strip.GetPixelColor(indexPixel);
        color.Darken(darkenBy);
        strip.SetPixelColor(indexPixel, color);
    }
}

void FadeAnimUpdate(const AnimationParam &param)
{
    if (param.state == AnimationState_Completed)
    {
        FadeAll(10);
        animations.RestartAnimation(param.index);
    }
}

void MoveAnimUpdate(const AnimationParam &param)
{
    // apply the movement animation curve
    float progress = moveEase(param.progress);

    // use the curved progress to calculate the pixel to effect
    uint16_t nextPixel;
    if (moveDir > 0)
    {
        nextPixel = progress * PixelCount;
    }
    else
    {
        nextPixel = (1.0f - progress) * PixelCount;
    }

    // if progress moves fast enough, we may move more than
    // one pixel, so we update all between the calculated and
    // the last
    if (lastPixel != nextPixel)
    {
        for (uint16_t i = lastPixel + moveDir; i != nextPixel; i += moveDir)
        {
            strip.SetPixelColor(i, CylonEyeColor);
        }
    }
    strip.SetPixelColor(nextPixel, CylonEyeColor);

    lastPixel = nextPixel;

    if (param.state == AnimationState_Completed)
    {
        // reverse direction of movement
        moveDir *= -1;

        // done, time to restart this position tracking animation/timer
        animations.RestartAnimation(param.index);
    }
}

void SetupAnimations()
{
    // fade all pixels providing a tail that is longer the faster
    // the pixel moves.
    animations.StartAnimation(0, 5, FadeAnimUpdate);

    // take several seconds to move eye fron one side to the other
    animations.StartAnimation(1, 2000, MoveAnimUpdate);
}

void StopAnimations()
{
    if (animations.IsAnimating())
    {
        animations.StopAll();
        strip.ClearTo(ClearPixel);
        strip.Show();
    }
}

void CheckVoltage()
{
    VoltageSampleTime++;

    if (VoltageSampleTime >= VoltageSampleFrequency)
    {
        measurementValue = analogRead(A0);
        voltage = measurementValue * 5.0 / 1023;
        VoltageSampleTime = 0;
    }
}

void setup()
{

    chargeStep = maxCharge / PixelCount; // should be 2250 loops, ~2secs
    VoltageSampleFrequency = chargeStep * PixelCount + 1;

    // initialize the pushbutton pin as an input:
    pinMode(triggerPin, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    strip.Begin(); // initialize the strip
    strip.Show();  // make sure it is visible
    // Serial.begin(9600);
    measurementValue = analogRead(A0);
    voltage = measurementValue * 5.0 / 1023;
}

void loop()
{
    CheckVoltage();
    if (voltage > 4)
    {

        buttonState = digitalRead(triggerPin);
        if (buttonState)
        { // button is not pressed
            if (lastPixelCharged > 0)
            { // if we had it charged, discharge
                strip.ClearTo(HarshWhite);
                strip.Show();
                delay(10 * lastPixelCharged);
            }

            // reset counters, clear all the pixels
            chargeTime = 0;
            lastPixelCharged = 0;
            strip.ClearTo(ClearPixel);
            strip.Show();

            // deciding to start the idle animation or not
            if (idleTime >= idleTimeout)
            {

                if (animations.IsAnimating())
                { // if animation is in progress, update it
                    animations.UpdateAnimations();
                    strip.Show();
                }
                else
                { // if not in progress, start it
                    SetupAnimations();
                }
            }
            else
            { // being sure to stop all the animations for the idle time
                StopAnimations();
                idleTime++;
            }
        }
        else
        { // trigger button is pressed
            chargeTime++;
            idleTime = 0;     // we're not idle, we're charging
            StopAnimations(); // being sure to clear all animations

            if (chargeTime % chargeStep == 0)
            { // deciding if we can update the charge level
                for (unsigned int i = 0; i <= lastPixelCharged; i++)
                { // lights up to the charged point
                    strip.SetPixelColor(i, CylonEyeColor);
                }
                lastPixelCharged++;
                strip.Show();
            }
        }
    }
}
