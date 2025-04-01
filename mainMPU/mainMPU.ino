//疑似メインMPU、ESP32-WROOM-32

#include <Arduino.h>
#include "key_table.h"  // key_table.hをインクルード
#include <EEPROM.h>

#define EEPROM_SIZE 1024  // EEPROMのサイズ（バイト単位）、ESP32で必要


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
  Serial.print("1\n");
  byte lowByte = EEPROM.read(boot_eeprom_address_L);  // 下位バイト
  byte highByte = EEPROM.read(boot_eeprom_address_H); // 上位バイト

  // 16ビット値を結合
  Serial.print("3\n");
  uint16_t boot_eeprom_counter = (highByte << 8) | lowByte;

  if(boot_eeprom_counter >= 1023){
    boot_eeprom_counter = 0;
  }else{
    boot_eeprom_counter += 1;
  }

  // 上位バイトと下位バイトに分けてEEPROMに保存
  Serial.print("3\n");
  byte lowByteNew = boot_eeprom_counter & 0xFF; // 下位バイト
  byte highByteNew = (boot_eeprom_counter >> 8) & 0xFF; // 上位バイト

  // 新しい値をEEPROMに保存
  Serial.print("4\n");
  EEPROM.write(boot_eeprom_address_L, lowByteNew);  // 下位バイト
  EEPROM.write(boot_eeprom_address_H, highByteNew); // 上位バイト

  // EEPROMへの書き込みを確定
  EEPROM.commit();

  Serial.print("5\n");

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
  Serial.println(boot_eeprom_counter);
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

  while(1){
    bootCounter_read(); //EEPROMのブート回数を表示する


    //起動回数＋起動回数を反転⇢XORで暗号化

    static uint8_t originalData = 0;

    originalData += 1;

    uint8_t notData = ~originalData;  //NOTで反転

    // 暗号化
    byte encryptedData = xorEncryptDecrypt(originalData);

    // 復号化
    byte decryptedData = xorEncryptDecrypt(encryptedData);

    Serial.print("originalData: ");
    Serial.println(originalData);

    Serial.print("notData: ");
    Serial.println(notData);

    Serial.print("encryptedData: ");
    Serial.println(encryptedData);

    Serial.print("decryptedData: ");
    Serial.println(decryptedData);
    



    delay(1000);
  }
}
