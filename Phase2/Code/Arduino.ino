#Hala Almutairi 2211112873
#Zaharaa Alrashidi 2191118389
#KAYO Robotics Team
//*********************************************** Library / Pins Define ******************************************************//
// Right Line Tracking Sensors
#define LTR 4
#define LTM 3
#define LTL 2

// Define L298N motor pins
#define ENA 5  // Enable Left Motor speed controller
#define ENB 6  // Enable Right Motor speed controller
#define IN1 7  // Left Motor pin1
#define IN2 8  // Left Motor pin2
#define IN3 9  // Right Motor pin3
#define IN4 11 // Right Motor pin4

//****************************************** Variable Initialization ********************************************************//
int carSpeed = 135; // Car speed
bool stoppedMode = false; // Flag for Stop mode
String command;  // Command received from Raspberry Pi 4
String state = "stop"; // Track previous command (initial stop)

int LT_R = 0;
int LT_M = 0;
int LT_L = 0;

int LeftIR = LT_L;
int middelIR = LT_M;
int RightIR = LT_R;



// Define Middle ultrasonic pins
int Echo = A4;  
int Trig = A5;

// Define Right ultrasonic pins
int EchoR = A0;  
int TrigR = A1;

// Define Left ultrasonic pins
int EchoL = A3;  
int TrigL = A2;

int MiddleState = 0;
int LeftState = 0;
int RightState = 0;

// Distance thresholds for obstacle detection and following
#define OBSTACLE_THRESHOLD 20     // Distance in cm to detect obstacle
#define FOLLOW_DISTANCE 15        // Distance to maintain when following object
#define PATH_CLEAR_DISTANCE 60    // Distance considered "clear path"
#define MAX_FOLLOW_DISTANCE 30    // Maximum distance before considering object lost

//***************************************************** Motion Functions *****************************************************//
void stop(){
  digitalWrite(ENA, LOW);
  digitalWrite(ENB, LOW);
  Serial.println("Stop");
}

void forward(){
  int leftSpeed = carSpeed - 10; // Wheel calibration
  analogWrite(ENA, leftSpeed);
  analogWrite(ENB, carSpeed);
  digitalWrite(IN4, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN1, HIGH);
}

void back(){
  int rightSpeed = carSpeed - 8; // Wheel calibration
  analogWrite(ENA, carSpeed);
  analogWrite(ENB, rightSpeed);
  digitalWrite(IN1, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN4, LOW);
}

void left(){
  analogWrite(ENA, carSpeed);
  analogWrite(ENB, carSpeed);
  digitalWrite(IN1, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN4, HIGH);
}

void right(){
  analogWrite(ENA, carSpeed);
  analogWrite(ENB, carSpeed);
  digitalWrite(IN2, LOW);
  digitalWrite(IN4, LOW);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN3, HIGH);
}

//************************************************ Strong Left/Right Turns **************************************************//
void strongLeft() {
  analogWrite(ENA, carSpeed);
  analogWrite(ENB, carSpeed);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  delay(2000);  // Longer turn
  stop();
}

void strongRight() {
  analogWrite(ENA, carSpeed);
  analogWrite(ENB, carSpeed);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  delay(2000);  // Longer turn
  stop();
}

//****************************************** Sensor Reading Stability Function **********************************************//
int stableRead(int pin) {
  int lastState = digitalRead(pin);
  int count = 0;

  for (int i = 0; i < 3; i++) { // Take 3 consecutive readings
    if (digitalRead(pin) == lastState) count++;
    delay(10); // Short delay between readings
  }
 
  return (count >= 2) ? lastState : !lastState; // Majority filtering
}

//****************************Function to calc distance via ultrasonic sensor with improved reliability*********************//

int Distance_test(int Trig, int Echo) {
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(20);
  digitalWrite(Trig, LOW);
 
  float Fdistance = pulseIn(Echo, HIGH); // Added timeout to prevent hanging
 
  // Handle timeout case
  if (Fdistance == 0) {
    return PATH_CLEAR_DISTANCE + 10; // Return a value indicating clear path if timeout
  }
 
  Fdistance = Fdistance / 58;
  return (int)Fdistance;
}  



