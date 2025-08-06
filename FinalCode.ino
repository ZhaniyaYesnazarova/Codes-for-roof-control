#include<HX711_ADC.h>
#include <elapsedMillis.h>
#include "AccelStepper.h"

elapsedMillis timeElapsed;

int a = 250;
bool process = true;
bool centered = false;


//  digitalWrite(dirPin, HIGH); //forward //second one
//  digitalWrite(dirPin2, LOW); // first one // x axis low forwars

const int buttonpin = A3;
const int tarebutton = A1;

#define liftdir A5
#define liftstep A4
#define motorInterfaceType 1

AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);

int buttonState = HIGH; 
int buttonState2 = HIGH;
int lastButtonState = HIGH;
int lastButtonState2 = HIGH;
// const unsigned long debounceDelay = 1000; 
// unsigned long lastDebounceTime = 0;

unsigned long currTime = 0;

//linear actuator pins
const int RPWM1 = 13;    //these are from ibt 2
const int LPWM1 = 12;
const int sensorPin1 = A0; // blue feedback wire

const int RPWM2 = 11;    
const int LPWM2 = 10;
const int sensorPin2 = A2;

int sensorVal1, sensorVal2;

float ext1, ext2;

const int Speed = 255;
const float strokeLength = 101.6; 

int max1, min1;
int max2, min2;

//stepper motor pins - centering platform
const int stepPin = 9;    
const int dirPin = 8;   
const int stepPin2 = 7;   // x axis 
const int dirPin2 = 6;   

// motor with rainbow cables need 5000 rotations and 
// motor only with red cables need 

const int y = 4950;  
const int x = 4500;  
// HX711 wiring
// the pins are already soldered to perfboard
const int HX711_dout_1 = 3;
const int HX711_sck_1  = 2;
//second one  short wires load cell 
const int HX711_dout_2 = 5;
const int HX711_sck_2  = 4;

HX711_ADC LoadCell_1(HX711_dout_1, HX711_sck_1);
HX711_ADC LoadCell_2(HX711_dout_2, HX711_sck_2);

long t;

bool landed = false; 
bool lidIsOpen = false;

void setup() {
  Serial.begin(57600);

  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);

  pinMode(buttonpin, INPUT_PULLUP);
  pinMode(tarebutton, INPUT_PULLUP);
  pinMode(RPWM1, OUTPUT);
  pinMode(LPWM1, OUTPUT);
  pinMode(sensorPin1, INPUT);

  pinMode(RPWM2, OUTPUT);
  pinMode(LPWM2, OUTPUT);
  pinMode(sensorPin2, INPUT);

  Serial.println("Calibrating actuator 1...");
  max1 = moveToLimit1(1);   
  delay(1000);
  min1 = moveToLimit1(-1); 
  delay(1000);

  Serial.println("Calibrating actuator 2...");
  max2 = moveToLimit2(1);   
  delay(1000);
  min2 = moveToLimit2(-1);  
  delay(1000);

  Serial.println("Calibration complete.");

  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  pinMode(dirPin2, OUTPUT);

  float calibrationValue_1; // calibration value load cell 1
  float calibrationValue_2; // calibration value load cell 2

  calibrationValue_1 = 105.0; // uncomment this if you want to set this value in the sketch
  calibrationValue_2 = 110.0; // uncomment this if you want to set this value in the sketch

  
  LoadCell_1.begin();
  LoadCell_2.begin();
  unsigned long stabilizingtime = 2000; // tare preciscion can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  byte loadcell_1_rdy = 0;
  byte loadcell_2_rdy = 0;
  while ((loadcell_1_rdy + loadcell_2_rdy) < 2) { //run startup, stabilization and tare, both modules simultaniously
    if (!loadcell_1_rdy) loadcell_1_rdy = LoadCell_1.startMultiple(stabilizingtime, _tare);
    if (!loadcell_2_rdy) loadcell_2_rdy = LoadCell_2.startMultiple(stabilizingtime, _tare);
  }
  if (LoadCell_1.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 no.1 wiring and pin designations");
  }
  if (LoadCell_2.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 no.2 wiring and pin designations");
  }
  LoadCell_1.setCalFactor(calibrationValue_1); 
  LoadCell_2.setCalFactor(calibrationValue_2); 
  Serial.println("Startup is complete");

}

