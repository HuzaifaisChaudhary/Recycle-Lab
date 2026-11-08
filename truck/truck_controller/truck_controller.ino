// =========================================================================
// ESP32 INTEGRATED CODE: BLUETOOTH RC + LINE FOLLOWER WITH TOGGLE SWITCH
// =========================================================================
#include "BluetoothSerial.h"
#include <ctype.h> // Included for the toupper() function

// Check if Bluetooth is properly enabled in the config
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to enable it.
#endif

BluetoothSerial ESP_BT;

// -------- IR SENSORS PIN DEFINITIONS --------
#define LEFT_SENSOR    34
#define CENTER_SENSOR  35
#define RIGHT_SENSOR   32

// -------- MOTOR DRIVER PIN DEFINITIONS --------
#define IN1 14
#define IN2 27
#define IN3 26
#define IN4 25
#define ENA 33
#define ENB 23

// -------- SYSTEM VARIABLES --------
char command;
bool lineFollowingMode = false; // Starts in Manual RC mode by default
bool branchTaken = false;       // Track the first left branch for line follower

void setup() {
  Serial.begin(115200);
  
  // Initialize Bluetooth
  ESP_BT.begin("ESP32_Dual_Car"); 
  Serial.println("Bluetooth Device Ready. Mode: MANUAL RC");

  // Initialize IR Sensor Pins
  pinMode(LEFT_SENSOR, INPUT);
  pinMode(CENTER_SENSOR, INPUT);
  pinMode(RIGHT_SENSOR, INPUT);

  // Initialize Motor Pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Keep motor drivers enabled
  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);
  
  // Start stopped
  stopMotors();
}

void loop() {
  // 1. CHECK FOR MODE SWITCHING OR MANUAL COMMANDS VIA BLUETOOTH
  if (ESP_BT.available()) {
    command = ESP_BT.read();

    // Skip junk characters
    if (command == '\n' || command == '\r' || command == ' ') {
      return; 
    }

    // CONVERT TO UPPERCASE: Handles both lower and upper case input universally
    command = toupper(command);

    Serial.print("Received Command: ");
    Serial.println(command);

    // DUAL-ACTION 'W' TOGGLE: 
    // - If running, it stops the car. 
    // - If stopped, it starts line following again.
    if (command == 'W') {
      lineFollowingMode = !lineFollowingMode; // Flip the true/false switch
      stopMotors();                           // Clear motors safely during transition
      branchTaken = false;                    // Reset line tracking branch history for a fresh start
      
      Serial.print("--- W PRESSED! New Mode: ");
      Serial.println(lineFollowingMode ? "LINE FOLLOWER (STARTED)" : "MANUAL RC (STOPPED)");
      return;
    }

    // IF WE ARE IN MANUAL MODE, PROCESS MOVEMENT KEYS IMMEDIATELY
    if (!lineFollowingMode) {
      switch (command) {
        case 'F': forward();   break;
        case 'B': backward();  break;
        case 'L': turnLeft();  break;
        case 'R': turnRight(); break;
        case 'S': stopMotors(); break;
        default:  stopMotors(); break; 
      }
    }
  }

  // 2. RUN AUTONOMOUS CODE IF LINE FOLLOWING MODE IS ACTIVE
  if (lineFollowingMode) {
    runLineFollower();
  }
}

// =========================================================================
// LINE FOLLOWING AUTONOMOUS LOGIC
// =========================================================================
void runLineFollower() {
  int left   = digitalRead(LEFT_SENSOR);
  int center = digitalRead(CENTER_SENSOR);
  int right  = digitalRead(RIGHT_SENSOR);

  // Stop Box condition
  if (left == 1 && center == 1 && right == 1) {
    stopMotors();
    Serial.println("Line Follower: End Box Detected. Stopped.");
    lineFollowingMode = false; // Turn off line following so you can recover via RC manual control
    return;
  }

  // Take first left branch only once
  if (!branchTaken) {
    if (left == 1 && center == 1 && right == 0) {
      Serial.println("Line Follower: LEFT BRANCH");
      softLeft();          
      delay(180);          
      branchTaken = true;
      return;
    }
  }

  // Normal Line Following
  if (left == 0 && center == 1 && right == 0) {
    forward();
  }
  else if (left == 1 && center == 0 && right == 0) {
    turnLeft();
  }
  else if (left == 0 && center == 0 && right == 1) {
    turnRight();
  }
  else if (left == 1 && center == 1 && right == 0) {
    turnLeft();
  }
  else if (left == 0 && center == 1 && right == 1) {
    turnRight();
  }
  else if (left == 0 && center == 0 && right == 0) {
    forward();
  }
}

// =========================================================================
// BASIC MOVEMENT FUNCTIONS
// =========================================================================
void forward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void backward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turnLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void turnRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void softLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}