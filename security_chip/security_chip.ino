//疑似セキュリティチップ、Arduino Uno

#include <Arduino.h>
#include "key_table.h"  // key_table.hをインクルード


const int LED = 13;  // LEDが接続されているピン番号

// XOR鍵（任意の値を使用）
const uint16_t XOR_KEY = 0x99;  // 例: 0x9988のバイトを鍵として使用

// 暗号化関数
uint16_t xorEncryptDecrypt(uint16_t input) {
  return input ^ XOR_KEY;  // XOR演算
}


//MPUから暗号化されたブート回数を受け取る
uint16_t mpu_rev(){
  // データが受信されるまで待機
  while (Serial.available() == 0) {
  }
  //下位バイトを受信
  uint8_t value =  Serial.read();

  return value;
}


//MPUにキーコードを暗号化して送信する
void mpu_send(uint16_t mpu_data){

  // 復号化
  uint16_t decryptedData = xorEncryptDecrypt(mpu_data);
  //Serial.print("decryptedData = ");
  //Serial.println(decryptedData, DEC);

  //キーテーブルの値を表示
  //uint16_t tx = key_table[decryptedData];
  //Serial.print("tx = ");
  //Serial.println(tx, DEC);

  // キーテーブルの取り出しと暗号化をする
  uint16_t keytable_xor = xorEncryptDecrypt(key_table[decryptedData]);
  //Serial.print("keytable_xor = ");
  //Serial.println(keytable_xor, DEC);

  Serial.write(keytable_xor);
}




void setup() {
  Serial.begin(115200);      // シリアル通信の初期化
  pinMode(LED, OUTPUT); // LEDピンを出力モードに設定
}


void loop() {
  delay(100);

  while(1){
    uint16_t mpu_data = mpu_rev();  //MPUから受信
    mpu_send(mpu_data); //キーテーブルからキーコードを取り出しして送信
  }
}
