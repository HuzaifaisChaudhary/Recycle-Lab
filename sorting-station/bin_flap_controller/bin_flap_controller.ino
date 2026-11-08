// --- SENSOR PINS ---
const int S1_Trig = 2, S1_Echo = 3;
const int S2_Trig = 4, S2_Echo = 5;
const int S3_Trig = 6, S3_Echo = 7;

// --- OUTPUT PINS ---
// Pair 1 (Linked to Sensor 1)
const int P1_A = 8;
const int P1_B = 9;

// Pair 2 (Linked to Sensor 2)
const int P2_A = 10;
const int P2_B = 11;

// Pair 3 (Linked to Sensor 3)
const int P3_A = 12;
const int P3_B = 13;

const int detectionRange = 20; // cm

void setup() {
  Serial.begin(9600);

  // Initialize Sensors
  pinMode(S1_Trig, OUTPUT); pinMode(S1_Echo, INPUT);
  pinMode(S2_Trig, OUTPUT); pinMode(S2_Echo, INPUT);
  pinMode(S3_Trig, OUTPUT); pinMode(S3_Echo, INPUT);

  // Initialize Outputs
  pinMode(P1_A, OUTPUT); pinMode(P1_B, OUTPUT);
  pinMode(P2_A, OUTPUT); pinMode(P2_B, OUTPUT);
  pinMode(P3_A, OUTPUT); pinMode(P3_B, OUTPUT);

  // Turn all outputs OFF initially
  resetAllPins();
  
  Serial.println("--- SYSTEM READY ---");
  delay(1000);
}

void loop() {
  // 1. Read Sensors
  int dist1 = getDistance(S1_Trig, S1_Echo);
  delay(10); // interference protection
  int dist2 = getDistance(S2_Trig, S2_Echo);
  delay(10);
  int dist3 = getDistance(S3_Trig, S3_Echo);

  // 2. PRINT STATUS (Every loop cycle)
  printStatus(dist1, dist2, dist3);

  // 3. CHECK TRIGGERS
  
  // Check Sensor 1
  if (dist1 > 0 && dist1 < detectionRange) {
    printTriggerMessage(1, S1_Trig, S1_Echo);
    runSequence(P1_A, P1_B);
  }
  // Check Sensor 2
  else if (dist2 > 0 && dist2 < detectionRange) {
    printTriggerMessage(2, S2_Trig, S2_Echo);
    runSequence(P2_A, P2_B);
  }
  // Check Sensor 3
  else if (dist3 > 0 && dist3 < detectionRange) {
    printTriggerMessage(3, S3_Trig, S3_Echo);
    runSequence(P3_A, P3_B);
  }

  delay(100); // Update speed (0.1 seconds)
}

// --- THE SEQUENCE FUNCTION ---
void runSequence(int activePinA, int activePinB) {
  
  // 1. First Pin ON (3.5s)
  Serial.print(">>> ACTION: Pin "); Serial.print(activePinA); Serial.println(" ON (3.5s)");
  digitalWrite(activePinA, HIGH);
  delay(3500);
  
  // 1b. First Pin OFF
  digitalWrite(activePinA, LOW);
  
  // 2. Wait 5s
  Serial.println(">>> ACTION: Waiting 5s gap...");
  delay(5000);

  // 3. Second Pin ON (4.5s)
  Serial.print(">>> ACTION: Pin "); Serial.print(activePinB); Serial.println(" ON (4.5s)");
  digitalWrite(activePinB, HIGH);
  delay(4500);

  // 3b. Second Pin OFF
  digitalWrite(activePinB, LOW);

  // 4. Lockout 30s
  Serial.println(">>> LOCKOUT: System Paused for 30s...");
  delay(30000);
  
  Serial.println(">>> SYSTEM RESUMING...");
}

// --- VISUALIZATION HELPERS ---

void printStatus(int d1, int d2, int d3) {
  // Prints: "S1: 145cm | S2: 12cm | S3: 0cm"
  Serial.print("Readings: S1:"); 
  Serial.print(d1); 
  Serial.print("cm | S2:"); 
  Serial.print(d2); 
  Serial.print("cm | S3:"); 
  Serial.print(d3); 
  Serial.println("cm"); 
}

void printTriggerMessage(int sensorNum, int trig, int echo) {
    Serial.println("\n--------------------------------");
    Serial.println("!!! OBJECT DETECTED !!!");
    Serial.print("Source: Sensor "); 
    Serial.println(sensorNum);
}

void resetAllPins() {
  digitalWrite(P1_A, LOW); digitalWrite(P1_B, LOW);
  digitalWrite(P2_A, LOW); digitalWrite(P2_B, LOW);
  digitalWrite(P3_A, LOW); digitalWrite(P3_B, LOW);
}

int getDistance(int trig, int echo) {
  digitalWrite(trig, LOW); delayMicroseconds(2);
  digitalWrite(trig, HIGH); delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH);
  return duration * 0.034 / 2;
}