const LINE_TOKEN = "TugGbX3aozYKrLgiknmi5bDT6IIqKKb81xNk88Nze/U8Q3N5KolRlFzkEXC9Cd+UGdLy0OXOiJR/KXb1NmtGjq60yUWWhBBQ468R3T5xsL+5469Kc09fC2vNBm/gU8ahn+cTc8vQsvfXJbRZCGF7yQdB04t89/1O/w1cDnyilFU=";
const FIREBASE_URL = "https://ebox-2fe90-default-rtdb.asia-southeast1.firebasedatabase.app/";

function doPost(e) {
  try {
    const jsonData = JSON.parse(e.postData.contents);

    // --- ส่วนที่ A: จัดการการลงทะเบียนจาก LINE (User ➔ Bot) ---
    if (jsonData.events && jsonData.events.length > 0) {
      const event = jsonData.events[0];
      if (event.type === "message" && event.message.type === "text") {
        const replyToken = event.replyToken;
        const userId = event.source.userId;
        const roomNum = event.message.text.trim(); // ผู้ใช้พิมพ์เลขห้องส่งมา

        // 1. บันทึกลง Google Sheets (Excel)
        const sheet = SpreadsheetApp.getActiveSpreadsheet().getSheets()[0];
        sheet.appendRow([new Date(), roomNum, userId]);

        // 2. อัปเดตข้อมูลเข้า Firebase โหนด users เพื่อให้ Dashboard ดึงไปใช้
        const firebaseUrl = FIREBASE_URL + "users/" + roomNum + ".json";
        UrlFetchApp.fetch(firebaseUrl, {
          "method": "put",
          "contentType": "application/json",
          "payload": JSON.stringify({ "line_id": userId })
        });

        // 3. ตอบกลับยืนยันกับผู้ใช้ใน LINE
        replyMessage(replyToken, `✅ ลงทะเบียนห้อง ${roomNum} สำเร็จ!\nต่อไปหากมีพัสดุมาส่ง นาริจะส่ง OTP ให้ที่นี่นะคะ ✨`);
      }
      return ContentService.createTextOutput("LINE Event Processed");
    }

    // --- ส่วนที่ B: จัดการการส่ง OTP จาก Dashboard (Dashboard ➔ Bot ➔ User) ---
    if (jsonData.line_id && jsonData.otp) {
      const message = `📦 มีพัสดุใหม่มาส่งแล้วค่ะ!\n🏢 ห้อง: ${jsonData.room}\n📍 ตู้: ${jsonData.locker}\n🔐 รหัส OTP: ${jsonData.otp}\n\nกรุณาใช้รหัสนี้เปิดตู้รับพัสดุนะคะ ✨`;
      pushMessage(jsonData.line_id, message);
      return ContentService.createTextOutput("OTP Sent Successfully");
    }

  } catch (err) {
    return ContentService.createTextOutput("Error: " + err.toString());
  }
}

// ฟังก์ชันช่วยส่งข้อความตอบกลับ (Reply)
function replyMessage(replyToken, msg) {
  UrlFetchApp.fetch("https://api.line.me/v2/bot/message/reply", {
    "method": "post",
    "headers": { "Content-Type": "application/json", "Authorization": "Bearer " + LINE_TOKEN },
    "payload": JSON.stringify({ "replyToken": replyToken, "messages": [{ "type": "text", "text": msg }] })
  });
}

// ฟังก์ชันช่วยส่งข้อความตรง (Push)
function pushMessage(to, msg) {
  UrlFetchApp.fetch("https://api.line.me/v2/bot/message/push", {
    "method": "post",
    "headers": { "Content-Type": "application/json", "Authorization": "Bearer " + LINE_TOKEN },
    "payload": JSON.stringify({ "to": to, "messages": [{ "type": "text", "text": msg }] })
  });
}
