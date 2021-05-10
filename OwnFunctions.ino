/*
   Initialize
*/
void InitializeIO(void) {
  pinMode(LDR_PIN, INPUT);
  pinMode(KNOB, INPUT);
  pinMode(KEY_1, INPUT_PULLUP);
  pinMode(KEY_2, INPUT_PULLUP);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  ClearLeds();
  Display.clear();
  Serial.begin(9600);
}
/*
  Calibrate daylight and at night
*/
void CalibrateNightDayBoundaries(void) {
  Display.show("----");
  Serial.println("Calibrating the uncovered LDR: by day.");// message to dashboard
  delay(100);
  Serial.println("Press any key to start...");
  while ((digitalRead(KEY_1)) && (digitalRead(KEY_2))) {
    // wait for key pressed
  }
  // Average of 10 measurements..
  for (int i = 0; i < 10; i++) {
    dayTimeThresHold = dayTimeThresHold + analogRead(LDR_PIN);
    delay(200);
  }
  dayTimeThresHold = (dayTimeThresHold / 10) * 0.9;
  Display.show(dayTimeThresHold);
  Serial.println("Done.");// message to dashboard
  delay(1000);
  Serial.println("Calibrating the covered LDR: at night.");// message to dashboard
  delay(100);
  Serial.println("Press any key to start...");
  while ((digitalRead(KEY_1)) && (digitalRead(KEY_2))) {
    // wait for key pressed
  }
  Display.show("----");
  // Average of 10 measurements..
  for (int i = 0; i < 10; i++) {
    nightTimeThresHold = nightTimeThresHold + analogRead(LDR_PIN);
    delay(200);
  }
  nightTimeThresHold = (nightTimeThresHold / 10) * 1.1;
  Display.show(nightTimeThresHold);
  delay(1000);
  Display.show("----");
  Serial.println("Done.");// message to dashboard
  delay(1000);
  Serial.println("clear");// message to dashboard
  delay(100);
  Serial.println("ENGINE STARTED");// message to dashboard
}

/*
   Steering state
   state 1, is going to the left
   state 2, is going forward
   state 3, is going to the right
*/
int SteeringState(void) {
  int steer = map(analogRead(KNOB), 0, 1023, MAXLEFT, MAXRIGHT);
  int state;
  if ((steer > -15) && (steer < 15)) {
    state = 2;
  } else if (steer < -20) {
    state = 1;
  } else if (steer > 20) {
    state = 3;
  }
  return state;
}
/*
   Update temperature.
   updates temperature every set interval
*/
void UpDateTemp(unsigned long cTime) {
  if (cTime - previousTimeTemp >= INTERVAL_TEMP) {
    // save the last time you updated the temp
    previousTimeTemp = cTime;
    // Update temp
    float temperature = DHT11.getTemperature();
    Serial.println("T:" + String(temperature)); // message to DashBoard
  }
}
/*
   Reset all LEDS connected to pin nr 4 up to 7
*/
void ClearLeds(void) {
  for (int i = 4; i < 8; i++) {
    digitalWrite(i, LOW);
  }
}
/*
   Automatic Control Direction lights and Hazard lights
   flashstate frequency of 1 Hz  = 1 sec = 2*INTERVAL_LIGHTS
*/
void ControlDirectionLights(unsigned long cTime ) {
  if (cTime - previousTimeLights >= INTERVAL_LIGHTS) {
    // save the last time you blinked the LED
    previousTimeLights = cTime;
    // Toggle flashState
    flashState = !flashState;
    // set the LED with the ledState of the variable:
    digitalWrite(LED_YELLOW, (flashState && (toLeft || hazardOn)));
    digitalWrite(LED_BLUE, (flashState && (toRight || hazardOn)));
    digitalWrite(LED_RED, (flashState && hazardOn));
  }
}
/*
   Control the HeadLights.
   Send log to dashboard if light changes
*/
void ControlHeadLights(int cIntensity, unsigned long cTime ) {
  if (cIntensity >= dayTimeThresHold) {
    stateHeadLights = true;
  }
  if (cIntensity <= nightTimeThresHold) {
    stateHeadLights = false;
  }
  if (stateHeadLights != prevStateHeadLights) {
    if (stateHeadLights) {
      digitalWrite(LED_GREEN, LOW);
      Serial.println("H:off"); // message to DashBoard
    }
    else {
      digitalWrite(LED_GREEN, HIGH);
      Serial.println("H:on"); // message to DashBoard
    }
  }
  delay(80); //prevent bouncing of light sensor
  prevStateHeadLights = stateHeadLights;
}
/*
   Arduino <== C# communitation via Serial USB
*/
void CommandMessages(void) {
  bool comm_er = false;
  if (Serial.available()) {
    String comm = Serial.readStringUntil('\n');
    comm.toUpperCase();
    if (comm == "HAZARDOFF") {
      hazardOn = false;
    } else if (comm == "HAZARDON") {
      hazardOn = true;
    }  else {
      comm_er = true;
    }
    // unknown command
    if (comm_er) {
      Serial.println("command ERROR: " + String(comm) + ".");// message to dashboard
    } else if (hazardOn) {
      Serial.println("HAZARD ON");// message to dashboard
    } else {
      Serial.println("HAZARD OFF");// message to dashboard
    }
  }
}
