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
uint8_t boot_eeprom_address = 0; // EEPROMのアドレス

// XOR鍵（任意の値を使用）
const uint16_t XOR_KEY = 0x99;  // 例: 0x99のバイトを鍵として使用

// 暗号化関数
uint16_t xorEncryptDecrypt(uint16_t input) {
  return input ^ XOR_KEY;  // XOR演算
}


//ブート毎にEEPROMにブート回数をカウントする。
//1023までいったら0にリセット
void bootCounter(){
  // EEPROMから2バイト読み込む
  uint8_t boot_eeprom_counter = EEPROM.read(boot_eeprom_address); 

  if(boot_eeprom_counter >= 255){
    boot_eeprom_counter = 0;
  }else{
    boot_eeprom_counter += 1;
  }
  
  Serial.print("boot_eeprom_counter : ");
  Serial.println(boot_eeprom_counter, DEC);

  //Serial.print("key_table[boot_eeprom_counter] : ");
  //Serial.println(key_table[boot_eeprom_counter], DEC);

  // 新しい値をEEPROMに保存
  EEPROM.write(boot_eeprom_address, boot_eeprom_counter);

  // EEPROMへの書き込みを確定
  EEPROM.commit();
}

//EEPROMの内容をシリアルに送信
void bootCounter_read(){
  // EEPROMから2バイト読み込む
  uint8_t value = EEPROM.read(boot_eeprom_address); 

  // 確認のため読み出した値をシリアルモニタに表示
  //Serial.print("boot counter value: ");
  //Serial.println(value, DEC);

  //Serial.print("key_table[lowByte] : ");
  //Serial.println(key_table[lowByte], DEC);  
}



//ブート回数をセキュリティチップに送信
void boot_count_tx(){
  // EEPROMから2バイト読み込む
  uint8_t value = EEPROM.read(boot_eeprom_address); 

  //暗号化
  uint16_t encryptedData = xorEncryptDecrypt(value);
  //erial.print("encryptedData : ");
  //Serial.println(encryptedData);

  Serial2.write(encryptedData);
}


uint16_t securityChip_rev(){
  // データが受信されるまで待機
  while (Serial2.available() == 0) {
  }
  //下位バイトを受信
  uint8_t value =  Serial2.read();

  return value;
}


//セキュリティ認証の検証
int check_securityKey(uint16_t decryptedData){

  // EEPROMから読み込む
  uint16_t boot_eeprom_counter = EEPROM.read(boot_eeprom_address); 


  //セキュリティチップから送られてくるデータは、ブート回数をkey_tableで参照してその値が戻ってくるので、
  //メインMPUのkey_table[ブート回数]　== セキュリティチップから戻ってきたデータを復号化したデータにする
  if(decryptedData == key_table[boot_eeprom_counter]){
    return(AUTH_STATUS_OK);
  }

  return(AUTH_STATUS_FAIL);
}


void setup() {
  Serial.begin(115200);      // シリアル通信の初期化
  Serial2.begin(115200);    //TX2(GPIO17)とRX2(GPIO16)のハードウェアシリアルを使う

  pinMode(LED, OUTPUT); // LEDピンを出力モードに設定
  // EEPROMの初期化、ESP32で必要
  EEPROM.begin(EEPROM_SIZE);
}


void loop() {

  delay(1000);
  bootCounter();  //ブート回数をEEPROMに保存、1023まで書き込み確認

  boot_count_tx();  //ブート回数の暗号化とセキュリティチップへ送信

  uint16_t securityChip_data = securityChip_rev();  //セキュリティチップから暗号化されたキーコードを受信

  Serial.print("securityChip_data(HEX) = ");
  Serial.println(securityChip_data, HEX);

  //キーコードの復号化
  uint16_t decryptedData = xorEncryptDecrypt(securityChip_data);

  //セキュリティ認証の状態
  //復号化したキーコードとメインMPUが持っているキーコードテーブルを認証する
  int AUTH_STATUS = check_securityKey(decryptedData);


  if(AUTH_STATUS == AUTH_STATUS_OK){
    Serial.println("AUTH_STATUS_OK");
  }else if(AUTH_STATUS == AUTH_STATUS_FAIL){
    Serial.println("AUTH_STATUS_FAIL");
  }

  /*
  //セキュリティ認証OK
  while(AUTH_STATUS == AUTH_STATUS_OK){
    Serial.println("AUTH_STATUS_OK");
    while(1){
      digitalWrite(LED,0);
      delay(900);
      digitalWrite(LED,1);
      delay(100);
    }
  }
  */

  /*
  //セキュリティ認証FAIL
  while(AUTH_STATUS == AUTH_STATUS_FAIL){
    Serial.println("AUTH_STATUS_FAIL");
    while(1){
      digitalWrite(LED,0);
      delay(400);
      digitalWrite(LED,1);
      delay(100);
    }
  }
  */


}
