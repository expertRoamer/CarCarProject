import serial
import os
import re  # 導入正則表達式，用來過濾垃圾字元
import time

# 設定路徑
current_dir = os.path.dirname(os.path.abspath(__file__))
file_path = os.path.join(current_dir, "calibration_data.txt")

# 設定 Serial
COM_PORT = 'COM9'
BAUD_RATE = 9600

# 用來暫存數據的字典
calib_results = {
    "WHITE": "[]",
    "BLACK": "[]"
}

def clean_data(raw_line):
    """
    過濾掉 ANSI 控制碼 (如 [10;19H)
    並只抓取包含 [ ... ] 的部分
    """
    # 移除 ANSI Escape Codes
    ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
    clean_line = ansi_escape.sub('', raw_line)

    # 尋找符合 [數字, 數字...] 格式的內容
    match = re.search(r'\[[\d\s,.]+\]', clean_line)
    return match.group(0) if match else None

def save_to_file():
    with open(file_path, "w", encoding="utf-8") as f:
        f.write(f"WHITE:{calib_results['WHITE']}\n")
        f.write(f"BLACK:{calib_results['BLACK']}\n")
    print(f"\n--- 檔案已存檔 ---")
    print(f"目前存下的白值: {calib_results['WHITE']}")
    print(f"目前存下的黑值: {calib_results['BLACK']}")

try:
    ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
    print(f"已連接 {COM_PORT}。")
    print("指令：[w]=紀錄白值, [b]=紀錄黑值, [q]=退出")

    while True:
        # 使用 input 之前先清空一下緩衝，避免誤讀
        cmd = input("\n請輸入指令 (w/b/q): ").strip().lower()

        if cmd == 'q':
            break

        if cmd in ['w', 'b']:
            print("正在捕捉數據...")
            ser.reset_input_buffer() # 清除積壓的舊數據
            time.sleep(0.2)          # 給 Arduino 時間送出一行乾淨的

            found = False
            # 嘗試讀取 10 行，直到抓到正確格式
            for _ in range(10):
                raw_line = ser.readline().decode('utf-8', errors='ignore').strip()
                result = clean_data(raw_line)

                if result:
                    label = "WHITE" if cmd == 'w' else "BLACK"
                    calib_results[label] = result
                    save_to_file()
                    found = True
                    break

            if not found:
                print("錯誤：無法從原始數據中解析出陣列格式。")
                print(f"最後讀到的原始數據為: {raw_line}")

except Exception as e:
    print(f"發生錯誤: {e}")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()