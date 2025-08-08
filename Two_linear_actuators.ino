#include <elapsedMillis.h>
elapsedMillis timeElapsed;

const int RPWM1 = 5;   
const int LPWM1 = 3;
const int sensorPin1 = A0;


const int RPWM2 = 11;    
const int LPWM2 = 10;
const int sensorPin2 = A2;

int sensorVal1, sensorVal2;
float ext1, ext2;

const int Speed = 255;
const float strokeLength = 101.6; 

int max1, min1;
int max2, min2;

void setup() {
  pinMode(RPWM1, OUTPUT);
  pinMode(LPWM1, OUTPUT);
  pinMode(sensorPin1, INPUT);

  pinMode(RPWM2, OUTPUT);
  pinMode(LPWM2, OUTPUT);
  pinMode(sensorPin2, INPUT);

  Serial.begin(57600);

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
  Serial.println("1 = Extend");
  Serial.println("2 = Retract");
  Serial.println("3 = Stop");
}

void loop() {
  if (Serial.available()) {
    char command = Serial.read();

    if (command == '1') {
      Serial.println("Extending to 100 mm...");
      driveActuator(1, 1, Speed);
      unsigned long startTime = millis();

      bool actuator2Started = false; //needed to open the second linear after a second since
      // first actuator was activated
      do {
        sensorVal1 = analogRead(sensorPin1);
        ext1 = mapfloat(sensorVal1, float(min1), float(max1), 0.0, strokeLength);
        
        if (!actuator2Started && millis() - startTime >= 1000) {
          driveActuator(1, 2, Speed);
          actuator2Started = true;
        }

        if(ext1 > 60){
          driveActuator(0, 1, Speed);
        }

        if (actuator2Started) {
          sensorVal2 = analogRead(sensorPin2);
          ext2 = mapfloat(sensorVal2, float(min2), float(max2), 0.0, strokeLength);
        }

        displayOutput();

        delay(20);  

      } while ( ext2 < 60);

      driveActuator(0, 1, Speed);
      driveActuator(0, 2, Speed);
      Serial.println("Target reached: 3.93 inches/ 100 mm");

    } else if (command == '2') {
      Serial.println("Retracting to 0 mm...");
      driveActuator(-1, 1, Speed);
      unsigned long startTime = millis();

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

        delay(20);  

      } while ( ext2 > 0);

      driveActuator(0, 1, Speed);
      driveActuator(0, 2, Speed);
      Serial.println("Fully retracted.");

    } else if (command == '3') {
      Serial.println("Stopped.");
      driveActuator(0, 1, Speed);
      driveActuator(0, 2, Speed);
      displayOutput();
    }
  }
}

int moveToLimit1(int Direction){
  int prevReading = 0;
  int currReading = 0;
  do {
    prevReading = currReading;
    driveActuator(Direction, 1, Speed);
    timeElapsed = 0;
    while (timeElapsed < 200) ;  
    currReading = analogRead(sensorPin1);
  } while (prevReading != currReading);
  driveActuator(0, 1, Speed);
  return currReading;
}

int moveToLimit2(int Direction){
  int prevReading = 0;
  int currReading = 0;
  do {
    prevReading = currReading;
    driveActuator(Direction, 2, Speed);
    timeElapsed = 0;
    while (timeElapsed < 200) ;  
    currReading = analogRead(sensorPin2);
  } while (prevReading != currReading);
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
