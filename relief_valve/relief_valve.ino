// Include the Arduino Stepper Library
#include <Stepper.h>

// Number of steps per output rotation
const int stepsPerRevolution = 100;

// Pins for button
const int button = 2;
int buttonState =  0;

// Create Instance of Stepper library
Stepper myStepper(stepsPerRevolution, 12, 11, 10, 9);

int count = 0;


void setup()
{
  // set the speed at 20 rpm:
  myStepper.setSpeed(6);
  // initialize the serial port:
  Serial.begin(9600);
  // init button
  pinMode(button, INPUT);
}

void open(){
// step one revolution in one direction:
  Serial.println("clockwise");
  myStepper.step(stepsPerRevolution);
}

void close(){
  // step one revolution in the other direction:
  Serial.println("counterclockwise");
  myStepper.step(-stepsPerRevolution);
}
void loop() 
{
  buttonState = digitalRead(button);
  Serial.println("START");
  while (count==0){
    Serial.println("Stop1");
    buttonState = digitalRead(button);
    if (buttonState == HIGH){
      count++;
      delay(500);
    }
  }
  while (count==1) {
      myStepper.step(2);
      Serial.println("Open");
      buttonState = digitalRead(button);
      if (buttonState == HIGH){
      count++;
      delay(500);
    }
  }
  while (count==2){
      Serial.println("Stop2");
      buttonState = digitalRead(button);
      if (buttonState == HIGH){
      count++;
      delay(500);
    }
  }
  while (count==3){
    myStepper.step(-2);
    Serial.println("Close");
    buttonState = digitalRead(button);
    if (buttonState == HIGH){
      count=0;
      delay(500);
    }
  }
  
}
