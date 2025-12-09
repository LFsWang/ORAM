import csv
import matplotlib
# 重要：設定為 Agg backend，專門用於沒有螢幕的伺服器環境產生圖檔
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
                    continue # 跳過解析錯誤的行
        return data
    except FileNotFoundError:
        print(f"錯誤: 找不到檔案 {filename}")
        return {}

def analysisSingle(data, output_img='analysis_result.png'):
    """
    分析並繪製圖表，最後儲存為圖片檔案。
    output_img: 輸出的圖片檔名 (例如: result.png)
    """
    if not data:
        print("沒有資料可供分析。")
        return

    all_x = []
    all_y = []
    mean_x = []
    mean_y = []

    sorted_sizes = sorted(data.keys())

    for size in sorted_sizes:
        opnums = data[size]
        for op in opnums:
            all_x.append(size)
            all_y.append(op)
        
        if opnums:
            avg = np.mean(opnums)
            mean_x.append(size)
            mean_y.append(avg)

    # 設定畫布大小
    plt.figure(figsize=(10, 6))

    # A. 繪製原始資料散點
    plt.scatter(all_x, all_y, color='gray', alpha=0.5, s=20, label='Raw Data')

    # B. 繪製平均值
    plt.scatter(mean_x, mean_y, color='red', marker='x', s=100, linewidth=2, label='Average')

    # C. 設定 Y 軸為對數座標
    plt.yscale('log')

    # D. 繪製擬合直線 (對 Log(y) 做線性擬合)
    if len(all_x) > 1:
        log_y = np.log10(all_y)
        slope, intercept = np.polyfit(all_x, log_y, 1)
        
        fit_x = np.linspace(min(all_x), max(all_x), 100)
        fit_y_log = slope * fit_x + intercept
        fit_y = 10 ** fit_y_log
        
        # 顯示回歸方程式在圖例中 (選用)
        equation = f'Fit: log(y)={slope:.4f}x + {intercept:.2f}'
        plt.plot(fit_x, fit_y, color='blue', linestyle='--', linewidth=2, label=equation)

    plt.title('Analysis of Stash Size vs Opnum (Log Scale)')
    plt.xlabel('Stash Size')
    plt.ylabel('Opnum (Log Scale)')
    plt.grid(True, which="both", ls="-", alpha=0.2)
    plt.legend()
    plt.tight_layout()

    # --- 這裡改為儲存檔案 ---
    plt.savefig(output_img, dpi=300) # dpi=300 讓圖片更清晰
    print(f"圖表繪製完成，已儲存為: {output_img}")
    
    # 清除目前的圖表，釋放記憶體 (在迴圈或大量繪圖時很重要)
    plt.close()

if __name__ == "__main__":
    input_csv = './result/Z3.csv'
    output_png = './result/Z3.png'
    
    # 1. 讀取
    dataset = loadCSV(input_csv)
    
    # 2. 分析並存檔
    analysisSingle(dataset, output_png)