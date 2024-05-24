//Robot control software
//Version for A4988 drivers and STM32 Blue Pill Development board

//screw thread 10mm; motor 200 steps

#include <AccelStepper.h>
#include <ArduinoJson.h>

StaticJsonDocument<40> doc;


//Pin and variable definitions
int ENABLE = 4;
int MS1 = 3;
int MS2 = 2;
int MS3 = 1;
unsigned int pos[3] = {0,0,0};
int spd[3] = {400,1000,400};
String axis[3] = {"x","z","y"};
int end_stop[3] = {7, 6, 5};

//Software reset funcion
void(* resetF) (void) = 0;

//AccelStepper(DRIVER, stepPin, dirPin)
//Speed = step/s; move = step


AccelStepper stepper_matrix[3] = {AccelStepper(1, 13, 12), AccelStepper(1, 11, 10), AccelStepper(1, 9, 8)};

//Motor move funtion definition
void motorMove() {
  String motors_position = "{ \"x\": "+String(stepper_matrix[0].currentPosition(), DEC)+", \"z\": "+String(stepper_matrix[1].currentPosition(), DEC)+", \"y\": "+String(stepper_matrix[2].currentPosition(), DEC)+"}";
  Serial.println(motors_position);
  delay(2); //temporary but works very well
  if (Serial.available() > 0) {
    String line = Serial.readStringUntil('#');
    if (line.length() > 6 & line.length() < 37) {
      DeserializationError error = deserializeJson(doc, line);
      //if json object is malformed print error
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        Serial.println(line);
        return;
      } else {
        if (doc["home"] == 1) {
          resetF();
        } else {
          for (int i=0; i<3; i++) {
            pos[i] = doc[axis[i]];
          }
        }
      }
    } else {
      return;
    }
  } else {
  //set future motor position and enable move to this position
    for(int i=0; i<3; i++) {
      stepper_matrix[i].moveTo(pos[i]);
      stepper_matrix[i].setSpeed(spd[i]);
      stepper_matrix[i].runSpeedToPosition();
    }
  }
}
//zero out axes
void motorsZero() {
  for (int i=0; i<3; i++) {
    stepper_matrix[i].setMaxSpeed(1000);
    stepper_matrix[i].setSpeed(-1000);
    while(true) {
      stepper_matrix[i].runSpeed();
      if (digitalRead(end_stop[i]) == true) {
        stepper_matrix[i].setCurrentPosition(0);
        break;
      }
    }
    Serial.println(i);
  }
}

void setup() {

  //Serial setup. serial timeout set to minimum to avoid problems in movement
  Serial.begin(2000000);
  //Serial.setTimeout(1/(pow(10,10)));
  Serial.setTimeout(5);
  //set modes to pins. This configures micro step mode. Setup for A4988
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);
  pinMode(ENABLE, OUTPUT);

  //HALF STEP
  digitalWrite(ENABLE, LOW);
  digitalWrite(MS1, HIGH);
  digitalWrite(MS2, LOW);
  digitalWrite(MS3, LOW);

  //zero out axes before main loop
  motorsZero();
}


void loop() {
  motorMove();
}
