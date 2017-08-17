
#include <SoftwareSerial.h>


/* Dell r610 Chassis Monitor
 * Author: Jimb0 "Tails" Hon1nbo
 * Platform: Arduino Nano
 * 
 * This monitor outputs various sensors over serial to see if the system is being tampered with.
 * The chassis sensor is a simple on/off with appropriate debounce
 * The tilt & acceleration sensors will alarm once, then remain in that state until \
 * they have rested for a predtermined period of time.
 * 
 */

  int tiltPin = 9;
  int tiltPinState = 0;
  int lastTiltPinState = 0;
  // we have to use unsigned longs for debounce since millis will quickly exceed the int and double space on Arduinos
  unsigned long tiltLastDebounce = 0;  
  unsigned long tiltDebounceDelay = 100;
  bool tiltDebounce = false;
  unsigned long tiltWaitTimer = 0;
  unsigned long tiltWaitDelay = 3000; // we want the tilt sensor to rest for 2 minutes to consider it "stable"
  bool tiltWait = false;
  
  int chassisPin = 5;
  int chassisPinState = 0;
  int lastChassisPinState = 0;
  unsigned long chassisLastDebounce = 0;
  unsigned long chassisDebounceDelay = 50;
  bool chassisDebounce = false;


void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port
  }

  pinMode(tiltPin, INPUT);
  pinMode(chassisPin, INPUT);
  
  // Ensure we have known states for current pin values
  tiltPinState = digitalRead(tiltPin);
  lastTiltPinState = tiltPinState;
  chassisPinState = digitalRead(chassisPin);
  lastChassisPinState = chassisPinState;

  Serial.println("Monitor Up!");
}

void loop() { 

    // read the pins
    tiltPinState = digitalRead(tiltPin);
    chassisPinState = digitalRead(chassisPin);

    // if the chassis state changes, and we are not in the debounce period, proceedd to raise notice
    if ( (chassisDebounce == false) && (chassisPinState != lastChassisPinState) ) {
        chassisLastDebounce = millis();
        chassisDebounce = true;
        
        if (chassisPinState == LOW) {
           Serial.write("Chassis Event Detected!\n");
        }
        if (chassisPinState == HIGH) {
           Serial.write("Chassis State Restored.\n");
        }
    }

    // if we are not in the debounce period, proceed to check status
    // we do not care if the state changed; we want to know if tilt switch still alarms to reset rest timer.
    if ( (tiltDebounce == false) ) { 
        tiltLastDebounce = millis();
        tiltDebounce = true;

        // if pin is LOW, check if we are in the rest time window.
        // if not in the rest window, raise an alert.
        // if we are already in the rest wait period, don't bother making a duplicate log
        if (tiltPinState == LOW) {
           if (tiltWait == false) {
               Serial.write("Tilt Event Detected!\n");
           }
           
           tiltWait = true; // always going to be set when tilt pin state is low
           // if the tilt sensor is still triggered, restart the rest wait period timer.
           tiltWaitTimer = millis();
        }
    }

    // check if we have passed safe debounce time for the various components
    
    if ((millis() - tiltLastDebounce) > tiltDebounceDelay) {
        tiltDebounce = false;
        lastTiltPinState = tiltPinState;
    }


    if ((millis() - chassisLastDebounce) > chassisDebounceDelay) {
        chassisDebounce = false;
        lastChassisPinState = chassisPinState;
    }

    // check if we have made it through the rest time frame successully
    // if we have, then place notice in the log that the tilt sensor has stabilized
    if ( ((millis() - tiltWaitTimer) > tiltWaitDelay) && (tiltWait == true) ) {
        tiltWait = false;
        Serial.write("Tilt Level Restored.\n");
    }

    // if millis() has rolled over, it will be apparent if it is smaller than any value we have initialized.
    // if that happens, we want to set our timers back to zero. Small scall debounces fail in small cases,
    // but this fails Secure in making alerts more prevalent if it happens during a state change.

    if (millis() < tiltLastDebounce)
        tiltLastDebounce = 0;
    if (millis() < chassisLastDebounce)
        chassisLastDebounce = 0;
    if (millis() < tiltWaitTimer)
        tiltWaitTimer = 0;
}