void loop() {

  int reading = digitalRead(buttonpin);
  int reading2 = digitalRead(tarebutton);
  //Serial.print("DEBUG BUTTON: ");
  //Serial.println(reading);

  if (reading != lastButtonState) {
    //Serial.print("DEBUG BUTTON CHANGING STATE: ");
    //Serial.println(reading);

    if (reading == LOW) {
      process = false;  
      //Serial.print("DEBUG BUTTON CHANGING STATE LID STATE 1 closed 0 open: ");
      //Serial.println(lidIsOpen);
    }
  }

  lastButtonState = reading;


  if(reading2 != lastButtonState2){
    if(reading2 == LOW){
      LoadCell_1.tareNoDelay();
      LoadCell_2.tareNoDelay();
    }
  }

  lastButtonState2 = reading2;


  if (!lidIsOpen && !process) {
      Serial.println("Extending to 100 mm...");
      driveActuator(1, 1, Speed);
      process = true;
      lidIsOpen = true;
      unsigned long startTime = millis();

      bool actuator2Started = false;
      do {
        sensorVal1 = analogRead(sensorPin1);
        ext1 = mapfloat(sensorVal1, float(min1), float(max1), 0.0, strokeLength);
        
        if (!actuator2Started && millis() - startTime >= 1000) {
          driveActuator(1, 2, Speed);
          actuator2Started = true;
        }

        if(ext1 > 70){
          driveActuator(0, 1, Speed);
        }

        if (actuator2Started) {
          sensorVal2 = analogRead(sensorPin2);
          ext2 = mapfloat(sensorVal2, float(min2), float(max2), 0.0, strokeLength);
        }

        displayOutput();


      } while ( ext2 < 70);

      driveActuator(0, 1, Speed);
      driveActuator(0, 2, Speed);
      Serial.println("Target reached: 100 mm");

  } 
  
  else if (lidIsOpen == true && !process ) {
      Serial.println("Retracting to 0 mm...");
      process = true;
      lidIsOpen = false;
      driveActuator(-1, 1, Speed);
      unsigned long startTime = millis();
      unsigned long closetime = millis();
      bool actuator2Started = false;
      do {
        sensorVal1 = analogRead(sensorPin1);
        ext1 = mapfloat(sensorVal1, float(min1), float(max1), 0.0, strokeLength);

        if (!actuator2Started && millis() - startTime >= 2000) {
          driveActuator(-1, 2, Speed);
          actuator2Started = true;
        }

        if (actuator2Started) {
          sensorVal2 = analogRead(sensorPin2);
          ext2 = mapfloat(sensorVal2, float(min2), float(max2), 0.0, strokeLength);
        }

        displayOutput();

      } while ( ext2 > 0.06 || (millis()-closetime) < 6000);

      driveActuator(0, 1, Speed);
      driveActuator(0, 2, Speed);
      Serial.println("Fully retracted.");
  } 



  static boolean newdata = 0;
  float first;
  float second;

  // check for new data/start next conversion:
  if (LoadCell_1.update()) newdata = true;
  LoadCell_2.update();

  //get smoothed value from data set
  if ((newdata)) {
      first = LoadCell_1.getData();
      second = LoadCell_2.getData();
      Serial.print("Load_cell 1 output val: ");
      Serial.print(first);
      Serial.print("    Load_cell 2 output val: ");
      Serial.println(second);
      newdata = 0;  
  }

  // receive command from serial terminal, send 't' to initiate tare operation:
  // if (Serial.available() > 0) {
  //   char inByte = Serial.read();
  //   if (inByte == 't') {
  //     LoadCell_1.tareNoDelay();
  //     LoadCell_2.tareNoDelay();
  //   }
  // }

  //check if last tare operation is complete
  if (LoadCell_1.getTareStatus() == true) {
    Serial.println("Tare load cell 1 complete");
  }
  if (LoadCell_2.getTareStatus() == true) {
    Serial.println("Tare load cell 2 complete");
  }

  

  if (!landed && (first > 5 || second > 5) ) {  
    landed = true; 
    Serial.println("Drone landed  центруем");

    digitalWrite(dirPin, LOW);
    digitalWrite(dirPin2, LOW);
    move(x, y);

    Serial.println("Returning to initial position");
    digitalWrite(dirPin, HIGH);
    digitalWrite(dirPin2, HIGH);

    move(x,y);
    
    centered = true;

    if(centered == true){
      stepper.moveTo(8000);
      stepper.runToPosition();
      
      unsigned long timer = millis();
      while(millis() - timer < 1000);

      stepper.moveTo(0);
      stepper.runToPosition();

      unsigned long timer2 = millis();
      while(millis() - timer2 < 1000);
      
    }

    centered = false;
  }

  if (first < 1 && second < 1 ) {
    landed = false;  
  }
}

