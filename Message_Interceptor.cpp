#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// SDA → GPIO21   SCL → GPIO22
// If screen is blank try address 0x3F instead of 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);
void printTS(unsigned long us){
  char buf[30];
  sprintf(buf,"%02lu:%02lu:%02lu.%03lu.%03lu",
    us/3600000000UL,(us/60000000)%60,(us/1000000)%60,(us/1000)%1000,us%1000);
  Serial.print(buf);
}
void onDataRecv(const uint8_t *mac, const uint8_t *data, int len){
  if(len!=20){
    Serial.println("[INVADER] Unknown packet length — ignored.");
    return;
  }
  unsigned long t = micros();
  uint32_t pktNum = ((uint32_t)data[0]<<24)|((uint32_t)data[1]<<16)
                   |((uint32_t)data[2]<<8)|data[3];
  const uint8_t *cipher = data + 4;
  Serial.println("================================================");
  Serial.print("[INVADER] Packet intercepted at   : "); printTS(t); Serial.println();
  Serial.print("          Packet #                : "); Serial.println(pktNum);
  Serial.print("          Sender MAC              : ");
  for(int i=0;i<6;i++){
    if(mac[i]<0x10) Serial.print("0");
    Serial.print(mac[i],HEX); if(i<5) Serial.print(":");
  }
  Serial.println();
  Serial.print("          Ciphertext (HEX)        : ");
  for(int i=0;i<16;i++){
    if(cipher[i]<0x10) Serial.print("0");
    Serial.print(cipher[i],HEX); Serial.print(" ");
  }
  Serial.println();
  Serial.println("          Decryption status      : FAILED — key not available");
  Serial.println("          Message                : ??? (unreadable)");
  Serial.println("================================================");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Pkt#"); lcd.print(pktNum); lcd.print(" ENCRYPTED");
  lcd.setCursor(0,1);
  for(int i=0;i<8;i++){
    if(cipher[i]<0x10) lcd.print("0");
    lcd.print(cipher[i],HEX);
  }
  delay(2000);
  lcd.setCursor(0,1);
  for(int i=8;i<16;i++){
    if(cipher[i]<0x10) lcd.print("0");
    lcd.print(cipher[i],HEX);
  }
}
void setup(){
  Serial.begin(115200);
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0); lcd.print("INVADER ACTIVE");
  lcd.setCursor(0,1); lcd.print("Listening...");
  WiFi.mode(WIFI_STA);
  if(esp_now_init() != ESP_OK){
    Serial.println("[INVADER] ESP-NOW init failed.");
    lcd.clear();
    lcd.setCursor(0,0); lcd.print("ESP-NOW FAILED");
    return;
  }
  esp_now_register_recv_cb(onDataRecv);
  Serial.println("================================================");
  Serial.println("   INVADER NODE — ESP-NOW Packet Interceptor");
  Serial.println("================================================");
  Serial.println(" Listening for broadcasts...");
  Serial.println(" No decryption key available.");
  Serial.println(" Only raw ciphertext will be captured.");
  Serial.println("================================================");
}
void loop(){ delay(1000); }