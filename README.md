# 📦 E-BOX: Smart Locker System via LINE OA

**E-BOX** เป็นระบบตู้พัสดุอัจฉริยะสำหรับหอพักขนาด SME ที่พัฒนาขึ้นเพื่อช่วยเหลือผู้ดูแลและผู้พักอาศัยในการบริหารจัดการพัสดุอย่างมีประสิทธิภาพ โดยประยุกต์ใช้เทคโนโลยีอินเทอร์เน็ตของสรรพสิ่ง (IoT) ร่วมกับระบบคลาวด์และแพลตฟอร์มการสื่อสาร (ESP32, Firebase Realtime Database, Google Apps Script และ LINE Messaging API) เพื่อให้การรับฝากพัสดุมีความปลอดภัย สะดวกสบาย แจ้งเตือนรหัส OTP และตรวจสอบสถานะได้แบบเรียลไทม์ 24 ชั่วโมง

## 👥 ผู้จัดทำ (Created By)
* **Chinnawat Sreesangjun** (Student Code: [ใส่รหัสนักศึกษาของคุณชิน])
* Computer Science Curriculum, Faculty of Science and Technology, Huachiew Chalermprakiet University
* Project Advisor: **Ajarn Premrat Poonsawat**

---

## 📂 โครงสร้างโปรเจกต์ (Project Structure)
โปรเจกต์นี้ใช้สถาปัตยกรรมแบบแยกส่วนการทำงาน (Microservices Architecture) เพื่อความปลอดภัยและง่ายต่อการบำรุงรักษา:

* **`ESP_Code/`** : โค้ดควบคุมฮาร์ดแวร์ (ESP32, Ultrasonic, Reed Switch, Solenoid Lock) พัฒนาด้วย **C++ (Arduino)**
* **`ebox-frontend/`** : หน้าเว็บ Dashboard สำหรับผู้ดูแลระบบ (HTML, CSS, JavaScript, Tailwind)
* **`ebox-backend/`** : ระบบ API / เซิร์ฟเวอร์จัดการข้อมูลเบื้องหลัง
* **`GAS/`** : สคริปต์ Google Apps Script สำหรับจัดการ Webhook และเชื่อมต่อ LINE API
* **`database.json`** : ไฟล์โครงสร้างฐานข้อมูล Firebase (Schema)
* **`.gitignore`** : ไฟล์กำหนดข้อยกเว้นการนำไฟล์ความลับขึ้น Git

---

## 🗄️ โครงสร้างฐานข้อมูล (Database Schema)
ระบบใช้ **Firebase Realtime Database** ในการเป็นศูนย์กลางข้อมูล (`database.json` แสดงโครงสร้างเริ่มต้น) ตัวอย่างโครงสร้างโหนดหลักของตู้พัสดุ:

```json
{
  "lockers": {
    "L01": {
      "status": "available",
      "otp": "none",
      "owner_room": "none",
      "door_open": false,
      "has_parcel": false,
      "command": "none"
    }
  },
  "logs": { ... }
}
```

⚙️ 1. ฝั่งฮาร์ดแวร์ (ESP_Code)
ควบคุมการอ่านค่าเซนเซอร์ ตรวจสอบ OTP จาก Keypad และสั่งงานกลอนประตูไฟฟ้า

🛠️ การติดตั้ง
ใช้โปรแกรม Arduino IDE พร้อมติดตั้ง Board Manager: esp32

ติดตั้งไลบรารี: Firebase ESP32 Client, Keypad, LiquidCrystal_I2C

ก่อนอัปโหลดโค้ด ให้สร้างไฟล์ config.h หรือแก้ไขตัวแปรส่วนหัวดังนี้:

C++
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define FIREBASE_HOST "YOUR_FIREBASE_URL"
#define FIREBASE_AUTH "YOUR_DATABASE_SECRET"
🌐 2. ฝั่ง Web Dashboard (ebox-frontend / ebox-backend)
ระบบหน้าเว็บสำหรับตรวจสอบสถานะเซนเซอร์แบบเรียลไทม์ และทำรายการฝากพัสดุ

🛠️ การติดตั้งและการรันระบบ
เปิด Terminal แล้วเข้าไปที่โฟลเดอร์ ebox-frontend หรือ ebox-backend

หากมีการใช้ Node.js ให้รันคำสั่ง:

Bash
npm install
สร้างไฟล์ .env เพื่อเก็บค่าคอนฟิกของ Firebase (ห้ามนำไฟล์นี้ขึ้น Git):

Code snippet
FIREBASE_API_KEY=your_api_key
FIREBASE_AUTH_DOMAIN=your_project_id.firebaseapp.com
FIREBASE_DATABASE_URL=https://your_project_id.firebaseio.com
รันคำสั่ง npm start หรือ npm run dev (หรือใช้ Live Server เปิดไฟล์ index.html)

💬 3. ฝั่ง Middleware (Google Apps Script)
รับ Trigger จากหน้าเว็บหรือ Firebase เพื่อสร้างแจ้งเตือนและยิง API เข้า LINE ของผู้พักอาศัย

🛠️ การติดตั้ง
คัดลอกโค้ดจากไฟล์ในโฟลเดอร์ GAS/

นำไปสร้างโปรเจกต์ใหม่บน Google Apps Script

กำหนดค่าตัวแปรในโค้ด:

JavaScript
const LINE_ACCESS_TOKEN = 'YOUR_LINE_CHANNEL_ACCESS_TOKEN';
เลือก Deploy -> Web app กำหนดสิทธิ์เป็น Anyone และนำ URL ไปใช้งาน

⚠️ ข้อควรระวังด้านความปลอดภัย (Security Guidelines)
ข้อมูล WIFI_PASSWORD, FIREBASE_AUTH, และ LINE_ACCESS_TOKEN เป็นความลับขั้นสูงสุด ห้าม Push ขึ้น GitHub เด็ดขาด

ตรวจสอบให้แน่ใจว่าไฟล์ความลับทั้งหมด รวมถึงโฟลเดอร์ระบบเช่น node_modules/ ถูกระบุไว้ในไฟล์ .gitignore เรียบร้อยแล้ว
