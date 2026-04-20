import csv
import heapq

# 定義絕對方向的數字映射 (順時針)
DIR_MAP = {'N': 0, 'E': 1, 'S': 2, 'W': 3}
# 相對方向的文字映射
REL_MAP = {0: 'f', 1: 'r', 2: 'b', 3: 'l'}
# 座標移動偏移量 (用來計算曼哈頓距離)
MOVE_OFFSET = {0: (0, 1), 1: (1, 0), 2: (0, -1), 3: (-1, 0)}

def analyze_best_target(csv_filepath, start_idx, start_facing='N', cost_weights=None):
    """
    計算從起點出發，前往哪個死路的「單位時間得分」最高。
    """
    if cost_weights is None:
        # 預設轉向時間成本 (可由外部動態傳入調整)
        cost_weights = {'f': 1.0, 'l': 1.5, 'r': 1.5, 'b': 3.0}

    # 1. 讀取地圖
    graph = {}
    with open(csv_filepath, 'r', encoding='utf-8') as file:
        reader = csv.DictReader(file)
        for row in reader:
            idx = int(row['index'])
            neighbors = {}
            if row.get('North'): neighbors['N'] = int(row['North'])
            if row.get('South'): neighbors['S'] = int(row['South'])
            if row.get('West'):  neighbors['W'] = int(row['West'])
            if row.get('East'):  neighbors['E'] = int(row['East'])
            graph[idx] = neighbors

    # 2. BFS 探索地圖，賦予所有節點 (X, Y) 座標
    coords = {start_idx: (0, 0)}
    queue = [start_idx]
    visited_map = {start_idx}
    
    while queue:
        curr = queue.pop(0)
        for d_str, nxt in graph[curr].items():
            if nxt not in visited_map:
                visited_map.add(nxt)
                dx, dy = MOVE_OFFSET[DIR_MAP[d_str]]
                coords[nxt] = (coords[curr][0] + dx, coords[curr][1] + dy)
                queue.append(nxt)

    # 3. 找出所有死路 (Dead Ends) 並計算分數
    dead_ends = {}
    for node, edges in graph.items():
        if len(edges) == 1 and node != start_idx:
            # 計算曼哈頓距離 * 10
            dx = abs(coords[node][0] - coords[start_idx][0])
            dy = abs(coords[node][1] - coords[start_idx][1])
            dead_ends[node] = (dx + dy) * 10

    # 4. 使用 Dijkstra 演算法尋找最佳路徑 (考慮轉向成本)
    # 優先權佇列存放: (累積成本, 當前節點, 當前面向, 路徑歷史, 指令歷史)
    start_dir = DIR_MAP[start_facing]
    pq = [(0.0, start_idx, start_dir, [start_idx], "")]
    
    # 記錄到達 (節點, 面向) 的最低成本，避免無效繞圈
    min_cost_state = {}
    
    best_paths_to_targets = {}

    while pq:
        cost, curr_node, curr_dir, path_history, cmd_history = heapq.heappop(pq)

        state = (curr_node, curr_dir)
        if state in min_cost_state and min_cost_state[state] <= cost:
            continue
        min_cost_state[state] = cost

        # 如果抵達某個死路，記錄下來
        if curr_node in dead_ends:
            if curr_node not in best_paths_to_targets or cost < best_paths_to_targets[curr_node]['cost']:
                best_paths_to_targets[curr_node] = {
                    'cost': cost,
                    'path': path_history,
                    'commands': cmd_history
                }

        # 探索相鄰節點
        for next_d_str, next_node in graph[curr_node].items():
            next_abs_dir = DIR_MAP[next_d_str]
            
            # 計算轉向: 0(直走), 1(右轉), 2(迴轉), 3(左轉)
            turn_val = (next_abs_dir - curr_dir) % 4
            rel_cmd = REL_MAP[turn_val]
            
            # 計算這一步的成本 (轉向成本)
            step_cost = cost_weights[rel_cmd]
            new_cost = cost + step_cost
            
            heapq.heappush(pq, (new_cost, next_node, next_abs_dir, path_history + [next_node], cmd_history + rel_cmd))

    # 5. 計算 Efficiency 並排序排行榜
    results = []
    for target, score in dead_ends.items():
        if target in best_paths_to_targets:
            data = best_paths_to_targets[target]
            total_cost = data['cost']
            eff = score / total_cost if total_cost > 0 else 0
            
            results.append({
                'target': target,
                'score': score,
                'cost': total_cost,
                'efficiency': round(eff, 3),
                'commands': data['commands']
            })

    # 依照效率 (單位時間得分) 由高到低排序
    results.sort(key=lambda x: x['efficiency'], reverse=True)
    return results

# ==========================================
# 測試區塊
# ==========================================
if __name__ == "__main__":
    # 你可以隨時在這裡動態調整轉彎的代價！
    my_car_weights = {
        'f': 1.0,  # 直走最快，基準為 1
        'r': 1.8,  # 右轉稍慢
        'l': 1.8,  # 左轉稍慢
        'b': 3.5   # 迴轉非常花時間
    }
    
    # 假設從節點 28 出發，車頭一開始朝向北邊 ('N')
    START_NODE = 28
    START_FACING = 'N' 
    
    print(f"正在計算從節點 {START_NODE} 出發的最佳決策...\n")
    results = analyze_best_target('sample_maze.csv', START_NODE, START_FACING, my_car_weights)
    
    for rank, res in enumerate(results):
        print(f"第 {rank+1} 名: 前往死路節點 {res['target']}")
        print(f"  - 預計得分: {res['score']}")
        print(f"  - 時間成本: {res['cost']}")
        print(f"  - 獲利效率: {res['efficiency']} (分/單位時間)")
        print(f"  - 執行指令: {res['commands']}\n")
        
    print(f"系統建議: 直接將指令 '{results[0]['commands']}' 傳送給車子！")