//疑似メインMPU、ESP32-WROOM-32

#include <Arduino.h>
#include "key_table.h"  //キーコードの入ったテーブルデータ[1024]
#include <EEPROM.h>

#define EEPROM_SIZE 1024  //EEPROMのサイズ（バイト単位）、ESP32で必要

#define AUTH_STATUS_OK 1
#define AUTH_STATUS_FAIL 0


const int LED = 2;  // LEDが接続されているピン番号
uint8_t boot_eeprom_address = 0; // EEPROMのアドレス

// XORキー
const uint16_t XOR_KEY = 0x99;  //プロジェクトによってXORキーは変更する、セキュリティチップとは同じ値にすること

// 暗号化関数
uint16_t xorEncryptDecrypt(uint16_t input) {
  return input ^ XOR_KEY;  // XOR演算
}


//ブート毎にEEPROMにブート回数をカウントする。
//255までいったら0にリセット
void bootCounter(){
  // EEPROMから1バイト読む
  uint8_t boot_eeprom_counter = EEPROM.read(boot_eeprom_address); 

  //ブート回数は255まで行ったら0にリセットされます。
  //キーコードのテーブルは1024まで用意していますが、
  //セキュリティチップの有用性確認のため（と簡単にコードを作るために）
  //1バイト分だけ管理しています。
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


//EEPROMの内容をシリアルに送信（デバッグ向け関数)
void bootCounter_read(){
  // EEPROMから1バイト読み込む
  uint8_t value = EEPROM.read(boot_eeprom_address); 

  // 確認のため読み出した値をシリアルモニタに表示
  Serial.print("boot counter value: ");
  Serial.println(value, DEC);

  //Serial.print("key_table[lowByte] : ");
  //Serial.println(key_table[lowByte], DEC);  
}



//ブート回数をセキュリティチップに送信
void boot_count_tx(){
  // EEPROMから1バイト読み込む
  uint8_t value = EEPROM.read(boot_eeprom_address); 

  //暗号化
  uint16_t encryptedData = xorEncryptDecrypt(value);
  //Serial.print("encryptedData : ");
  //Serial.println(encryptedData);

  //暗号化したブート回数をセキュリティチップに送信
  Serial2.write(encryptedData);
}


//セキュリティチップからキーコードを受信する
uint16_t securityChip_rev(){
  // データが受信されるまで待機
  while (Serial2.available() == 0) {
  }
  
  uint8_t value =  Serial2.read();
  return value;
}


//セキュリティ認証の検証
int check_securityKey(uint16_t decryptedData){

  // EEPROMからブート回数を読み出す
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
  bootCounter();  //ブート回数をEEPROMに保存、255まで

  boot_count_tx();  //ブート回数の暗号化とセキュリティチップへ送信

  uint16_t securityChip_data = securityChip_rev();  //セキュリティチップから暗号化されたキーコードを受信

  Serial.print("securityChip_data(HEX) = ");  //暗号化されたキーコードを表示（デバッグ用）
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
}
