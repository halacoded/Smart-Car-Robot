// Hala Almutairi 2211112873
// Zaharaa Alrashidi 2191118389
// KAYO Robotics Team

#include <Servo.h> //Servo library to control motor
Servo myservo; // servo object



//Right Line Tracking
#define RLT_R !digitalRead(10)
#define RLT_M !digitalRead(4)
#define RLT_L !digitalRead(2)

//Left Line Tracking
#define LLT_R digitalRead(12)
#define LLT_M digitalRead(13)
#define LLT_L digitalRead(3) //took a LED pin to not mess with the motors

// Define L298N motor pins
#define ENA 5  // Enable Left Motor speed controller
#define ENB 6  // Enable Right Motor speed controller
#define IN1 7  // Left Motor pin1
#define IN2 8  // Left Motor pin2
#define IN3 9  // Right Motor pin3
#define IN4 11 // Right Motor pin4
int carSpeed = 150; // car speed 

bool stoppedMode = false; // Flag for Stop mode
char command;  //to store command received from website
String state = "stop"; //keep track of prev command ---> initial stop

// Define Middle ultrasonic pins
int Echo = A4;  
int Trig = A5; 
// Define Right ultrasonic pins
int EchoR = A0;  
int TrigR = A1; 
// Define Left ultrasonic pins
int EchoL = A3;  
int TrigL = A2; 

// initialize ultrasonic 
int rightDistance = 0;
int leftDistance = 0;
int middleDistance = 0;

// Function to STOP car
void stop(){
  digitalWrite(ENA, LOW);
  digitalWrite(ENB, LOW);
  Serial.println("Stop");
} 

// Function to move car forward with calibrated speeds
void forward(){ 
  int leftSpeed = carSpeed-10;
  analogWrite(ENA, leftSpeed);
  analogWrite(ENB, carSpeed);
  digitalWrite(IN4, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN1, HIGH);
 
 
  
 
  Serial.println("Forward");
}

// Function to move car back with calibrated speeds
void back(){
  int rightSpeed = carSpeed-8;
  analogWrite(ENA, carSpeed);
  analogWrite(ENB, rightSpeed);
  digitalWrite(IN1, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN4, LOW);
  Serial.println("Back");
}

// Function to move car left with calibrated speeds
void left(){
  analogWrite(ENA, carSpeed);
  analogWrite(ENB, carSpeed);
  digitalWrite(IN1, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN4, HIGH); 
  Serial.println("Left");
}

// Function to move car right with calibrated speeds
void right(){
  analogWrite(ENA, carSpeed);
  analogWrite(ENB, carSpeed);
  digitalWrite(IN2, LOW);
  digitalWrite(IN4, LOW);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN3, HIGH);
  Serial.println("Right");
}

// Function to calc distance via ultrasonic sensor
int Distance_test(int Trig, int Echo) {
  digitalWrite(Trig, LOW);    //Initialize Trig
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);  //Send Trig pulses
  delayMicroseconds(20);
  digitalWrite(Trig, LOW);   //Stop Trig pulses
  float Fdistance = pulseIn(Echo, HIGH);  // Calc pulse distance
  Fdistance = Fdistance / 58;       // Convert distance to cm
  return (int)Fdistance; //Return Distance 
}  
int Distance_test2(int Trig, int Echo) {
  digitalWrite(Trig, LOW);    //Initialize Trig
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);  //Send Trig pulses
  delayMicroseconds(20);
  digitalWrite(Trig, LOW);   //Stop Trig pulses
  float Fdistance = pulseIn(Echo, HIGH);  // Calc pulse distance
  Fdistance = Fdistance / 58;       // Convert distance to cm
  return (int)Fdistance; //Return Distance 
}  
void setup() { 
  myservo.attach(3); // Attach servo obj to pin3
  Serial.begin(9600);
  pinMode(Echo, INPUT);    
  pinMode(Trig, OUTPUT);  
  pinMode(EchoR, INPUT);    
  pinMode(TrigR, OUTPUT);
  pinMode(EchoL, INPUT);    
  pinMode(TrigL, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(RLT_R, INPUT);
  pinMode(RLT_M, INPUT);
  pinMode(RLT_L, INPUT);
  pinMode(LLT_R, INPUT);
  pinMode(LLT_M, INPUT);
  pinMode(LLT_L, INPUT);
  state = "stop";
  stop(); // Initialize car at stop mode
} 

void loop() { 
  if (Serial.available() > 0) { // Check if there is command from Serial communication
    command = Serial.read();
    
    if (command == 'R') {
      stoppedMode = false;
    } else if (command == 'x') {
      stoppedMode = true;   
      stop();            
    } else if (command == 'L') { // Slow
      carSpeed = 90;
    } else if (command == 'M') { // Medium
      carSpeed = 150;
    } else if (command == 'H') { // Fast
      carSpeed = 220;
    } else if (!stoppedMode) { // if is not any of the Mode flag then its Remote Mode
      if (command == 'f') {
        state = "forward";
        forward();
      } else if (command == 'b') {
        state = "back";
        back();
      } else if (command == 'l') {
        if(state == "forward") {
          left();
          delay(100);
          forward();
        } else if (state == "back") {
          left();
          delay(100);
          back();
        } else if (state == "stop") {
          left();
        }
      } else if (command == 'r') {
        if(state == "forward") {
          right();
          delay(100);
          forward();
        } else if (state == "back") {
          right();
          delay(100);
          back();
        } else if (state == "stop") {
          right();
        }
      } else if (command == 's') {
        state = "stop";
        stop();
      }
    }
  }
  
  //send ultrasonic sensors data to Raspberry Pi4 via serial
  Serial.print("US_M:");
  Serial.println(Distance_test(Trig, Echo));
  Serial.print("US_R:");
  Serial.println(Distance_test2(TrigR, EchoR)); 
  Serial.print("US_L:");
  Serial.println(Distance_test(TrigL, EchoL));
  
  //send right IR sensors data to Raspberry Pi4 via serial
  Serial.print("IR_R_R:"); Serial.println(RLT_R); 
  Serial.print("IR_R_M:"); Serial.println(RLT_M); 
  Serial.print("IR_R_L:"); Serial.println(RLT_L);
  
  //send left IR sensors data to Raspberry Pi4 via serial
  Serial.print("IR_L_R:"); Serial.println(LLT_R);
  Serial.print("IR_L_M:"); Serial.println(LLT_M); 
  Serial.print("IR_L_L:"); Serial.println(LLT_L);
}