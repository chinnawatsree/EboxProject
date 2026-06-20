const functions = require("firebase-functions/v1"); // กลับมาใช้ Gen 1
const admin = require("firebase-admin");
const line = require("@line/bot-sdk");

// เริ่มต้นการเชื่อมต่อ
admin.initializeApp();

// 🔑 กุญแจ LINE ของคุณชิน
const client = new line.messagingApi.MessagingApiClient({
    channelAccessToken: "C1s7zEl+N7ej2cMBMSzqaZByjcTgRT6Ep5u+Ekuri0HfxXL4lubS+a29qmC0QgebGdLy0OXOiJR/KXb1NmtGjq60yUWWhBBQ468R3T5xsL9UfTZfBwhNArdTTFpMAM3RU0pEyXq2ZJd1NE+lBeGL9AdB04t89/1O/w1cDnyilFU="
});

// สร้างฟังก์ชัน Gen 1 + บังคับย้ายไปสิงคโปร์ (.region)
exports.notifyLineMessage = functions.region("asia-southeast1").database.ref("/lockers/{lockerId}/status")
    .onUpdate(async (change, context) => {
        const beforeStatus = change.before.val();
        const afterStatus = change.after.val();
        const lockerId = context.params.lockerId;

        // ถ้าสถานะเปลี่ยนเป็น occupied
        if (beforeStatus !== "occupied" && afterStatus === "occupied") {
            const message = {
                type: "text",
                text: `📦 แจ้งเตือนจาก Ebox!\nตู้ ${lockerId} มีพัสดุมาส่งแล้วค่ะ กรุณามารับพัสดุด้วยนะคะ`
            };

            try {
                await client.broadcast({ messages: [message] });
                console.log(`ส่งข้อความแจ้งเตือนตู้ ${lockerId} สำเร็จ!`);
            } catch (error) {
                console.error("เกิดข้อผิดพลาดในการส่ง LINE:", error);
            }
        }

        return null;
    });