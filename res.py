import re
import matplotlib.pyplot as plt
import os
import numpy as np  # 新增 numpy 用於線性擬合計算

def readLogDir(dirpath):
    """
    讀取指定目錄下的所有 .log 檔案，並回傳 {檔名: 路徑} 的字典。
    檔名會依照字典順序 (Alphabetical Order) 排序，確保圖表圖例整齊。
    """
    log_dict = {}
    if not os.path.exists(dirpath):
        print(f"錯誤: 找不到目錄 {dirpath}")
        return log_dict

    print(f"正在掃描目錄: {dirpath} ...")
    
    # 1. 先取得所有 .log 檔案
    files = [f for f in os.listdir(dirpath) if f.endswith(".log")]
    
    # 2. 進行排序 (這是關鍵，確保放入字典的順序是固定的)
    files.sort() 
    
    # 3. 依序放入字典 (Python 3.7+ 保證字典會記住這個插入順序)
    for filename in files:
        full_path = os.path.join(dirpath, filename)
        log_dict[filename] = full_path
    
    return log_dict

def _parse_single_log(file_path, use_final=False):
    """
    內部 Helper 函數：負責讀取單一 Log 檔並回傳排序後的 x, y 資料
    
    Args:
        file_path (str): 檔案路徑
        use_final (bool): 是否只讀取 "Stash size distribution for" 之後的資料
    """
    x_data = []
    y_data = []
    
    # 若 use_final 為 True，預設不收集資料 (直到看到標頭)
    # 若 use_final 為 False，預設一開始就收集
    collecting = not use_final 

    if not os.path.exists(file_path):
        print(f"錯誤: 找不到檔案 {file_path}")
        return None, None

    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            for line in f:
                # 處理 use_final 邏輯
                if use_final and "Stash size distribution for" in line:
                    # 看到標頭，開始收集。
                    # 如果之前已經收集過 (例如 Warm-up)，這裡清空之前的數據，只保留最後一次的結果
                    collecting = True
                    x_data = [] 
                    y_data = []
                    continue

                if collecting:
                    # 使用正規表達式尋找模式: "Stash size [數字]: [數字] times"
                    match = re.search(r'Stash size (\d+):\s+(\d+)\s+times', line)
                    if match:
                        size = int(match.group(1))
                        times = int(match.group(2))
                        x_data.append(size)
                        y_data.append(times)
                        
    except Exception as e:
        print(f"讀取 {file_path} 時發生錯誤: {e}")
        return None, None

    if not x_data:
        msg = f"警告: {file_path} 中未找到符合格式的資料。"
        if use_final:
            msg += " (已開啟 use_final 模式，請確認 log 是否包含 'Stash size distribution for')"
        print(msg)
        return None, None

    # 確保資料按 X 軸 (Stash size) 排序
    sorted_pairs = sorted(zip(x_data, y_data))
    x_sorted, y_sorted = zip(*sorted_pairs)
    return x_sorted, y_sorted

def make_result(log_files_dict, title, output_png, xmax=None, ymax=None, use_final=False):
    """
    讀取多個 log 檔案並產生比較圖表 (png)。
    
    Args:
        log_files_dict (dict): 包含標籤與路徑的字典。
        title (str): 圖表標題
        output_png (str): 輸出的 PNG 檔案名稱
        xmax (float/int, optional): X 軸最大值. Defaults to None.
        ymax (float/int, optional): Y 軸最大值. Defaults to None.
        use_final (bool): 是否只抓取 "Stash size distribution for" 之後的數據. Defaults to False.
    """
    
    # 建立畫布
    plt.figure(figsize=(10, 6))
    
    has_valid_data = False
    
    # 遍歷字典，依序讀取檔案並繪製線條
    # 注意: Python 3.7+ 這裡的遍歷順序 = 插入字典的順序
    for label, file_path in log_files_dict.items():
        # print(f"正在處理: {label} ({file_path}) ...")
        
        x_data, y_data = _parse_single_log(file_path, use_final)
        
        if x_data and y_data:
            plt.plot(x_data, y_data, marker='o', linestyle='-', label=label, alpha=0.8)
            has_valid_data = True
        else:
            print(f"跳過 {label}: 無有效資料")

    if not has_valid_data:
        print("錯誤: 沒有任何有效資料可供繪圖。")
        plt.close()
        return

    # 設定通用圖表屬性
    plt.yscale('log')
    plt.title(title, fontsize=16)
    plt.xlabel('Stash Size', fontsize=12)
    plt.ylabel('Times (Log Scale)', fontsize=12)
    plt.grid(True, which="both", ls="--", alpha=0.5)

    if xmax is not None:
        plt.xlim(right=xmax)
        plt.xlim(left=0) 
        
    if ymax is not None:
        plt.ylim(top=ymax)

    plt.legend()
    plt.tight_layout()
    plt.savefig(output_png, dpi=300)
    plt.close()
    
    print(f"圖表已成功產生並儲存為: {output_png}")

