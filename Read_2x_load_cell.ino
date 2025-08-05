
#include <HX711_ADC.h>

// Pins
const int HX711_dout_1 = 4; 
const int HX711_sck_1  = 5; 
const int HX711_dout_2 = 6; 
const int HX711_sck_2  = 7; 

// HX711 constructor (dout pin, sck pin)
HX711_ADC LoadCell_1(HX711_dout_1, HX711_sck_1); // HX711 1
HX711_ADC LoadCell_2(HX711_dout_2, HX711_sck_2); // HX711 2

unsigned long t = 0;

void setup() {
  Serial.begin(57600);
  delay(10);
  Serial.println();
  Serial.println("Starting...");

  float calibrationValue_1 = 105.0; // calibration value load cell 1
  float calibrationValue_2 = 143.0; // calibration value load cell 2

  LoadCell_1.begin();
  LoadCell_2.begin();

  unsigned long stabilizingtime = 2000; // tare precision can be improved by waiting
  bool _tare = true;
  byte loadcell_1_rdy = 0;
  byte loadcell_2_rdy = 0;

  // Run startup, stabilization and tare for both load cells
  while ((loadcell_1_rdy + loadcell_2_rdy) < 2) {
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
  static bool newDataReady = false;
  const int serialPrintInterval = 0;

  // Check for new data
  if (LoadCell_1.update()) newDataReady = true;
  LoadCell_2.update();

  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float a = LoadCell_1.getData();
      float b = LoadCell_2.getData();
      Serial.print("Load_cell 1 output val: ");
      Serial.print(a);
      Serial.print("    Load_cell 2 output val: ");
      Serial.println(b);
      newDataReady = false;
      t = millis();
    }
  }

  // Serial command to tare
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') {
      LoadCell_1.tareNoDelay();
      LoadCell_2.tareNoDelay();
    }
  }

  // Check tare status
  if (LoadCell_1.getTareStatus()) {
    Serial.println("Tare load cell 1 complete");
  }
  if (LoadCell_2.getTareStatus()) {
    Serial.println("Tare load cell 2 complete");
  }
}
