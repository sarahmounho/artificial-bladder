// Include the Arduino Stepper Library
#include <Stepper.h>

// Arduino library for I2C
#include <Wire.h> 

// Number of steps per output rotation
const int stepsPerRevolution = 200;

// Create Instance of Stepper library
Stepper myStepper(stepsPerRevolution, 12, 11, 10, 9);

// Variable initialization 
const int ADDRESS = 0x08; // Standard address for LD20 Liquid Flow Sensors
const float SCALE_FACTOR_FLOW = 10.0;
const float SCALE_FACTOR_TEMP = 200.0;
const char *UNIT_FLOW = " ul/min";
const char *UNIT_TEMP = " deg C";

int ret;
int16_t signed_flow_value;
float sensor_reading;
double totalizer_volume = 0.0;
double delta_t;
double this_micros;
double last_micros;

double new_vol = 0.0;
double old_vol = 0.0;

void open() {
  // step one revolution in one direction:
  Serial.println("Open");
  myStepper.step(stepsPerRevolution);
}

void close() {
  // step one revolution in the other direction:
  Serial.println("Close");
  myStepper.step(-stepsPerRevolution);
}

// -----------------------------------------------------------------------------
// Measurement routine
// -----------------------------------------------------------------------------
void measure_flow(){
  
  Wire.requestFrom(ADDRESS, 3);
  if (Wire.available() < 3) {
    Serial.println("Error while reading flow measurement");
  }

  signed_flow_value  = Wire.read() << 8; // read the MSB from the sensor
  signed_flow_value |= Wire.read();      // read the LSB from the sensor

  sensor_reading = ((float) signed_flow_value) / SCALE_FACTOR_FLOW;
}
// -----------------------------------------------------------------------------
// Arduino setup routine, just runs once:
// -----------------------------------------------------------------------------
void setup() {

  Serial.begin(9600); // initialize serial communication
  Wire.begin();       // join i2c bus (address optional for master)
  // set the speed at 20 rpm:
  myStepper.setSpeed(20);

  do {
    // Soft reset the sensor
    Wire.beginTransmission(0x00);
    Wire.write(0x06);
    ret = Wire.endTransmission();
    if (ret != 0) {
      Serial.println("Error while sending soft reset command, retrying...");
      delay(500); // wait long enough for chip reset to complete
    }
  } while (ret != 0);
  delay(50); // wait long enough for chip reset to complete

  // Begin measurement
  Wire.beginTransmission(ADDRESS);
  Wire.write(0x36);
  Wire.write(0x08);
  ret = Wire.endTransmission();
  if (ret != 0) {
    Serial.println("Error while sending start measurement command, retrying...");
  }
}


// -----------------------------------------------------------------------------
// The Arduino loop routine runs over and over again forever:
// -----------------------------------------------------------------------------
void loop(){
  
  

  // init
  this_micros = micros();
  totalizer_volume = 0.0;
  new_vol = 0.0;

  while (true) {

    measure_flow();
    last_micros = this_micros;
    this_micros = micros(); // get microseconds since board has been running program
    delta_t = ((this_micros - last_micros)) / 1000000.0 / 60.0 ; // ul/min
    totalizer_volume = totalizer_volume + sensor_reading * delta_t ; // seems to be off by 0.1 ul/min
    Serial.print("Total Vol: ");
    Serial.println(totalizer_volume);
    Serial.print("New Vol: ");
    Serial.println(new_vol);
    
    if (new_vol >= 100){
        open(); // open 
        old_vol = old_vol + 100;
        new_vol = new_vol - 100; 
    }
    else{
        new_vol = totalizer_volume - old_vol ; 

    }
    delay(10); // 
  }
}
