###remember to update weights 
import csv
import heapq

# Directions Mapping (Clockwise)
DIR_MAP = {'N': 0, 'E': 1, 'S': 2, 'W': 3}
REL_MAP = {0: 'F', 1: 'R', 2: 'B', 3: 'L'}
MOVE_OFFSET = {0: (0, 1), 1: (1, 0), 2: (0, -1), 3: (-1, 0)}

def get_absolute_direction(graph, coords, u, v):
    if u in graph:
        for d_str, nxt in graph[u].items():
            if nxt == v:
                return d_str
            
    if coords and u in coords and v in coords:
        dx = coords[v][0] - coords[u][0]
        dy = coords[v][1] - coords[u][1]
        if dx > 0: return 'E'
        if dx < 0: return 'W'
        if dy > 0: return 'N'
        if dy < 0: return 'S'

    print(f"[WARNING] Cannot determine direction from {u} to {v}. Defaulting to 'N'.")
    return 'N'

def build_map_data(csv_filepath, origin_idx):
    graph = {}
    with open(csv_filepath, 'r', encoding='utf-8') as file:
        reader = csv.DictReader(file)
        for row in reader:
            idx = int(row['index'])
            neighbors = {}
            for d in ['North', 'South', 'West', 'East']:
                val = row.get(d)
                if val and val.strip() and float(val) > 0:
                    neighbors[d[0]] = int(float(val))
            graph[idx] = neighbors

    coords = {origin_idx: (0, 0)}
    queue = [origin_idx]
    visited_coords = {origin_idx}
    
    while queue:
        curr = queue.pop(0)
        for d_str, nxt in graph[curr].items():
            if nxt not in visited_coords:
                visited_coords.add(nxt)
                dx, dy = MOVE_OFFSET[DIR_MAP[d_str]]
                coords[nxt] = (coords[curr][0] + dx, coords[curr][1] + dy)
                queue.append(nxt)

    dead_ends = {}
    for node, edges in graph.items():
        if len(edges) == 1 and node != origin_idx:
            dx = abs(coords[node][0] - coords[origin_idx][0])
            dy = abs(coords[node][1] - coords[origin_idx][1])
            dead_ends[node] = (dx + dy) * 10

    return graph, dead_ends, coords

def find_best_next_target(graph, coords, center_x, center_y, current_node, current_facing, unvisited_targets, weights):
    start_dir = DIR_MAP[current_facing]
    pq = [(0.0, current_node, start_dir, [current_node], "")]
    min_cost_state = {}
    best_results = {}

    while pq:
        cost, curr, curr_dir, path_hist, cmd_hist = heapq.heappop(pq)

        state = (curr, curr_dir)
        if state in min_cost_state and min_cost_state[state] <= cost:
            continue
        min_cost_state[state] = cost

        if curr in unvisited_targets:
            if curr not in best_results or cost < best_results[curr]['cost']:
                best_results[curr] = {
                    'cost': cost,
                    'commands': cmd_hist,
                    'path': path_hist
                }

        for next_d_str, nxt in graph[curr].items():
            next_abs_dir = DIR_MAP[next_d_str]
            turn_val = (next_abs_dir - curr_dir) % 4
            
            # --- 已經將 A 與 B 的結果對調 ---
            if turn_val == 2: 
                x, y = coords[curr]
                if curr_dir == 0:   # 原始面朝北，準備朝南
                    rel_cmd = 'B' if x > center_x else 'A'
                elif curr_dir == 1: # 原始面朝東，準備朝西
                    rel_cmd = 'B' if y < center_y else 'A'
                elif curr_dir == 2: # 原始面朝南，準備朝北
                    rel_cmd = 'B' if x < center_x else 'A'
                elif curr_dir == 3: # 原始面朝西，準備朝東
                    rel_cmd = 'B' if y > center_y else 'A'
            else:
                rel_cmd = REL_MAP[turn_val]
            
            new_cost = cost + weights.get(rel_cmd, 3.0) 
            heapq.heappush(pq, (new_cost, nxt, next_abs_dir, path_hist + [nxt], cmd_hist + rel_cmd))

    best_target = None
    best_efficiency = -1
    best_data = None

    for target, data in best_results.items():
        score = unvisited_targets[target]
        eff = score / data['cost'] if data['cost'] > 0 else 0
        
        if eff > best_efficiency:
            best_efficiency = eff
            best_target = target
            best_data = data

    return best_target, best_data, best_efficiency

