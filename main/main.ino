// Include the Arduino Stepper Library
#include <Stepper.h>

// Arduino library for I2C
#include <Wire.h> 

// Number of steps per output rotation
const int stepsPerRevolution = 200;

// Create Instance of Stepper library
Stepper myStepper(stepsPerRevolution, 12, 11, 10, 9);

// Pins for LED
const int red_light_pin= 3;
const int green_light_pin = 5;
const int blue_light_pin = 6;

// Pins for button
const int button = 2;


// Variable initialization
int button_state = 0; // var for reading push button status

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
double total_vol = 385000; // 385 mL = 385000 uL

int motor_steps = 0;

void open() {
  // step one revolution in one direction:
  Serial.println("Open");
  myStepper.step(1);
  motor_steps += 1;
  Serial.print("Motor Steps:");
  Serial.println(motor_steps);
  Serial.print("Total Vol: ");
  Serial.println(totalizer_volume/1000);
  Serial.print("New Vol: ");
  Serial.println(new_vol/1000);
  Serial.print("Old Vol: ");
  Serial.println(old_vol/1000);
  

}

void close(int old_vol) {
  // step one revolution in the other direction:
  Serial.println("Close");
  int steps_close = old_vol / 1000; // convert uL to mL
  steps_close += 1; // add  1 extra step to remove all the urine
  myStepper.step(steps_close);
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
// LED routine
// -----------------------------------------------------------------------------
void RGB_color(int red_light_value, int green_light_value, int blue_light_value)
 {
  analogWrite(red_light_pin, red_light_value);
  analogWrite(green_light_pin, green_light_value);
  analogWrite(blue_light_pin, blue_light_value);
}

// -----------------------------------------------------------------------------
// Arduino setup routine, just runs once:
// -----------------------------------------------------------------------------
void setup() {

  Serial.begin(9600); // initialize serial communication
  Wire.begin();       // join i2c bus (address optional for master)
  // set the speed at 20 rpm:
  myStepper.setSpeed(20);

  // init LED
  pinMode(red_light_pin, OUTPUT);
  pinMode(green_light_pin, OUTPUT);
  pinMode(blue_light_pin, OUTPUT);

  // init button
  pinMode(button, INPUT);

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
    // check if user pressed button
    if (button_state == HIGH) {
      close(old_vol);
      // reset volume 
      old_vol = 0;
    }

    measure_flow();
    last_micros = this_micros;
    this_micros = micros(); // get microseconds since board has been running program
    delta_t = ((this_micros - last_micros)) / 1000000.0 / 60.0 ; // ul/min
    totalizer_volume = totalizer_volume + sensor_reading * delta_t ; // seems to be off by 0.1 ul/min
//    Serial.print("Total Vol: ");
//    Serial.println(totalizer_volume);
//    Serial.print("New Vol: ");
//    Serial.println(new_vol);
    
    if (new_vol >= 1000){
        open(); // open 
        old_vol = old_vol + 1000;
        new_vol = new_vol - 1000; 
    }
    else{
        new_vol = totalizer_volume - old_vol ; 
    }

    // notify user <60% full
    if (old_vol < 0.6*total_vol){
      RGB_color(0, 255, 0); // Green LED
    }
    // notify user 60-90% full
    else if (old_vol >= 0.6*total_vol && old_vol < 0.9*total_vol){
      RGB_color(255, 255, 0); // Yellow LED
    }
    // notify user 90% full
    else if (old_vol >= 0.9*total_vol){
      RGB_color(255, 0, 0); // Red LED
      // bladder too full force emptying
      if (old_vol >= 0.95*total_vol){
        close(old_vol); 
        // reset volume
        old_vol = 0;
      }
    }
    delay(10); // 
  }
}
