/*
   Stepper_bipolar sketch

   the stepper is controlled from the serial port.
   a numeric value followed by '+' or '-' steps the motor

*/
#include <Stepper.h>
const int stepsPerRevolution = 200;  // change this to fit the number of steps per revolution
// for your motor

// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 7, 9, 11, 10);

int steps = 0;

void setup()
{
  // set the speed of the motor to 30 RPMs
  myStepper.setSpeed(30);
  Serial.begin(9600);
}
void loop()
{
  if ( Serial.available()) {
    char ch = Serial.read();
    if (ch >= '0' && ch <= '9') {
      steps = steps * 10 + ch - '0';
    }
    else if (ch == '+') {
      myStepper.step(steps);
      steps = 0;
    }
    else if (ch == '-') {
      myStepper.step(steps * -1);
      steps = 0;
    }
  }
}
