#2025/03/30
#同人ハードウェアの不正コピーの対策のためのキーテーブル出力プログラム
# 任意のシード値を使用してヘッダーファイルを生成
# ヘッダーファイルはキーテーブルとして他のプログラムから使います


import random


#シード値を指定、整数か文字列どちらでもOK
seed_value = 19830715



def generate_key_table(seed):
    # シード値を設定
    random.seed(seed)

    # 1024個の乱数を生成（0から255の範囲）
    key_table = [random.randint(0, 255) for _ in range(1024)]

    return key_table

def generate_header_file(seed):
    key_table = generate_key_table(seed)

    # ヘッダーファイルの内容
    header_content = """#ifndef KEY_TABLE_H
#define KEY_TABLE_H

// キーテーブルの定義
unsigned char key_table[1024] = {
"""
    
    # キーテーブルの値をヘッダー形式で追加
    for value in key_table:
        header_content += f"    {value},\n"

    header_content += """};

#endif // KEY_TABLE_H
"""

    return header_content



header_file_content = generate_header_file(seed_value)

# ヘッダーファイル内容を出力
with open('key_table.h', 'w') as f:
    f.write(header_file_content)

print("ヘッダーファイル 'key_table.h' が生成されました。")

