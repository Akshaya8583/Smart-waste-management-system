#include <NewPing.h>
#include <Servo.h>
#include <IRremote.h>

// for the moisture sensor
#define Wet 875
#define sensorPower 13
#define sensorPin A0
int p=12;

// for servo motor
#define SERVO_PIN 11
#define SERVO_PIN2 10

// for metal sensor
const int metalDetectorPin = 9;

const int start = 4;

// servo object instance
Servo servoRotation;  // create a servo object to control a servo motor

Servo servoTilt;

void setup() {
  pinMode(metalDetectorPin, INPUT);

  servoRotation.attach(SERVO_PIN);  // attaches the servo on pin 11
  servoTilt.attach(SERVO_PIN2);
  servoRotation.write(60);  // sets the servo position to 0 degrees to denote the untilted platform
  servoTilt.write(90);
  pinMode(start,INPUT);

  Serial.begin(9600);  // initialize serial communication
}

void loop() {

  // for the metal sensor
  int metalStatus = digitalRead(metalDetectorPin);

  int starting = !digitalRead(start);
  Serial.println(starting);

  // for the moisture sensor
	int moisture = MoistureSensor();
  analogWrite(p,0);

  if(starting){
    Separator(moisture,metalStatus,starting);
  }
	
  delay(1000);


  // // for servo motor
  // servoTilt.write(90);  // sets the servo position to 90 degrees 
}


//  This function returns the analog moisture measurement
int MoistureSensor() {
	digitalWrite(sensorPower, HIGH);	// Turn the sensor ON
	delay(500);							// Allow power to settle
	int val = analogRead(sensorPin);	// Read the analog value from sensor
	// digitalWrite(sensorPower, LOW);		// Turn the sensor OFF

  Serial.print("Analog Output: ");
  Serial.println(val);
  return val;							// Return analog moisture value
}

// // for servomotor rotation
void rotateMotor(int duration,byte flag) {
  // Set the motor to rotate in one direction
  if(flag){
    for(int i=60;i<120;i++){
      servoRotation.write(i);
      delay(10);  // sets the servo position to 120 degrees
    }
    delay(1000); 
    for(int i=90;i>0;i--){
      servoTilt.write(i);
      delay(10);  // sets the servo position to 120 degrees
    }
    delay(1000);
    for(int i=0;i<90;i++){
      servoTilt.write(i);
      delay(5);  // sets the servo position to 120 degrees
    }
    delay(1000); 
    for(int i=120;i>60;i--){
      servoRotation.write(i);
      delay(10);  // sets the servo position to 120 degrees
    }         // sets the servo position to 0 degrees
    delay(1000);        // waits for a second
     
  }
  else{
    for(int i=60;i>0;i--){
      servoRotation.write(i); 
      delay(10); // sets the servo position to 120 degrees
    }
    delay(1000); 
    for(int i=90;i>0;i--){
      servoTilt.write(i);
      delay(10);  // sets the servo position to 120 degrees
    }
    delay(1000);
    for(int i=0;i<90;i++){
      servoTilt.write(i);
      delay(10);  // sets the servo position to 120 degrees
    }
    delay(1000); 
    for(int i=0;i<60;i++){
      servoRotation.write(i);
      delay(10);  // sets the servo position to 120 degrees
    }         // sets the servo position to 0 degrees
    delay(1000); 
    
  }

}

void Separator(int moisture,int metal,int flag){
  // Serial.println(moisture);
  if(metal){
    Serial.println("Status: Metal Waste");
    rotateMotor(1000,1);
    return ;
  }

  Serial.println(flag);
  // Determine status of our waste
	if (moisture < Wet) {
		Serial.println("Status: Wet Waste");
   rotateMotor(1000,0);
	} 
  else if(flag)
  { 
    delay(1000); 
    for(int i=90;i>0;i--){
      servoTilt.write(i);
      delay(10);  // sets the servo position to 120 degrees
    }
    delay(1000);
    for(int i=0;i<90;i++){
      servoTilt.write(i);
      delay(5);  // sets the servo position to 120 degrees
    }
    delay(1000);
		Serial.println("Status:Dry Waste");
	}
}