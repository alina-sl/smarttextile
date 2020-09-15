// code import and arduino settings
#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>
#include <Adafruit_NeoPixel.h>

// pulse sensor constants
const int OUTPUT_TYPE = SERIAL_PLOTTER;
const int PULSE_INPUT = A4;    // Input pin pulse sensor
const int PULSE_BLINK = 13;    // Pin 13 is the on-board LED; used for error signal
const int THRESHOLD = 550;   // Adjust this number to avoid noise when idle

// pixel board constants
const int PIXEL_BOARD_PIN = 10;  // Which pin the pixels are connected to
const int LED_COUNT = 3;  //Number of pixels used

// vibe board constants
const int VIBRATOR_PIN = 5; // Pin on which the vibe board +pole is connected

// heart rate constants
const int HEART_RATE_NORMAL = 80;   // Heart rate threshold for normal level, up to this level the LED blinks green
const int HEART_RATE_ELEVATED = 100;    // Heart rate threshold for elevated level, up to this level the LED blinks yellow, above it the LED blinks red

// create a pulse sensor instance
PulseSensorPlayground pulseSensor;    // This is used to read the pulse

// create a neo pixel instance
Adafruit_NeoPixel leds = Adafruit_NeoPixel(LED_COUNT, PIXEL_BOARD_PIN, NEO_GRB + NEO_KHZ800); // This is used to flash the Pixel Board

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
  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);
  pulseSensor.setThreshold(THRESHOLD);

  leds.begin();  // Start up the Pixel Board
  leds.show();   // LEDs don't actually update until you call this

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
  // create variable to save the heart rate
  int bpm = pulseSensor.getBeatsPerMinute();

  // write the latest sample to serial display
  pulseSensor.outputSample();

// If a beat has happened since we last checked, we blink the LED
  if (pulseSensor.sawStartOfBeat()) {
    pulseSensor.outputBeat(); // Output to Serial Plotter

    if (bpm <= HEART_RATE_NORMAL) {
      blinkPixelBoard(20, 255, 0, 255, 0);  // Blink green
    }
    if (bpm > HEART_RATE_NORMAL && bpm <= HEART_RATE_ELEVATED) {
      blinkPixelBoard(20, 255, 255, 128, 0);  // Blink yellow
    }
    if (bpm > HEART_RATE_ELEVATED) {
      blinkPixelBoard(20, 255, 255, 0, 0);    // Blink red
      // turn on & off vibe board
      digitalWrite(VIBRATOR_PIN, HIGH);
      delay(200); // Turn on for 200 miliseconds 
      digitalWrite(VIBRATOR_PIN, LOW);
    }
  }


}

// Function to blink the Pixel Board
// Parameters:
// fadeDelay -> time to fade in and fade out the LED for smooth effect
// brightness -> overall brightness
// red -> intensity of red LED (between 0 (min) and 255 (max))
// green -> instensity of green LED (between 0 (min) and 255 (max))
// blue -> intensity of blue LED (between 0 (min) and 255 (max))
// => mix intensities to create colors, e.g Yellow (255 red, 128 green, 0 blue)
void blinkPixelBoard(int fadeDelay, int brightness, int red, int green, int blue) {
  leds.setBrightness(brightness);   // set overall brightness
  leds.show();  // update LED to turn on LED
  delay(fadeDelay);   // delay for smoother LED effect
  leds.setPixelColor(0, red, green, blue);  // set color
  leds.show();  // update LED to set color
  delay(fadeDelay);   // delay for smoother LED effect
  leds.setBrightness(0);  // turn LED off again
  leds.show();  // update LED to turn off LED
}