def generate_continuous_mission(csv_filepath, start_idx, start_facing, weights=None):
    if weights is None:
        weights = {'F': 1.0, 'L': 1.5, 'R': 1.5, 'A': 3.0, 'B': 3.0}

    graph, unvisited_targets, coords = build_map_data(csv_filepath, start_idx)
    
    # 計算地圖的幾何中心點
    min_x = min(x for x, y in coords.values())
    max_x = max(x for x, y in coords.values())
    min_y = min(y for x, y in coords.values())
    max_y = max(y for x, y in coords.values())
    center_x = (min_x + max_x) / 2.0
    center_y = (min_y + max_y) / 2.0
    
    current_node = start_idx
    current_facing = start_facing
    
    total_score = 0
    total_cost = 0
    master_commands = ""

    print("="*60)
    print("INITIALIZING CONTINUOUS MISSION")
    print(f"Origin: {start_idx} | Facing: {start_facing}")
    print("="*60)

    step = 1
    while unvisited_targets:
        target, data, eff = find_best_next_target(graph, coords, center_x, center_y, current_node, current_facing, unvisited_targets, weights)
        
        if not target:
            print("No more reachable targets.")
            break
            
        path = data['path']
        cmds = data['commands']
        cost = data['cost']
        score = unvisited_targets[target]
        
        if len(path) > 1:
            current_facing = get_absolute_direction(graph, coords, path[-2], path[-1])
        
        if current_facing is None:
            current_facing = 'N' 
        
        total_score += score
        total_cost += cost
        master_commands += cmds
        
        print(f"Phase {step}: Harvest Node {target}")
        print(f"  Earned    : {score} Points")
        print(f"  Cost      : {cost} units")
        print(f"  Path      : {' -> '.join(map(str, path))}")
        print(f"  Commands  : {cmds}")
        print(f"  New State : At Node {target}, Facing {current_facing}")
        print("-" * 60)
        
        del unvisited_targets[target]
        current_node = target
        step += 1

    print("="*60)
    print("MISSION PLANNING COMPLETE")
    print(f"Total Expected Score : {total_score}")
    print(f"Total Time Cost      : {total_cost}")
    print(f"Master Command String: {master_commands}")
    print("="*60)
    
    return master_commands

# --- Main Execution ---
if __name__ == "__main__":
    FILE_NAME = "medium_maze.csv" 
    START_FACING = 'N'
    START_NODE = 1

    CAR_WEIGHTS = {'F': 1.0, 'L': 1.5, 'R': 1.5, 'A': 3.0, 'B': 3.0}

    try:
        final_command_sequence = generate_continuous_mission(FILE_NAME, START_NODE, START_FACING, CAR_WEIGHTS)
        
    except FileNotFoundError:
        print(f"[ERROR] {FILE_NAME} not found. Please check the file path.")
    except Exception as e:
        print(f"[ERROR] An unexpected error occurred: {e}")
# import csv
# import heapq

# # Directions Mapping (Clockwise)
# DIR_MAP = {'N': 0, 'E': 1, 'S': 2, 'W': 3}
# REL_MAP = {0: 'F', 1: 'R', 2: 'B', 3: 'L'}
# MOVE_OFFSET = {0: (0, 1), 1: (1, 0), 2: (0, -1), 3: (-1, 0)}

# def get_absolute_direction(graph, coords, u, v):
#     """
#     Helper to find the absolute direction from node u to node v.
#     Includes a geometric fallback to prevent KeyError: None if CSV is malformed.
#     """
#     # Strategy 1: Find direction using the adjacency graph
#     if u in graph:
#         for d_str, nxt in graph[u].items():
#             if nxt == v:
#                 return d_str
            
#     # Strategy 2: Fallback to physical (X, Y) coordinates if connection is abnormal
#     if coords and u in coords and v in coords:
#         dx = coords[v][0] - coords[u][0]
#         dy = coords[v][1] - coords[u][1]
#         if dx > 0: return 'E'
#         if dx < 0: return 'W'
#         if dy > 0: return 'N'
#         if dy < 0: return 'S'

#     # Strategy 3: Ultimate fail-safe to prevent system crash
#     print(f"[WARNING] Cannot determine direction from {u} to {v}. Defaulting to 'N'.")
#     return 'N'

# def build_map_data(csv_filepath, origin_idx):
#     """Parses the CSV, builds the graph, assigns coordinates, and calculates fixed scores."""
#     graph = {}
#     with open(csv_filepath, 'r', encoding='utf-8') as file:
#         reader = csv.DictReader(file)
#         for row in reader:
#             idx = int(row['index'])
#             neighbors = {}
#             for d in ['North', 'South', 'West', 'East']:
#                 val = row.get(d)
#                 if val and val.strip() and float(val) > 0:
#                     neighbors[d[0]] = int(float(val))
#             graph[idx] = neighbors

#     coords = {origin_idx: (0, 0)}
#     queue = [origin_idx]
#     visited_coords = {origin_idx}
    
#     while queue:
#         curr = queue.pop(0)
#         for d_str, nxt in graph[curr].items():
#             if nxt not in visited_coords:
#                 visited_coords.add(nxt)
#                 dx, dy = MOVE_OFFSET[DIR_MAP[d_str]]
#                 coords[nxt] = (coords[curr][0] + dx, coords[curr][1] + dy)
#                 queue.append(nxt)

#     # Scores are FIXED based on Manhattan distance to the ORIGINAL starting point
#     dead_ends = {}
#     for node, edges in graph.items():
#         if len(edges) == 1 and node != origin_idx:
#             dx = abs(coords[node][0] - coords[origin_idx][0])
#             dy = abs(coords[node][1] - coords[origin_idx][1])
#             dead_ends[node] = (dx + dy) * 10

