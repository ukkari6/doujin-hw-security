//疑似メインMPU、ESP32-WROOM-32

#include <Arduino.h>
#include "key_table.h"  // key_table.hをインクルード
#include <EEPROM.h>

#define EEPROM_SIZE 1024  // EEPROMのサイズ（バイト単位）、ESP32で必要

#define AUTH_STATUS_OK 1
#define AUTH_STATUS_FAIL 0



//TX2(GPIO17)とRX2(GPIO16)のハードウェアシリアルを使う
//HardwareSerial Serial2(2);


const int LED = 2;  // LEDが接続されているピン番号
uint8_t boot_eeprom_address_L = 0; // EEPROMのアドレス、下位
uint8_t boot_eeprom_address_H= 1; // EEPROMのアドレス、上位


// XOR鍵（任意の値を使用）
const byte XOR_KEY = 0x55;  // 例: 0x55のバイトを鍵として使用

// 暗号化関数
byte xorEncryptDecrypt(byte input) {
  return input ^ XOR_KEY;  // XOR演算
}


//ブート毎にEEPROMにブート回数をカウントする。
//1023までいったら0にリセット
void bootCounter(){
  // EEPROMから2バイト読み込む
  byte lowByte = EEPROM.read(boot_eeprom_address_L);  // 下位バイト
  byte highByte = EEPROM.read(boot_eeprom_address_H); // 上位バイト

  // 16ビット値を結合
  uint16_t boot_eeprom_counter = (highByte << 8) | lowByte;

  if(boot_eeprom_counter >= 1023){
    boot_eeprom_counter = 0;
  }else{
    boot_eeprom_counter += 1;
  }
  
  Serial.print("boot_eeprom_counter : ");
  Serial.println(boot_eeprom_counter, DEC);

  Serial.print("key_table[boot_eeprom_counter] : ");
  Serial.println(key_table[boot_eeprom_counter], DEC);

  // 上位バイトと下位バイトに分けてEEPROMに保存
  byte lowByteNew = boot_eeprom_counter & 0xFF; // 下位バイト
  byte highByteNew = (boot_eeprom_counter >> 8) & 0xFF; // 上位バイト

  // 新しい値をEEPROMに保存
  EEPROM.write(boot_eeprom_address_L, lowByteNew);  // 下位バイト
  EEPROM.write(boot_eeprom_address_H, highByteNew); // 上位バイト

  // EEPROMへの書き込みを確定
  EEPROM.commit();
}

//EEPROMの内容をシリアルに送信
void bootCounter_read(){
  // EEPROMから2バイト読み込む
  byte lowByte = EEPROM.read(boot_eeprom_address_L);  // 下位バイト
  byte highByte = EEPROM.read(boot_eeprom_address_H); // 上位バイト

  // 16ビット値を結合
  uint16_t boot_eeprom_counter = (highByte << 8) | lowByte;

  // 確認のため読み出した値をシリアルモニタに表示
  Serial.print("boot counter value: ");
  Serial.println(boot_eeprom_counter, DEC);

  //Serial.print("key_table[boot_eeprom_counter] : ");
  //Serial.println(key_table[boot_eeprom_counter], DEC);  
}



//ブート回数をセキュリティチップに送信
void boot_count_tx(){
  // EEPROMから2バイト読み込む
  byte lowByte = EEPROM.read(boot_eeprom_address_L);  // 下位バイト
  //byte highByte = EEPROM.read(boot_eeprom_address_H); // 上位バイト

  // 16ビット値を結合
  //uint16_t boot_eeprom_counter = (highByte << 8) | lowByte;

  //Serial2.print(lowByte);
  Serial2.write(0xAA);
}


uint16_t securityChip_rev(){
  // データが受信されるまで待機
  while (Serial2.available() == 0) {
  }
  
  // 受信したデータを戻り値にする
  return Serial2.read();

}


void setup() {
  Serial.begin(115200);      // シリアル通信の初期化
  Serial2.begin(115200);    //TX2(GPIO17)とRX2(GPIO16)のハードウェアシリアルを使う

  pinMode(LED, OUTPUT); // LEDピンを出力モードに設定
  // EEPROMの初期化、ESP32で必要
  EEPROM.begin(EEPROM_SIZE);
}

//Serial.println(key_table[i]);
void loop() {

  //Serial2.print("ON\n");
  delay(1000);
  bootCounter();  //ブート回数をEEPROMに保存
  //bootCounter_read(); //EEPROMのブート回数を表示する

  // 暗号化
  byte lowByte = EEPROM.read(boot_eeprom_address_L);  // 下位バイト
  byte encryptedData = xorEncryptDecrypt(lowByte);
  Serial2.write(encryptedData); //暗号化したブート回数を送信する

  //boot_count_tx();  //ブート回数をシリアルで送信

  uint16_t securityChip_data = securityChip_rev();
  //Serial.println(securityChip_data, HEX); //MPUから受信したデータをそのまま返す

  // 復号化
  byte decryptedData = xorEncryptDecrypt(securityChip_data);

  Serial.print("decryptedData : ");
  Serial.println(decryptedData, DEC);



  //セキュリティ認証の状態
  int AUTH_STATUS; 
  //セキュリティチップから送られてくるデータは、ブート回数をkey_tableで参照してその値が戻ってくるので、
  //メインMPUのkey_table[ブート回数]　== セキュリティチップから戻ってきたデータを復号化したデータにする
  if(decryptedData == key_table[lowByte]){
    AUTH_STATUS = AUTH_STATUS_OK;
  }else{
    AUTH_STATUS = AUTH_STATUS_FAIL;
  }

  //セキュリティ認証OK
  while(AUTH_STATUS == AUTH_STATUS_OK){
    Serial.println("AUTH_STATUS_OK");
    digitalWrite(LED,0);
    delay(500);
    digitalWrite(LED,1);
    delay(500);
    while(1){      
    }
  }

  //セキュリティ認証FAIL
  while(AUTH_STATUS == AUTH_STATUS_FAIL){
    Serial.println("AUTH_STATUS_FAIL");
    digitalWrite(LED,0);
    delay(100);
    digitalWrite(LED,1);
    delay(100);
    while(1){      
    }
  }


}
