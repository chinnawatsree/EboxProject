#include <WiFi.h>
#include <FirebaseESP32.h>
#include <Keypad.h>

// ==========================================
// 1. ข้อมูลการเชื่อมต่อ Wi-Fi และ Firebase
// ==========================================
#define WIFI_SSID "kala"
#define WIFI_PASSWORD "9999999999"

#define FIREBASE_HOST "ebox-2fe90-default-rtdb.asia-southeast1.firebasedatabase.app" 
#define FIREBASE_AUTH "BOBz6ZFDh7YHCuhr6bXFvdV3Ut8I0GwkzEMIOgmL"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ==========================================
// 2. การตั้งค่าพินฮาร์ดแวร์
// ==========================================
#define RELAY_PIN 13       // ควบคุมกลอนไฟฟ้า
#define REED_PIN 21        // เซนเซอร์แม่เหล็กประตู
#define TRIG_PIN 32        // อัลตราโซนิก Trig
#define ECHO_PIN 35        // อัลตราโซนิก Echo

// ==========================================
// 3. ตั้งค่า Keypad 4x3 
// ==========================================
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {19, 18, 5, 17}; 
byte colPins[COLS] = {16, 4, 15}; 
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ตัวแปรช่วยการทำงาน
String inputOTP = "";
unsigned long previousMillis = 0;
// ปรับเวลาตรวจสอบเว็บเป็น 2 วินาที (2000ms) เพื่อลดอาการแป้นกดยาก
const long interval = 2000; 

void setup() {
  Serial.begin(115200);

  // ตั้งค่า Pin โหมด
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);    // ล็อกประตูเริ่มต้น
  pinMode(REED_PIN, INPUT_PULLUP); // เซนเซอร์ประตู
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // เชื่อมต่อ Wi-Fi
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi Connected!");

  // เชื่อมต่อ Firebase
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase Connected! E-BOX System Ready.");
}

// ฟังก์ชันสั่งปลดล็อกประตู
void unlockDoor() {
  Serial.println("🔓 Unlocking...");
  digitalWrite(RELAY_PIN, HIGH); // สั่งรีเลย์ (ปลดล็อก)
  delay(5000);                   // เปิดค้างไว้ 5 วินาที
  digitalWrite(RELAY_PIN, LOW);  // ล็อกกลับ
  Serial.println("🔒 Locked.");
}

// ฟังก์ชันเซนเซอร์จับพัสดุ (เพิ่ม Timeout ป้องกันบอร์ดค้าง)
long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Timeout 30000 ไมโครวินาที (ป้องกันบอร์ดค้างถ้าระยะไกลเกินไป)
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  
  if (duration == 0) return 999; // ถ้าหาค่าไม่ได้ให้ส่งกลับ 999
  return duration * 0.034 / 2;
}

void loop() {
  // --- 1. ส่วน Keypad (กรอก OTP ที่หน้าตู้) ---
  char key = keypad.getKey();
  if (key) {
    if (key == '#') {
      Serial.println("\nVerifying OTP...");
      
      if (Firebase.getString(fbdo, "/lockers/L01/otp")) {
        String correctOTP = fbdo.stringData();
        
        if (inputOTP == correctOTP && correctOTP != "none" && correctOTP != "") {
          Serial.println("✅ OTP Correct! Sending open_door...");

          FirebaseJson updateData;
          updateData.set("otp", "none");
          updateData.set("status", "available");
          updateData.set("owner_room", "none");
          updateData.set("command", "open_door"); 

          if (Firebase.updateNode(fbdo, "/lockers/L01", updateData)) {
            Serial.println("✅ Database updated to open_door");
            unlockDoor(); 
            Firebase.setString(fbdo, "/lockers/L01/command", "none");
            Serial.println("🔒 Command cleared to none");
          }
        } else {
          Serial.println("❌ OTP Incorrect!");
        }
      }
      inputOTP = ""; 
    } else if (key == '*') {
      inputOTP = "";
      Serial.println("\nOTP Cleared!");
    } else {
      inputOTP += key;
      Serial.print("*"); 
    }
  }

  // --- 2. ส่วนตรวจสอบคำสั่งจาก Dashboard (ทุกๆ 2 วินาที) ---
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // A. ตรวจสอบคำสั่ง 'open_door'
    if (Firebase.getString(fbdo, "/lockers/L01/command")) {
      String cmd = fbdo.stringData();
      if (cmd == "open_door") {
        Serial.println("🚨 Remote Open Command Received!");
        unlockDoor();
        Firebase.setString(fbdo, "/lockers/L01/command", "none");
      }
    }

    // B. อัปเดตสถานะเซนเซอร์ประตูแม่เหล็ก
    bool isDoorOpen = (digitalRead(REED_PIN) == HIGH);
    Firebase.setBool(fbdo, "/lockers/L01/door_open", isDoorOpen);

    // C. ตรวจสอบพัสดุในตู้ด้วยอัลตราโซนิก
    long distance = getDistance();
    bool hasParcel = false;
    
    // **จุดปรับจูน:** ถ้าระยะน้อยกว่า 15 ซม. แปลว่ามีกล่องพัสดุวางขวางอยู่
    if (distance > 0 && distance < 15) { 
      hasParcel = true;
    }
    
    // อัปเดตสถานะว่า "มีของ" หรือ "ไม่มีของ" ขึ้น Firebase
    Firebase.setBool(fbdo, "/lockers/L01/has_parcel", hasParcel);
    
    // แสดงผลออก Serial Monitor เพื่อให้เช็กระยะได้ง่าย
    Serial.print("🚪 Door: ");
    Serial.print(isDoorOpen ? "OPEN" : "CLOSED");
    Serial.print(" | 📦 Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
  }
}