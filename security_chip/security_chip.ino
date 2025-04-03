//疑似セキュリティチップ、Arduino Uno

#include <Arduino.h>
#include "key_table.h"  // key_table.hをインクルード


const int LED = 13;  // LEDが接続されているピン番号

// XOR鍵（任意の値を使用）
const byte XOR_KEY = 0x55;  // 例: 0x55のバイトを鍵として使用

// 暗号化関数
byte xorEncryptDecrypt(byte input) {
  return input ^ XOR_KEY;  // XOR演算
}


//MPUから暗号化されたブート回数を受け取る
uint8_t mpu_rev(){
  // データが受信されるまで待機
  while (Serial.available() == 0) {
  }
  
  // 受信したデータを戻り値にする
  return Serial.read();

}


void setup() {
  Serial.begin(115200);      // シリアル通信の初期化

  pinMode(LED, OUTPUT); // LEDピンを出力モードに設定
}


void loop() {
  delay(100);

  while(1){
    uint8_t mpu_data = mpu_rev();

    // 復号化
    byte decryptedData = xorEncryptDecrypt(mpu_data);

    // キーテーブルの暗号化する
    byte keytable_xor = xorEncryptDecrypt(key_table[decryptedData]);

    Serial.write(keytable_xor); //復号化した起動回数をキーテーブルに使う
  }
}
