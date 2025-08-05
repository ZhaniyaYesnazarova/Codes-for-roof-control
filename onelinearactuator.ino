#include <elapsedMillis.h>
elapsedMillis timeElapsed;

int RPWM = 11;   //check ur digital pin
int LPWM = 10;  // check ur digital pin
int sensorPin = A2; // check ur analog pin

int sensorVal;
int Speed = 200;
float strokeLength = 101.6; 
float extensionLength;

int maxAnalogReading;
int minAnalogReading;

void setup() {
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(sensorPin, INPUT);
  Serial.begin(9600);

  // This part is needed to calibrate the actuators,
  // when uploading code, it will extend fully and retract. 
  Serial.println("Calibrating limits...");
  maxAnalogReading = moveToLimit(1);   // Extend fully
  delay(1000);
  minAnalogReading = moveToLimit(-1);  // Retract fully
  delay(1000);

  Serial.println("Calibration complete.");
  Serial.println("1 = Extend");
  Serial.println("2 = Retract");
  Serial.println("3 = Stop");
}

void loop() {
  if (Serial.available()) {
    char command = Serial.read();
// if 1 is entered to Serial monitor, linear actuator will extend
    if (command == '1') {
      Serial.println("Extending to 100 mm...");
      driveActuator(1, Speed);
      do {
        sensorVal = analogRead(sensorPin);
        //this is needed to convert analogreadings to length extracted
        extensionLength = mapfloat(sensorVal, float(minAnalogReading), 
        float(maxAnalogReading), 0.0, strokeLength);
        displayOutput();
        delay(20);  
      } while (extensionLength < 80); // you can adjust the length of the stroke here
      // units are in millimeters
      driveActuator(0, Speed);  
      Serial.println("Target reached: 100 mm");
    }

else if (command == '2') {
      Serial.println("Extending to 100 mm...");
      driveActuator(-1, Speed);
      do {
        sensorVal = analogRead(sensorPin);
        extensionLength = mapfloat(sensorVal, float(minAnalogReading), 
        float(maxAnalogReading), 0.0, strokeLength);
        displayOutput();
        delay(20);  
      } while (extensionLength > 1);
  driveActuator(0, Speed);
  Serial.println("Fully retracted.");
}


    else if (command == '3') {
      Serial.println("Stopped.");
      driveActuator(0, Speed);
      displayOutput();
    }
  }
}

//used for calibration
int moveToLimit(int Direction){
  int prevReading = 0;
  int currReading = 0;
  do {
    prevReading = currReading;
    driveActuator(Direction, Speed);
    timeElapsed = 0;
    while (timeElapsed < 200) delay(1);  // wait 200ms
    currReading = analogRead(sensorPin);
    Serial.print("DEBUG ANALOG READING: ");
    Serial.println(currReading);
  } while (prevReading != currReading);
  driveActuator(0, Speed);
  return currReading;
}

float mapfloat(float x, float inputMin, float inputMax, 
float outputMin, float outputMax){
  return (x - inputMin) * (outputMax - outputMin) / (inputMax - inputMin) + outputMin;
}

void displayOutput(){
  Serial.print("Analog Reading: ");
  Serial.print(sensorVal);
  Serial.print("\tExtension: ");
  Serial.print(extensionLength);
  Serial.println(" mm");
}

void driveActuator(int Direction, int Speed){
  switch(Direction){
    case 1: // extend
    //LPWm in order to mpve forward
      analogWrite(LPWM, Speed);
      analogWrite(RPWM, 0);
      break;
    case 0: // stop
  
      analogWrite(RPWM, 0);
      analogWrite(LPWM, 0);
      break;
    case -1: // retract
      //RPWM to move backward
      analogWrite(LPWM, 0);
      analogWrite(RPWM, Speed);
      break;
  }
}