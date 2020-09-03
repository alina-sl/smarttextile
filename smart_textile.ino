#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>
#include <Adafruit_NeoPixel.h>

// pulse sensor constants
const int OUTPUT_TYPE = SERIAL_PLOTTER;
const int PULSE_INPUT = A4;
const int PULSE_BLINK = 13;    // Pin 13 is the on-board LED
const int PULSE_FADE = 5;
const int THRESHOLD = 550;   // Adjust this number to avoid noise when idle

// pixel board constants
const int PIXEL_BOARD_PIN = 10;  //Which pin the pixels are connected to
const int LED_COUNT = 3;  //Number of pixels used

const int VIBRATOR_PIN = 5;

// heart rate constants
const int HEART_RATE_NORMAL = 80;
const int HEART_RATE_ELEVATED = 100;

// create a pulse sensor instance
PulseSensorPlayground pulseSensor;

// create a neo pixel instance
Adafruit_NeoPixel leds = Adafruit_NeoPixel(LED_COUNT, PIXEL_BOARD_PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
  /*
     Use 115200 baud because that's what the Processing Sketch expects to read,
     and because that speed provides about 11 bytes per millisecond.

     If we used a slower baud rate, we'd likely write bytes faster than
     they can be transmitted, which would mess up the timing
     of readSensor() calls, which would make the pulse measurement
     not work properly.
  */
  Serial.begin(115200);

  // setup vibrator pin
  pinMode(VIBRATOR_PIN, OUTPUT);

  // Configure the PulseSensor manager.
  pulseSensor.analogInput(PULSE_INPUT);
  // uncomment this code for blink of onboard led
  //pulseSensor.blinkOnPulse(PULSE_BLINK);
  //pulseSensor.fadeOnPulse(PULSE_FADE);

  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);
  pulseSensor.setThreshold(THRESHOLD);

  leds.begin();  // Start up the LED strip.
  leds.show();   // LEDs don't actually update until you call this.

  // Now that everything is ready, start reading the PulseSensor signal.
  if (!pulseSensor.begin()) {
    /*
       PulseSensor initialization failed,
       likely because our particular Arduino platform interrupts
       aren't supported yet.

       If your Sketch hangs here, try PulseSensor_BPM_Alternative.ino,
       which doesn't use interrupts.
    */
    for(;;) {
      // Flash the led to show things didn't work.
      digitalWrite(PULSE_BLINK, LOW);
      delay(50);
      digitalWrite(PULSE_BLINK, HIGH);
      delay(50);
    }
  }
}

void loop()
{
  int bpm = pulseSensor.getBeatsPerMinute();

  // write the latest sample to Serial.
  pulseSensor.outputSample();

  /*
     If a beat has happened since we last checked,
     write the per-beat information to Serial.
   */
  if (pulseSensor.sawStartOfBeat()) {
    pulseSensor.outputBeat();

    if (bpm <= HEART_RATE_NORMAL) {
      blinkPixelBoard(20, 255, 0, 255, 0); 
    }
    if (bpm > HEART_RATE_NORMAL && bpm <= HEART_RATE_ELEVATED) {
      blinkPixelBoard(20, 255, 255, 128, 0);
    }
    if (bpm > HEART_RATE_ELEVATED) {
      blinkPixelBoard(20, 255, 255, 0, 0);
      // turn on & off vibrator
      digitalWrite(VIBRATOR_PIN, HIGH);
      delay(200);
      digitalWrite(VIBRATOR_PIN, LOW);
    }
  }


}


void blinkPixelBoard(int fadeDelay, int brightness, int red, int green, int blue) {
  leds.setBrightness(brightness);
  leds.show();
  delay(fadeDelay);
  leds.setPixelColor(0, red, green, blue);
  leds.show();
  delay(fadeDelay);
  leds.setBrightness(0);
  leds.show();
}
