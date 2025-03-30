#include <Arduino.h>
#include "key_table.h"  // key_table.hをインクルード
#include <EEPROM.h>


const int ledPin = 13;  // LEDが接続されているピン番号（通常は13番ピン）
uint8_t boot_eeprom_address_L = 0; // EEPROMのアドレス、下位
uint8_t boot_eeprom_address_H= 1; // EEPROMのアドレス、上位

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

  // 上位バイトと下位バイトに分けてEEPROMに保存
  byte lowByteNew = boot_eeprom_counter & 0xFF; // 下位バイト
  byte highByteNew = (boot_eeprom_counter >> 8) & 0xFF; // 上位バイト

  // 新しい値をEEPROMに保存
  EEPROM.write(boot_eeprom_address_L, lowByteNew);  // 下位バイト
  EEPROM.write(boot_eeprom_address_H, highByteNew); // 上位バイト
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
  Serial.begin(9600);      // シリアル通信の初期化
  pinMode(ledPin, OUTPUT); // LEDピンを出力モードに設定
}

void loop() {
  delay(1000);
  bootCounter();  //ブート回数をEEPROMに保存
  while(1){
    bootCounter_read(); //EEPROMのブート回数を表示する
    delay(1000);
  }
}
