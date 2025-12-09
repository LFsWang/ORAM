import re
import csv

def convert(input_filename, output_filename):
    """
    讀取 log 檔案，提取 Stash size 與 opnum，
    並依據 Stash size 彙整 opnum 後輸出至 CSV。
    """
    
    # 建立一個字典來儲存資料
    # 結構: { stash_size_int: [opnum_int, opnum_int, ...] }
    data_map = {}

    # 定義正規表達式模式
    # \s* 代表可以有任意數量的空白
    # (\d+) 代表抓取數字
    pattern = re.compile(r'Stash size\s*=\s*(\d+).*?opnum\s*=\s*(\d+)')

    try:
        # 讀取輸入檔案
        with open(input_filename, 'r', encoding='utf-8') as f_in:
            for line in f_in:
                # 搜尋每一行是否符合模式
                match = pattern.search(line)
                if match:
                    # 抓取出數字字串並轉為整數
                    size = int(match.group(1))
                    opnum = int(match.group(2))

                    # 如果這個 size 還沒出現過，初始化一個空 list
                    if size not in data_map:
                        data_map[size] = []
                    
                    # 將 opnum 加入該 size 的 list 中
                    data_map[size].append(opnum)

        # 寫入輸出檔案
        with open(output_filename, 'w', newline='', encoding='utf-8') as f_out:
            writer = csv.writer(f_out)
            
            # 取得所有的 key (stash size) 並進行排序 (由小到大)
            sorted_sizes = sorted(data_map.keys())

            for size in sorted_sizes:
                # 組合要寫入的列: [size, opnum1, opnum2, ...]
                # data_map[size] 是一個 list，我們將其展開合併
                row = [size] + data_map[size]
                writer.writerow(row)
                
        print(f"轉換完成！已將結果存入 {output_filename}")

    except FileNotFoundError:
        print(f"錯誤: 找不到檔案 '{input_filename}'")
    except Exception as e:
        print(f"發生未預期的錯誤: {e}")

# --- 測試與執行區塊 ---
if __name__ == "__main__":
    # 這裡是範例，您可以修改這裡的檔案名稱，或是直接在別的腳本 import convert
    input_file = 'logs/Z3.log'  # 請確認您的來源檔案名稱
    output_file = 'Z3.csv'     # 輸出的檔案名稱
    
    # 為了測試方便，如果沒有來源檔案，我先建立一個假檔案來示範
    import os
    if not os.path.exists(input_file):
        print(f"找不到檔案 {input_file}")
        exit(1)
        
    # 呼叫函數
    convert(input_file, output_file)