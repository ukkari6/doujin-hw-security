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
const uint16_t XOR_KEY = 0x9988;  // 例: 0x9988のバイトを鍵として使用

// 暗号化関数
uint16_t xorEncryptDecrypt(uint16_t input) {
  return input ^ XOR_KEY;  // XOR演算
}


//ブート毎にEEPROMにブート回数をカウントする。
//1023までいったら0にリセット
void bootCounter(){
  // EEPROMから2バイト読み込む
  uint8_t lowByte = EEPROM.read(boot_eeprom_address_L);  // 下位バイト
  uint8_t highByte = EEPROM.read(boot_eeprom_address_H); // 上位バイト

  // 16ビット値を結合
  uint16_t boot_eeprom_counter = (highByte << 8) | lowByte;

  if(boot_eeprom_counter >= 1023){
    boot_eeprom_counter = 0;
  }else{
    boot_eeprom_counter += 1;
  }
  
  Serial.print("boot_eeprom_counter : ");
  Serial.println(boot_eeprom_counter, DEC);

  //Serial.print("key_table[boot_eeprom_counter] : ");
  //Serial.println(key_table[boot_eeprom_counter], DEC);

  // 上位バイトと下位バイトに分けてEEPROMに保存
  uint8_t lowByteNew = boot_eeprom_counter & 0xFF; // 下位バイト
  uint8_t highByteNew = (boot_eeprom_counter >> 8) & 0xFF; // 上位バイト

  // 新しい値をEEPROMに保存
  EEPROM.write(boot_eeprom_address_L, lowByteNew);  // 下位バイト
  EEPROM.write(boot_eeprom_address_H, highByteNew); // 上位バイト

  // EEPROMへの書き込みを確定
  EEPROM.commit();
}

//EEPROMの内容をシリアルに送信
void bootCounter_read(){
  // EEPROMから2バイト読み込む
  uint8_t lowByte = EEPROM.read(boot_eeprom_address_L);  // 下位バイト
  uint8_t highByte = EEPROM.read(boot_eeprom_address_H); // 上位バイト

  // 16ビット値を結合
  uint16_t boot_eeprom_counter = (highByte << 8) | lowByte;

  // 確認のため読み出した値をシリアルモニタに表示
  //Serial.print("boot counter value: ");
  //Serial.println(boot_eeprom_counter, DEC);

  //Serial.print("key_table[boot_eeprom_counter] : ");
  //Serial.println(key_table[boot_eeprom_counter], DEC);  
}



//ブート回数をセキュリティチップに送信
void boot_count_tx(){
  // EEPROMから2バイト読み込む
  uint8_t lowByte = EEPROM.read(boot_eeprom_address_L);  // 下位バイト
  uint8_t highByte = EEPROM.read(boot_eeprom_address_H); // 上位バイト

  // 16ビット値を結合
  uint16_t boot_eeprom_counter = (highByte << 8) | lowByte;

  //暗号化
  uint16_t encryptedData = xorEncryptDecrypt(boot_eeprom_counter);
  //erial.print("encryptedData : ");
  //Serial.println(encryptedData);

  //暗号化したブート回数を上位と下位に分ける
  uint8_t low =  encryptedData & 0xFF;
  uint8_t high =  encryptedData >> 8;

  //Serial.print("send-low : ");
  //Serial.println(low);
  //Serial.print("send-high : ");
  //Serial.println(high);
  //下位・上位の順番で送信
  Serial2.write(low);
  Serial2.write(high);
}


uint16_t securityChip_rev(){
  // データが受信されるまで待機
  while (Serial2.available() == 0) {
  }
  //下位バイトを受信
  uint8_t lowByte =  Serial2.read();


  // データが受信されるまで待機
  while (Serial2.available() == 0) {
  }
  //下位バイトを受信
  uint8_t highByte =  Serial2.read();

  // 16ビット値を結合
  uint16_t serialChip_data = (highByte << 8) | lowByte;
  return serialChip_data;
}


//セキュリティ認証の検証
int check_securityKey(uint16_t decryptedData){

  // EEPROMから2バイト読み込む
  byte lowByte = EEPROM.read(boot_eeprom_address_L);  // 下位バイト
  byte highByte = EEPROM.read(boot_eeprom_address_H); // 上位バイト

  // 16ビット値を結合
  uint16_t boot_eeprom_counter = (highByte << 8) | lowByte;


  //セキュリティチップから送られてくるデータは、ブート回数をkey_tableで参照してその値が戻ってくるので、
  //メインMPUのkey_table[ブート回数]　== セキュリティチップから戻ってきたデータを復号化したデータにする
  if(decryptedData == key_table[boot_eeprom_counter]){
    return(AUTH_STATUS_OK);
  }else{
    return(AUTH_STATUS_FAIL);
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

  delay(100);
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
