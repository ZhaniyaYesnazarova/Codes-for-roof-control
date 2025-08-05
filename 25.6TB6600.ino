const int stepPin = 9;    
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

} 

void loop(){
  if(Serial.available()){
    char byte = Serial.read();

    if(byte == '1'){
    Serial.println("Lalala");
    digitalWrite(dirPin, LOW);
    digitalWrite(dirPin2, LOW);
    move(x, y);
    }
    else if(byte == '2'){
    Serial.println("Vivivi");
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

    startTime = micros();
    while (micros() - startTime < 250);

    if (i < a) digitalWrite(stepPin, LOW);
    if (i < b) digitalWrite(stepPin2, LOW);

    startTime = micros();
    while (micros() - startTime < 250);
  }
}