def make_result_rate(log_files_dict, title, output_png, xmax=None, ymax=None, use_final=False):
    """
    讀取多個 log 檔案並產生「機率分布」比較圖表 (png)。
    Y 軸為該 Stash size 出現次數佔總次數的比例 (Probability)。
    """
    
    # 建立畫布
    plt.figure(figsize=(10, 6))
    
    has_valid_data = False
    
    # 遍歷字典，依序讀取檔案並繪製線條
    for label, file_path in log_files_dict.items():
        x_data, y_data = _parse_single_log(file_path, use_final)
        
        if x_data and y_data:
            # 計算總次數 (用於正規化)
            total_count = sum(y_data)
            
            if total_count > 0:
                # 將次數轉換為機率 (Rate)
                y_rate = [y / total_count for y in y_data]
                
                plt.plot(x_data, y_rate, marker='o', linestyle='-', label=label, alpha=0.8)
                has_valid_data = True
            else:
                print(f"跳過 {label}: 數據總和為 0")
        else:
            print(f"跳過 {label}: 無有效資料")

    if not has_valid_data:
        print("錯誤: 沒有任何有效資料可供繪圖。")
        plt.close()
        return

    # 設定通用圖表屬性
    plt.yscale('log')
    plt.title(title, fontsize=24)
    plt.xlabel('Stash Size', fontsize=16)
    plt.ylabel('Probability (Log Scale)', fontsize=16) # Y 軸標籤改為機率
    plt.grid(True, which="both", ls="--", alpha=0.5)

    if xmax is not None:
        plt.xlim(right=xmax)
        plt.xlim(left=0) 
        
    if ymax is not None:
        plt.ylim(top=ymax)
    else:
        # 如果沒有指定 ymax，對於機率圖來說，最大值通常是 1 (10^0)
        # 這裡不強制設為 1，讓 matplotlib 自動決定，但通常會在 1 附近
        pass

    plt.legend()
    plt.tight_layout()
    plt.savefig(output_png, dpi=300)
    plt.close()
    
    print(f"圖表已成功產生並儲存為: {output_png}")

def report_slope(log_files_dict, output_png=None, max_stash=10, use_final=False):
    """
    計算並列印出 Stash size 1 到 max_stash 之間的機率分布斜率以及 R^2。
    計算方式：對 (x, log10(Probability)) 進行線性擬合。
    
    Args:
        log_files_dict (dict): 包含標籤與路徑的字典
        output_png (str, optional): 若提供檔名，將繪製斜率比較長條圖 (x=log name, y=abs(slope))
        max_stash (int): 計算斜率時採納的最大 Stash size (包含此值)
        use_final (bool): 是否只抓取 "Stash size distribution for" 之後的數據
    """
    
    print("=" * 60)
    print(f"斜率與 R^2 報告 (Fit on Log10(Probability), Range: 1~{max_stash})")
    print("=" * 60)

    plot_labels = []
    plot_slopes = []

    for label, file_path in log_files_dict.items():
        x_data, y_data = _parse_single_log(file_path, use_final)
        
        if not x_data or not y_data:
            print(f"[report_slope] log {label} : 無有效資料")
            continue

        total_count = sum(y_data)
        if total_count == 0:
            continue

        # 準備擬合用的數據 (只取 1 ~ max_stash 且次數 > 0 的點)
        xs = []
        log_ys = []
        
        for x, y in zip(x_data, y_data):
            if 1 <= x <= max_stash and y > 0:
                xs.append(x)
                # 使用 log10 是因為圖表通常是 10^-1, 10^-2 刻度
                prob = y / total_count
                log_ys.append(np.log10(prob))
        
        if len(xs) < 2:
            print(f"[report_slope] log {label} : 有效數據點不足 (<2) 於範圍 1~{max_stash}，無法計算斜率")
            continue

        # 進行線性擬合 (1次多項式)
        # polyfit 回傳 [斜率, 截距]
        slope, intercept = np.polyfit(xs, log_ys, 1)
        
        # 計算 R^2
        predicted_ys = [slope * x + intercept for x in xs]
        ss_res = sum((y_true - y_pred) ** 2 for y_true, y_pred in zip(log_ys, predicted_ys))
        mean_y = np.mean(log_ys)
        ss_tot = sum((y_true - mean_y) ** 2 for y_true in log_ys)
        
        if ss_tot == 0:
            r2 = 1.0 
        else:
            r2 = 1 - (ss_res / ss_tot)
        
        print(f"[report_slope] log {label} stash data (1~{max_stash}) slope = {slope:.6f}, r2={r2:.6f}")

        # 收集數據用於繪圖 (取絕對值)
        plot_labels.append(label)
        plot_slopes.append(abs(slope))

    # 如果有指定 output_png 且有資料，則繪製圖表
    if output_png and plot_labels:
        plt.figure(figsize=(10, 6))
        
        # 繪製長條圖
        bars = plt.bar(plot_labels, plot_slopes, color='skyblue', edgecolor='black', alpha=0.7)
        
        # 標示數值在 Bar 上方
        for bar in bars:
            height = bar.get_height()
            plt.text(bar.get_x() + bar.get_width()/2., height,
                     f'{height:.2f}',
                     ha='center', va='bottom')

        plt.xlabel('Log Name', fontsize=12)
        plt.ylabel('Slope (Absolute Value)', fontsize=12)
        plt.title(f'Slope Comparison (Abs) - Range: 1~{max_stash}', fontsize=14)
        
        # 如果 Label 很長，旋轉 45 度避免重疊
        plt.xticks(rotation=45, ha='right')
        
        plt.grid(axis='y', linestyle='--', alpha=0.5)
        plt.tight_layout()
        plt.savefig(output_png, dpi=300)
        plt.close()
        
        print(f"斜率比較圖表已儲存為: {output_png}")


