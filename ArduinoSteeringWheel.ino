/*
   SteeringWheel
   Authors: Serggio Pizzella <serggiopizzella@gmail.com>, O.Figaroa
   Class: PCB 06
   Date: 23/04/2021
*/

#include <DHT11.h>
#include <Display.h>
#include <TM1637Display.h>

// I/O pins
const uint8_t BUZZER      = 3;
const uint8_t LED_RED     = 4;
const uint8_t LED_GREEN   = 5;
const uint8_t LED_BLUE    = 6;
const uint8_t LED_YELLOW  = 7;
const uint8_t KEY_1       = 8;
const uint8_t KEY_2       = 9;
const uint8_t KNOB        = 14;
const uint8_t LDR_PIN     = 16;

// Keys variables
int stateKey1       =   1;
int stateKey2       =   1;
int prevStateKey1   =   1; // previous state of Key 1
int prevStateKey2   =   1; // previous state of Key 2

// Other variables
const int DEBOUNCEDELAY = 30;
const int MAXLEFT = -120;  // Steering wheel at left most position
const int MAXRIGHT = 120;  // Steering wheel at right most position
const int INTERVAL_LIGHTS = 500;  // flashing period
const int INTERVAL_TEMP = 5000;  // temperature period

int dayTimeThresHold = 0;     // Day
int nightTimeThresHold = 0;   // Night

enum SteerState {MOSTLEFT = 1, FORWARD = 2, MOSTRIGHT = 3};
SteerState currSS; // Current steer state

// Direction variables
int prevSS; //previous steer state
bool toLeft;
bool toRight;
bool steerStateChanged;

bool hazardOn = false;
bool flashState;
bool stateHeadLights;
bool prevStateHeadLights;

//Time variables
unsigned long debounceTimeKey1 = 0;
unsigned long debounceTimeKey2 = 0;
unsigned long previousTimeLights = 0;
unsigned long previousTimeTemp = 0;
unsigned long currTime = 0;

//Function prototypes
void InitializeIO(void);
void CommandMessages(void);
void UpDateTemp(unsigned long cTime);
void CalibrateNightDayBoundaries(void);
void ControlDirectionLights(unsigned long cTime );
void ControlHeadLights(int cIntensity, unsigned long cTime );

int SteeringState(void);

void setup() {
  InitializeIO();
  CalibrateNightDayBoundaries();
}

void loop() {
  /*
    Statechange steering wheel
  */
  currSS = SteeringState();
  if (currSS != prevSS) {
    steerStateChanged = true;
  } else {
    steerStateChanged = false;
  }
  /*
     State machine on
  */
  switch (currSS) {
    case MOSTLEFT:
      if (steerStateChanged) {
        // one shot execution
        Display.show("_---");
        Serial.println("Turning Left");// message to dashboard
      }
      //Continuous execution
      break;
    case FORWARD:
      if (steerStateChanged) {
        if (prevSS == MOSTRIGHT) {
          toRight = false;
        }
        if (prevSS == MOSTLEFT) {
          toLeft = false;
        }
        Display.show("-__-");
        Serial.println("Going Forward");// message to dashboard
      }
      //Cyclic execution
      break;
    case MOSTRIGHT:
      if (steerStateChanged) {
        // one shot execution
        Display.show("---_");
        Serial.println("Turning Right");// message to dashboard
      }
      //Cyclic execution
      break;
    default:
      //Cyclic execution
      break;
  }
  prevSS = currSS;
  /*
    Handling Key One : direction right
  */
  int readState1 = digitalRead(KEY_1);
  if (readState1 != prevStateKey1) {
    // reset the debouncing timer
    debounceTimeKey1 = millis();
  }
  if ((millis() - debounceTimeKey1) > DEBOUNCEDELAY) {
    if (readState1 != stateKey1) {
      stateKey1 = readState1;
      if (stateKey1 == 0) {
        //toggling toRight
        toRight = !toRight;
        //reset toLeft
        toLeft = false;
        if (toRight) {
          Serial.println("TURN RIGHT SIGN");// message to dashboard
        }
      }
    }
  }
  prevStateKey1 = readState1;
  /*
    HandlingKey Two : direction left
  */
  int readState2 = digitalRead(KEY_2);
  if (readState2 != prevStateKey2) {
    debounceTimeKey2 = millis();
  }
  if ((millis() - debounceTimeKey2) > DEBOUNCEDELAY) {
    if (readState2 != stateKey2) {
      stateKey2 = readState2;
      if (stateKey2 == 0) {
        //toggling toLeft
        toLeft = !toLeft;
        //reset toRight
        toRight = false;
        if (toLeft) {
          Serial.println("TURN LEFT SIGN");// message to dashboard
        }
      }
    }
  }
  prevStateKey2 = readState2;
  /*
    Handling Lights
  */
  CommandMessages(); //get hazard status from dashboard
  
  currTime = millis();
  ControlDirectionLights(currTime);
  ControlHeadLights(analogRead(LDR_PIN), currTime);
  /*
    Handling Temperature
  */
  UpDateTemp(currTime);
}
