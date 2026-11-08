#include <WiFi.h>
#include <ESP32Servo.h>

// ================= SERVO OBJECTS =================
Servo servo1;  // GPIO 13
Servo servo2;  // GPIO 14

// ================= EXTRA PIN =================
const int STATUS_PIN = 12; // GPIO12

// ================= WIFI =================
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

IPAddress local_IP(192, 168, 43, 122);
IPAddress gateway(192, 168, 43, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- ESP32 STARTING ---");

  // Attach servos
  servo1.attach(13);
  servo2.attach(14);

  servo1.write(180);
  servo2.write(0);

  pinMode(STATUS_PIN, OUTPUT);
  digitalWrite(STATUS_PIN, LOW);

  Serial.println("Servos initialized at 0°");
  Serial.println("STATUS PIN: OFF");

  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin();
  Serial.println("Server started");
  Serial.println("------------------------");
}

void loop() {
  WiFiClient client = server.available();
  if (!client) return;

  while (!client.available()) delay(1);

  char command = client.read();
  client.flush();

  Serial.print("Received Command: ");
  Serial.println(command);

  // ===== SERVO 1 =====
  if (command == '1') {
    Serial.println("→ Servo 1 moving to 45°");
    digitalWrite(STATUS_PIN, HIGH);

    servo1.write(135);
    delay(8000);

    servo1.write(180);
    Serial.println("→ Servo 1 returned to 0°");

    digitalWrite(STATUS_PIN, LOW);
  }

  // ===== SERVO 2 =====
  else if (command == '2') {
    Serial.println("→ Servo 2 moving to 45°");
    digitalWrite(STATUS_PIN, HIGH);

    servo2.write(45);
    delay(8000);

    servo2.write(0);
    Serial.println("→ Servo 2 returned to 0°");

    digitalWrite(STATUS_PIN, LOW);
  }

  // ===== COMMAND 3 =====
  else if (command == '3') {
    Serial.println("→ Servo 3 moving to 45°");
    digitalWrite(STATUS_PIN, HIGH);

    //servo2.write(45);
    delay(8000);

    //servo2.write(0);
    //Serial.println("→ Servo 2 returned to 0°");

    digitalWrite(STATUS_PIN, LOW);
  }
  else {
    Serial.println("⚠ Unknown command received");
  }

  Serial.println("------------------------");
}