void move(long a, long b) {
  long max = max(a, b);
  unsigned long startTime;

  for (long i = 0; i < max; i++) {

    if (i < a) digitalWrite(stepPin, HIGH);
    if (i < b) digitalWrite(stepPin2, HIGH);

    startTime = micros();
    while (micros() - startTime < 250);

    if (i < a) digitalWrite(stepPin, LOW);
    if (i < b) digitalWrite(stepPin2, LOW);

    startTime = micros();
    while (micros() - startTime < 250);
  }
}




int moveToLimit1(int Direction){
  int prevReading = 0;
  int currReading = 0;
  unsigned long currTime = millis();
  do {
    prevReading = currReading;
    driveActuator(Direction, 1, Speed);
    timeElapsed = 0;
    while(timeElapsed < 200);
    currReading = analogRead(sensorPin1);
  } while (prevReading != currReading);
  driveActuator(0, 1, Speed);
  return currReading;
}

int moveToLimit2(int Direction){
  int prevReading = 0;
  int currReading = 0;
  unsigned long currTime = millis();
  do {
    prevReading = currReading;
    driveActuator(Direction, 2, Speed);
    timeElapsed = 0;
    while(timeElapsed < 200);
    currReading = analogRead(sensorPin2);
  } while (prevReading != currReading );
  driveActuator(0, 2, Speed);
  return currReading;
}

float mapfloat(float x, float inputMin, float inputMax, float outputMin, float outputMax){
  return (x - inputMin) * (outputMax - outputMin) / (inputMax - inputMin) + outputMin;
}

void displayOutput(){
  Serial.print("Actuator1: ");
  Serial.print(sensorVal1);
  Serial.print(" -> ");
  Serial.print(ext1);
  Serial.print(" in\t");

  Serial.print("Actuator2: ");
  Serial.print(sensorVal2);
  Serial.print(" -> ");
  Serial.print(ext2);
  Serial.println(" in");
}

void driveActuator(int Direction, int actuator, int Speed){
  int RPWM, LPWM;
  if (actuator == 1) {
    RPWM = RPWM1;
    LPWM = LPWM1;
  } else {
    RPWM = RPWM2;
    LPWM = LPWM2;
  }

  switch(Direction){
    case 1: // extend
      analogWrite(LPWM, Speed);
      analogWrite(RPWM, 0);
      break;
    case 0: // stop
      analogWrite(RPWM, 0);
      analogWrite(LPWM, 0);
      break;
    case -1: // retract
      analogWrite(LPWM, 0);
      analogWrite(RPWM, Speed);
      break;
  }
}