def report_rate(log_files_dict, max_stash=10, use_final=False):
    """
    計算每一個資料，stash > max_stash 的累積機率 (Cumulative Probability)，並列印報告。
    例如 max_stash=5，則計算 Stash size 5 以上的累積機率總和。
    """
    
    print("=" * 60)
    print(f"累積機率報告 (Range: 1~{max_stash})")
    print("=" * 60)

    plot_labels = []
    plot_slopes = []

    for label, file_path in log_files_dict.items():
        x_data, y_data = _parse_single_log(file_path, use_final)
        
        if not x_data or not y_data:
            print(f"[report_slope] log {label} : 無有效資料")
            continue

        total_count = sum(y_data)
        sub = 0
        for x, y in zip(x_data, y_data):
            if x <= max_stash:
                sub += y

        if total_count == 0:
            continue
        # 用科學記號輸出
        rate = (total_count - sub) / total_count
        ## we output N x 10^-6
        N = rate * 1e6
        # print(f"[report_rate] log {label} stash <= {max_stash} cumulative probability = {N:.6f} x 10^-3")
        # print(f"[report_rate] log {label} stash <= {max_stash} cumulative probability = {sub}/{total_count} = {sub/total_count:.6e}")
        # for latex table
        print(f"{label} & {N:.3f} \\\\")

# ==========================================
# 測試區塊
# ==========================================
def fig1():
    test_dir = "./logs/figZ3-6/"
    logs = readLogDir(test_dir)
    make_result(
        log_files_dict=logs,
        title="Stash Distribution (Counts)",
        output_png= test_dir+"figZ3-6_counts.png",
        xmax=25,
        use_final=True
    )
    make_result_rate(
        log_files_dict=logs,
        title="Stash Distribution for Uniform Z (Probability)",
        output_png= test_dir+"figZ3-6_rate.png",
        xmax=25,
        use_final=True
    )
    # report_slope(
    #     log_files_dict=logs,
    #     max_stash=10,
    #     use_final=True
    # )


def fig2():
    test_dir = "./logs/figZ45-Zmid/"
    logs = readLogDir(test_dir)
    make_result(
        log_files_dict=logs,
        title="Stash Distribution (Counts)",
        output_png= test_dir+"figZ45-Zmid_counts.png",
        xmax=25,
        use_final=True
    )
    make_result_rate(
        log_files_dict=logs,
        title="",
        output_png= test_dir+"figZ45-Zmid_rate.png",
        xmax=25,
        use_final=True
    )
def tab3():
    test_dir = "./logs/tableZ5x0-14/"
    logs = readLogDir(test_dir)
    make_result_rate(
        log_files_dict=logs,
        title="Stash Distribution (Probability)",
        output_png= test_dir+"tableZ5x0-14_counts.png",
        xmax=25,
        use_final=True
    )
    report_rate(
        log_files_dict=logs,
        max_stash=10,
        use_final=True
    )

if __name__ == "__main__":
    fig1()
    fig2()
    tab3()

    exit(0) #pass, left for manual testing
    
    # 測試目錄 (請自行修改為你的測試目錄)
    # 1. 測試 readLogDir
    test_dir = "./logs/Test3/"
    logs_from_dir = readLogDir(test_dir)
    print("讀取到的檔案:", logs_from_dir)

    # 2. 關於合併字典的範例 (回答你的問題)
    # Python 3.9+ 可以用 | 運算子，或使用 {**d1, **d2}
    more_logs = {"Z5": "some/path/file.log"}
    combined_logs = {**logs_from_dir, **more_logs} 
    # 或者: logs_from_dir.update(more_logs)

    # 2. 測試 make_result (原始次數圖)
    make_result(
        log_files_dict=logs_from_dir,
        title="Stash Distribution (Counts)",
        output_png="stash_counts.png",
        xmax=5,
        use_final=True
    )
    
    # 3. 測試 make_result_rate (機率圖)
    make_result_rate(
        log_files_dict=logs_from_dir,
        title="Stash Distribution for Uniform Z (Probability)",
        output_png="stash_rate.png",
        xmax=25,
        use_final=True
    )