//***************************** Survival mode - more intelligent border following to find a way out*********************//
void Survive() {
  Serial.println("SURVIVAL MODE ACTIVATED");
  Serial.print("Left=");
  Serial.print(LeftState);
  Serial.print(", Middle=");
  Serial.print(MiddleState);
  Serial.print(", Right=");
  Serial.println(RightState);
 
  // Try to find the clearest direction
  if (LeftState > RightState) {
    // Left is clearer, turn left and try to follow something on the right
    Serial.println("Survival: trying left direction");
    left();
    delay(400);

  } else {
    // Right is clearer, turn right and try to follow something on the left
    Serial.println("Survival: trying right direction");
    right();
    delay(400);

  }


}

//************************ Functions to check for a path around obstacles and initiate following behavior********************//
void checkForPath() {
  Serial.println("Checking for path around obstacle");
  int counterR =0;
  int counterM =0;
  int counterL =0;
  // Get the latest sensor readings
  MiddleState = Distance_test(Trig, Echo);
  LeftState = Distance_test(TrigL, EchoL);
  RightState = Distance_test(TrigR, EchoR);
 
  // Print all sensor readings for debugging
  Serial.print("Distances - Left: ");
  Serial.print(LeftState);
  Serial.print("cm, Middle: ");
  Serial.print(MiddleState);
  Serial.print("cm, Right: ");
  Serial.print(RightState);
  Serial.println("cm");
 
  // First check if middle is blocked
  if (MiddleState < OBSTACLE_THRESHOLD) {
    // Middle is blocked, check left and right paths
     Serial.println("Check path to left - turning left");
      left();
      delay(500);
      stop();
      delay(100);

      // Get the latest sensor readings
      MiddleState = Distance_test(Trig, Echo);
      LeftState = Distance_test(TrigL, EchoL);
      RightState = Distance_test(TrigR, EchoR);
     
    if (MiddleState > PATH_CLEAR_DISTANCE) {
      // Clear path on left, obstacle must be on right

      Serial.println("Clear path on left - turning left");
      while(RightState < OBSTACLE_THRESHOLD){
        forward();
        delay(700);
        stop();
        delay(100);
        RightState = Distance_test(TrigR, EchoR);
        if (LT_R){
          counterR++;
        }
        if (LT_M){
          counterM++;
        }
        if (LT_L){
          counterL++;
        }
      }

      while ((counterR < 5) && (counterM < 5) && (counterL < 5) && (RightState > PATH_CLEAR_DISTANCE)){
        forward();
        delay(400);
        right();
        delay(200);
        stop();
        delay(100);
        if (LT_R){
          counterR++;
        }
       if (LT_M){
          counterM++;
        }
       if (LT_L){
          counterL++;
        }

      }



   

    }
    else if (MiddleState < PATH_CLEAR_DISTANCE) {

     Serial.println("Check path to right - turning right");
      right();
      delay(1000);
      stop();
      delay(100);
      // Clear path on right, obstacle must be on left
      // Get the latest sensor readings
      MiddleState = Distance_test(Trig, Echo);
      LeftState = Distance_test(TrigL, EchoL);
      RightState = Distance_test(TrigR, EchoR);
     
    if (MiddleState > PATH_CLEAR_DISTANCE) {
      // Clear path on left, obstacle must be on right

      Serial.println("Clear path on left - turning left");
      while(LeftState < OBSTACLE_THRESHOLD){
        forward();
        delay(700);
        stop();
        delay(100);
        LeftState = Distance_test(TrigL, EchoL);
        if (LT_R){
          counterR++;
        }
        if (LT_M){
          counterM++;
        }
        if (LT_L){
          counterL++;
        }
      }

      while ((counterR < 5) && (counterM < 5) && (counterL < 5) && (LeftState > PATH_CLEAR_DISTANCE)){
        forward();
        delay(400);
        left();
        delay(200);
        stop();
        delay(100);
        if (LT_R){
          counterR++;
        }
       if (LT_M){
          counterM++;
        }
       if (LT_L){
          counterL++;
        }

      }

    }
    else {
      // No clear path, enter survival mode to find a way out
      Serial.println("No clear path - entering survival mode");
      Survive();
    }
  }
  // If middle is clear but objects on sides
  else if (LeftState < OBSTACLE_THRESHOLD) {
    // Object on left, follow its border
    Serial.println("Object detected on left - will follow left border");

  }
  else if (RightState < OBSTACLE_THRESHOLD) {
    // Object on right, follow its border
    Serial.println("Object detected on right - will follow right border");

  }
  else {
    // No obstacles detected, continue normal operation

    Serial.println("Path clear, resuming normal operation");
  }
}

 
  // Move forward a bit to get new sensor readings
  forward();
  delay(300);
  stop();
}

