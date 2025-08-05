// by setting direction pin as LOW, You make motors to move counterclockwise. In order to
//move forward, they must be moving counterclockwise.
// by setting direction as HIGH, they move clockwise -> moves backwards
// NOTE: however it depends how you connect the wires of the stepper motor 
// in My setup: black - B+; green - B-; blue - A+; red - A-
const int stepPin = 9;    //y axis
const int dirPin = 8;   
const int stepPin2 = 7;   // x axis 
const int dirPin2 = 6;  

const int x = 4500;  
const int y = 4950;  

void setup(){
  Serial.begin(9600);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  Serial.println("1 Forward");
  Serial.println("2 Backward");
  
} 

void loop(){
  if(Serial.available()){
    char byte = Serial.read();

    if(byte == '1'){
    Serial.println("Forward");
    digitalWrite(dirPin, LOW);
    digitalWrite(dirPin2, LOW);
    move(x, y);
    }
      
    else if(byte == '2'){
    Serial.println("Backward");
    digitalWrite(dirPin, HIGH);
    digitalWrite(dirPin2, HIGH);
    move(x, y);
    }
  }
}

void move(long a, long b) {
  long max = max(a, b);
  unsigned long startTime;

  for (long i = 0; i < max; i++) {

    if (i < a) digitalWrite(stepPin, HIGH);
    if (i < b) digitalWrite(stepPin2, HIGH);
// using micros in order to replace delayMicroseconds
// bcs for now I am using push buttons
// for linear actuator, therefore to not have any pauses
    
    startTime = micros();
    while (micros() - startTime < 250);

    if (i < a) digitalWrite(stepPin, LOW);
    if (i < b) digitalWrite(stepPin2, LOW);

    startTime = micros();
    while (micros() - startTime < 250); 
  }
}


