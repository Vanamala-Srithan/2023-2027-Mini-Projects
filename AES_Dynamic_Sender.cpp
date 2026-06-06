#include <WiFi.h>
#include <esp_now.h>
#define GAS_PIN 34
#define uS_TO_S 1000000ULL
RTC_DATA_ATTR uint32_t packetCount  = 0;
RTC_DATA_ATTR uint32_t sleepSeconds = 0;
uint8_t broadcastAddress[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
esp_now_peer_info_t peerInfo;
const uint8_t sbox[256] = {
  0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
  0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
  0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
  0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
  0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
  0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
  0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
  0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
  0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
  0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
  0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
  0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
  0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
  0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
  0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
  0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
};
const uint8_t baseKey[16] = {
  0x10,0x22,0x30,0x44,0x50,0x66,0x70,0x88,
  0x90,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0x11
};
void buildKey(uint32_t n, uint8_t *out){ for(int i=0;i<16;i++) out[i]=baseKey[i]+(uint8_t)n; }
uint8_t xtime(uint8_t x){ return (x<<1)^((x&0x80)?0x1b:0x00); }
void subBytes(uint8_t *s){ for(int i=0;i<16;i++) s[i]=sbox[s[i]]; }
void shiftRows(uint8_t *s){
  uint8_t t;
  t=s[1]; s[1]=s[5]; s[5]=s[9]; s[9]=s[13]; s[13]=t;
  t=s[2]; s[2]=s[10]; s[10]=t;
  t=s[6]; s[6]=s[14]; s[14]=t;
  t=s[3]; s[3]=s[15]; s[15]=s[11]; s[11]=s[7]; s[7]=t;
}
void mixColumns(uint8_t *s){
  for(int i=0;i<4;i++){
    uint8_t b0=s[i],b1=s[i+4],b2=s[i+8],b3=s[i+12],t=b0^b1^b2^b3;
    s[i]=b0^t^xtime(b0^b1); s[i+4]=b1^t^xtime(b1^b2);
    s[i+8]=b2^t^xtime(b2^b3); s[i+12]=b3^t^xtime(b3^b0);
  }
}
void addRoundKey(uint8_t *s,const uint8_t *k){ for(int i=0;i<16;i++) s[i]^=k[i]; }
void encrypt(uint8_t *in, uint8_t *out, const uint8_t *k){
  uint8_t s[16]; memcpy(s,in,16);
  addRoundKey(s,k);
  for(int r=0;r<9;r++){ subBytes(s); shiftRows(s); mixColumns(s); addRoundKey(s,k); }
  subBytes(s); shiftRows(s); addRoundKey(s,k);
  memcpy(out,s,16);
}
void printTS(unsigned long us){
  char buf[30];
  sprintf(buf,"%02lu:%02lu:%02lu.%03lu.%03lu",
    us/3600000000UL,(us/60000000)%60,(us/1000000)%60,(us/1000)%1000,us%1000);
  Serial.print(buf);
}
void doWorkCycle(){
  unsigned long cycleStart = micros();
  Serial.println("================================================");
  Serial.print("[WAKE]  System woke up at         : "); printTS(cycleStart); Serial.println();
  Serial.print("        Packet #                  : "); Serial.println(packetCount);
  Serial.println("------------------------------------------------");
  unsigned long t1 = micros();
  int gasValue = analogRead(GAS_PIN);
  unsigned long t2 = micros();
  Serial.print("[T1]    Sensor read at            : "); printTS(t2); Serial.println();
  Serial.print("        Gas Value                 : "); Serial.println(gasValue);
  Serial.print("        Sensor read time          : "); Serial.print(t2-t1); Serial.println(" us");
  Serial.println("------------------------------------------------");
  uint8_t sessionKey[16];
  buildKey(packetCount, sessionKey);
  uint8_t msg[16]={0}, enc[16];
  sprintf((char*)msg,"G:%d",gasValue);
  unsigned long t3 = micros();
  encrypt(msg, enc, sessionKey);
  unsigned long t4 = micros();
  Serial.print("[T2]    Encryption done at        : "); printTS(t4); Serial.println();
  Serial.print("        Encryption time           : "); Serial.print(t4-t3); Serial.println(" us");
  Serial.print("        Session key (packet #)    : "); Serial.println(packetCount);
  Serial.print("        Encrypted (HEX)           : ");
  for(int i=0;i<16;i++){
    if(enc[i]<0x10) Serial.print("0");
    Serial.print(enc[i],HEX); Serial.print(" ");
  }
  Serial.println();
  Serial.println("------------------------------------------------");
  uint8_t packet[20];
  packet[0]=(packetCount>>24)&0xFF; packet[1]=(packetCount>>16)&0xFF;
  packet[2]=(packetCount>>8)&0xFF;  packet[3]=packetCount&0xFF;
  memcpy(packet+4, enc, 16);
  unsigned long t5 = micros();
  esp_now_send(broadcastAddress, packet, 20);
  delay(100);
  unsigned long t6 = micros();
  Serial.print("[T3]    Transmission done at      : "); printTS(t6); Serial.println();
  Serial.print("        Transmission time         : "); Serial.print(t6-t5); Serial.println(" us");
  Serial.print("        Total active time         : "); Serial.print(t6-cycleStart); Serial.println(" us");
  Serial.print("================================================");
  Serial.print("[SLEEP] Sleeping for              : "); Serial.print(sleepSeconds); Serial.println(" seconds...");
  Serial.println("================================================");
  Serial.flush();
  packetCount++;
  esp_sleep_enable_timer_wakeup(sleepSeconds * uS_TO_S);
  esp_deep_sleep_start();
}
void setup(){
  Serial.begin(115200);
  delay(500);
  WiFi.mode(WIFI_STA);
  esp_now_init();
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel=0; peerInfo.encrypt=false;
  esp_now_add_peer(&peerInfo);
  esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();
  if(reason == ESP_SLEEP_WAKEUP_TIMER){
    Serial.println("================================================");
    Serial.println("[BOOT]  Woke from deep sleep.");
    doWorkCycle();
  } else {
    Serial.println("================================================");
    Serial.println("   SECURE IoT SENDER — Modified AES + ESP-NOW  ");
    Serial.println("================================================");
    Serial.println(" Enter sleep interval in seconds (e.g. 10):");
    Serial.print(" >>> ");
    while(!Serial.available());
    sleepSeconds = Serial.parseInt();
    Serial.println(sleepSeconds);
    Serial.print("[BOOT]  Interval set to: "); Serial.print(sleepSeconds); Serial.println(" sec.");
    doWorkCycle();
  }
}
void loop(){}