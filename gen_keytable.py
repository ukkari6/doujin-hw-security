# 2025/03/30
# 同人ハードウェアの不正コピー対策用：キーコードテーブル生成プログラム
# コマンドライン引数で「テーブルサイズ」と「シード値」を指定可能

#　使い方
# デフォルト（1024個, シード19830715）
#python3 gen_keytable.py

# テーブル数512、デフォルトシード
#python3 gen_keytable.py 512

# テーブル数256、シード "my-secret-seed"
#python3 gen_keytable.py 256 "my-secret-seed"

# テーブル数128、シード 12345（整数でもOK）
#python3 gen_keytable.py 128 12345


import random
import sys

# --- デフォルト設定 ---
default_table_size = 1024
default_seed_value = 19830715

# --- コマンドライン引数の処理 ---
# 使い方: python3 gen_keytable.py [table_size] [seed]
try:
    # 引数1: テーブルサイズ
    table_size = int(sys.argv[1]) if len(sys.argv) > 1 else default_table_size
    if table_size <= 0:
        raise ValueError("テーブルサイズは正の整数で指定してください。")

    # 引数2: シード値（整数 or 文字列OK）
    seed_value = sys.argv[2] if len(sys.argv) > 2 else default_seed_value

except ValueError as ve:
    print(f"引数エラー: {ve}")
    sys.exit(1)

def generate_key_table(seed, size):
    random.seed(seed)
    return [random.randint(0, 255) for _ in range(size)]

def generate_header_file(seed, size):
    key_table = generate_key_table(seed, size)

    header_content = f"""#ifndef KEY_TABLE_H
#define KEY_TABLE_H

// キーテーブルの定義（サイズ: {size}, シード: {repr(seed)}）
unsigned char key_table[{size}] = {{
"""

    for i, value in enumerate(key_table):
        if i % 8 == 0:
            header_content += "    "
        header_content += f"{value}, "
        if (i + 1) % 8 == 0:
            header_content += "\n"

    header_content += f"""}};

#endif // KEY_TABLE_H
"""

    return header_content

# --- 実行 ---
header_file_content = generate_header_file(seed_value, table_size)

with open('key_table.h', 'w') as f:
    f.write(header_file_content)

print(f"ヘッダーファイル 'key_table.h' を生成しました（サイズ: {table_size}, シード: {repr(seed_value)}）")