//***************************************** Scanning Left and Right with Correction ******************************************//
void scanLeft() {
  left();
  delay(1000);
  stop();
}

void scanRight() {
  right();
  delay(1000);
  stop();

  // Check if center remains active after scanning
}



//*********************************************** Execution Code *************************************************************//
void setup() {
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
  pinMode(LTR, INPUT);
  pinMode(LTM, INPUT);
  pinMode(LTL, INPUT);
 
  LT_R = digitalRead(LTR);
  LT_M = digitalRead(LTM);
  LT_L = digitalRead(LTL);

  // Take initial distance readings
  MiddleState = Distance_test(Trig, Echo);
  LeftState = Distance_test(TrigL, EchoL);
  RightState = Distance_test(TrigR, EchoR);

  stop();
  state = "stop";
}

void loop() {

  carSpeed = 150;
  // Hold previous state unless multiple confirmed changes
  LeftIR = stableRead(2);
  middelIR = stableRead(3);
  RightIR = stableRead(4);
 
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
     if (cmd == "M") {
      handleManual();
    }else if (cmd == "F") {
      carSpeed = 130; // Car speed
      while ((LeftIR == LT_L) && (middelIR == LT_M) && (RightIR == LT_R) && !((RightState <= OBSTACLE_THRESHOLD) || (MiddleState <= OBSTACLE_THRESHOLD) || (LeftState <= OBSTACLE_THRESHOLD))) {
        forward();
        delay(50); // Short delay for stability
        LeftIR = stableRead(2);
        middelIR = stableRead(3);
        RightIR = stableRead(4);
        MiddleState = Distance_test(Trig, Echo);
        RightState = Distance_test(TrigR, EchoR);
        LeftState = Distance_test(TrigL, EchoL);

        // Break loop if stop command is received
        if (Serial.available() > 0) {
          String newCmd = Serial.readStringUntil('\n');
          newCmd.trim();
          if (newCmd == "S") {
            stop();
            break;
          }
        }
      }
      back();
      delay(300);
    }
    else if (cmd == "B") back();
    else if (cmd == "L") left();
    else if (cmd == "R") right();
    else if (cmd == "S") stop();
    else if (cmd == "SCAN_LEFT") scanLeft();
    else if (cmd == "SCAN_RIGHT") scanRight();
    else if (cmd == "STRONG_LEFT") strongLeft();
    else if (cmd == "STRONG_RIGHT") strongRight();
  }


// Get latest sensor readings
  MiddleState = Distance_test(Trig, Echo);
  RightState = Distance_test(TrigR, EchoR);
  LeftState = Distance_test(TrigL, EchoL);
 
  // Print sensor readings for debugging
  Serial.print("US_L:");
  Serial.print(LeftState);
  Serial.print(" US_M:");
  Serial.print(MiddleState);
  Serial.print(" US_R:");
  Serial.println(RightState);

  // Normal operation - no object following
  // Check if any sensor detects an obstacle
  if ((RightState <= OBSTACLE_THRESHOLD) || (MiddleState <= OBSTACLE_THRESHOLD) || (LeftState <= OBSTACLE_THRESHOLD)) {
    stop();
    delay(100); // Pause to get stable readings
    checkForPath(); // Determine how to navigate around obstacle
  }


  // Send sensor data to Raspberry Pi 4
  Serial.print("IR_R:"); Serial.println(LT_R);
  Serial.print("IR_M:"); Serial.println(LT_M);
  Serial.print("IR_L:"); Serial.println(LT_L);
}


void handleManual() {  
  bool manualMode = true;
  while (manualMode) {
    if (Serial.available() > 0) {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim();
      cmd.toUpperCase(); // Make command case-insensitive
          
      if (cmd == "F") {
        forward();
      }
      else if (cmd == "B") {
        back();
      }
      else if (cmd == "L") {
        left();
      }
      else if (cmd == "R") {
        right();
      }
      else if (cmd == "S") {
        stop();
      }
      else if (cmd == "X") {
        stop();
        manualMode = false;
      }
    }
  }
}