#     # Returns coords as well for the fail-safe mechanism
#     return graph, dead_ends, coords

# def find_best_next_target(graph, current_node, current_facing, unvisited_targets, weights):
#     """Runs Dijkstra to find the most efficient NEXT target from the current state."""
#     start_dir = DIR_MAP[current_facing]
#     pq = [(0.0, current_node, start_dir, [current_node], "")]
#     min_cost_state = {}
#     best_results = {}

#     while pq:
#         cost, curr, curr_dir, path_hist, cmd_hist = heapq.heappop(pq)

#         state = (curr, curr_dir)
#         if state in min_cost_state and min_cost_state[state] <= cost:
#             continue
#         min_cost_state[state] = cost

#         if curr in unvisited_targets:
#             if curr not in best_results or cost < best_results[curr]['cost']:
#                 best_results[curr] = {
#                     'cost': cost,
#                     'commands': cmd_hist,
#                     'path': path_hist
#                 }

#         for next_d_str, nxt in graph[curr].items():
#             next_abs_dir = DIR_MAP[next_d_str]
#             turn_val = (next_abs_dir - curr_dir) % 4
#             rel_cmd = REL_MAP[turn_val]
            
#             new_cost = cost + weights[rel_cmd]
#             heapq.heappush(pq, (new_cost, nxt, next_abs_dir, path_hist + [nxt], cmd_hist + rel_cmd))

#     # Evaluate Efficiency: Fixed Target Score / Travel Cost from current position
#     best_target = None
#     best_efficiency = -1
#     best_data = None

#     for target, data in best_results.items():
#         score = unvisited_targets[target]
#         eff = score / data['cost'] if data['cost'] > 0 else 0
        
#         if eff > best_efficiency:
#             best_efficiency = eff
#             best_target = target
#             best_data = data

#     return best_target, best_data, best_efficiency

# def generate_continuous_mission(csv_filepath, start_idx, start_facing, weights=None):
#     """Generates a continuous sequence of commands to harvest points greedily."""
#     if weights is None:
#         weights = {'F': 1.0, 'L': 1.5, 'R': 1.5, 'B': 3.0}

#     # Fetch graph, targets, and coordinates
#     graph, unvisited_targets, coords = build_map_data(csv_filepath, start_idx)
    
#     current_node = start_idx
#     current_facing = start_facing
    
#     total_score = 0
#     total_cost = 0
#     master_commands = ""

#     print("="*60)
#     print("INITIALIZING CONTINUOUS MISSION")
#     print(f"Origin: {start_idx} | Facing: {start_facing}")
#     print("="*60)

#     step = 1
#     while unvisited_targets:
#         target, data, eff = find_best_next_target(graph, current_node, current_facing, unvisited_targets, weights)
        
#         if not target:
#             print("No more reachable targets.")
#             break
            
#         path = data['path']
#         cmds = data['commands']
#         cost = data['cost']
#         score = unvisited_targets[target]
        
#         # Determine the facing direction upon arriving at the target
#         if len(path) > 1:
#             current_facing = get_absolute_direction(graph, coords, path[-2], path[-1])
        
#         # Final safety lock
#         if current_facing is None:
#             current_facing = 'N' 
        
#         # Update metrics
#         total_score += score
#         total_cost += cost
#         master_commands += cmds
        
#         print(f"Phase {step}: Harvest Node {target}")
#         print(f"  Earned    : {score} Points")
#         print(f"  Cost      : {cost} units")
#         print(f"  Path      : {' -> '.join(map(str, path))}")
#         print(f"  Commands  : {cmds}")
#         print(f"  New State : At Node {target}, Facing {current_facing}")
#         print("-" * 60)
        
#         # Remove harvested target from the list
#         del unvisited_targets[target]
#         current_node = target
#         step += 1

#     print("="*60)
#     print("MISSION PLANNING COMPLETE")
#     print(f"Total Expected Score : {total_score}")
#     print(f"Total Time Cost      : {total_cost}")
#     print(f"Master Command String: {master_commands}")
#     print("="*60)
    
#     return master_commands

# # --- Main Execution ---
# if __name__ == "__main__":
#     # For Medium Maze
#     FILE_NAME = "medium_maze.csv" 
#     START_FACING = 'N'
#     START_NODE = 1

#     #For Big Maze
#     # FILE_NAME = "big_maze_114.csv"
#     # START_NODE = 25
#     # START_FACING = 'S'

#     # Customize Weights 
#     CAR_WEIGHTS = {'F': 1.0, 'L': 1.5, 'R': 1.5, 'B': 3.0}

#     try:
#         final_command_sequence = generate_continuous_mission(FILE_NAME, START_NODE, START_FACING, CAR_WEIGHTS)
        
#     except FileNotFoundError:
#         print(f"[ERROR] {FILE_NAME} not found. Please check the file path.")
#     except Exception as e:
#         print(f"[ERROR] An unexpected error occurred: {e}")