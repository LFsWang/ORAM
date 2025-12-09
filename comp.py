import csv
import matplotlib
# 設定後端，支援遠端無螢幕執行
matplotlib.use('Agg') 
import matplotlib.pyplot as plt
import numpy as np

def loadCSV(filename):
    """
    讀取 CSV 檔案。
    格式: Stash_Size, opnum1, opnum2, ...
    回傳: 一個字典 {size: [opnum_list]}
    """
    data = {}
    try:
        with open(filename, 'r', encoding='utf-8') as f:
            reader = csv.reader(f)
            for row in reader:
                if not row: continue
                try:
                    size = int(row[0])
                    opnums = [int(x) for x in row[1:]]
                    data[size] = opnums
                except ValueError:
                    continue 
        return data
    except FileNotFoundError:
        print(f"錯誤: 找不到檔案 {filename}")
        return {}

def analysisMany(dataset_list, output_img='swapped_axes_result.png'):
    """
    分析並比較多組資料 (XY軸互換版)。
    X軸: Opnum (Log Scale)
    Y軸: Stash Size
    """
    
    plt.figure(figsize=(10, 8))
    
    # --- 修改點 1: 設定 X 軸為對數座標 ---
    plt.xscale('log')

    for entry in dataset_list:
        data = entry[0]
        name = entry[1]
        color = entry[2]

        if not data:
            continue

        # 準備數據
        mean_size = []   # 這是原本的 X (Size)
        mean_opnum = []  # 這是原本的 Y (Opnum)
        
        # 擬合用原始數據
        fit_size_raw = []
        fit_opnum_raw = []

        sorted_sizes = sorted(data.keys())

        for size in sorted_sizes:
            opnums = data[size]
            for op in opnums:
                fit_size_raw.append(size)
                fit_opnum_raw.append(op)
            
            if opnums:
                avg = np.mean(opnums)
                mean_size.append(size)
                mean_opnum.append(avg)

        # --- 修改點 2: 繪製平均點 (座標互換) ---
        # 傳入參數順序改為 (Opnum, Size) -> (X, Y)
        plt.scatter(mean_opnum, mean_size, color=color, marker='o', s=60, label=f"{name} (Avg)")

        # --- 修改點 3: 繪製擬合直線 (計算邏輯不變，繪圖互換) ---
        if len(fit_size_raw) > 1:
            try:
                # 數學模型依然是: Opnum 隨著 Size 指數成長
                # log(Opnum) = a * Size + b
                log_opnum = np.log10(fit_opnum_raw)
                slope, intercept = np.polyfit(fit_size_raw, log_opnum, 1)
                
                # 產生擬合線的點 (根據 Size 的範圍)
                line_size = np.linspace(min(fit_size_raw), max(fit_size_raw), 100)
                
                # 算出對應的 Opnum
                line_opnum_log = slope * line_size + intercept
                line_opnum = 10 ** line_opnum_log
                
                # 繪圖時互換: X=Opnum, Y=Size
                plt.plot(line_opnum, line_size, color=color, linestyle='--', linewidth=2, label=f"{name} (Fit)")
            except Exception as e:
                print(f"擬合失敗: {e}")

    # --- 圖表修飾 ---
    plt.title('Comparison: Opnum vs Stash Size (Axes Swapped)')
    plt.xlabel('Opnum (Log Scale)') # X 軸標籤變更
    plt.ylabel('Stash Size')        # Y 軸標籤變更
    plt.grid(True, which="both", ls="-", alpha=0.2)
    plt.legend()
    plt.tight_layout()

    plt.savefig(output_img, dpi=300)
    print(f"圖表繪製完成，已儲存為: {output_img}")
    plt.close()

# --- 使用範例 ---
if __name__ == "__main__":
    # 這裡示範如何呼叫
    
    # 1. 讀取兩個不同的 CSV (假設你有 output1.csv 和 output2.csv)
    # 為了示範，我們先讀取同一個檔案兩次，或者您可以換成真實的檔案
    file1 = 'output.csv' 
    file2 = 'result/Z6.csv' 
    file3 = 'result/Z5.csv' 
    file4 = 'result/Z4.csv' 
    file5 = 'result/Z3.csv' 
    
    data1 = loadCSV(file1)
    data2 = loadCSV(file2)
    data3 = loadCSV(file3)
    data4 = loadCSV(file4)
    data5 = loadCSV(file5)

    # 2. 準備輸入結構
    # 格式: [資料, 名稱, 顏色]
    datasets_to_plot = [
        [data1, "Experiment A", "blue"],
        [data2, "Experiment Z6", "#FF0000"],
        [data3, "Experiment Z5", "#AA0000"],
        [data4, "Experiment Z4", "#550000"],
        [data5, "Experiment Z3", "#000000"],
    ]

    # 3. 執行分析
    analysisMany(datasets_to_plot, "multi_analysis.png")