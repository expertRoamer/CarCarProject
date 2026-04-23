import csv
from collections import deque

def find_shortest_path_directions(csv_filepath, start_idx, end_idx):
    """
    Reads a maze from a CSV file and returns a string of relative directions (f, b, l, r)
    representing the shortest path from start_idx to end_idx.
    """
    # 1. Read the graph from the CSV file
    graph = {}
    with open(csv_filepath, 'r', encoding='utf-8') as file:
        reader = csv.DictReader(file)
        for row in reader:
            idx = int(row['index'])
            neighbors = {}
            # Map absolute directions to connected nodes (ignoring distances)
            if row.get('North'): neighbors['N'] = int(row['North'])
            if row.get('South'): neighbors['S'] = int(row['South'])
            if row.get('West'):  neighbors['W'] = int(row['West'])
            if row.get('East'):  neighbors['E'] = int(row['East'])
            graph[idx] = neighbors

    # 2. Find the shortest path using Breadth-First Search (BFS)
    queue = deque([[start_idx]])
    visited = {start_idx}
    shortest_path = None

    while queue:
        path = queue.popleft()
        node = path[-1]

        # Check if we have reached the destination
        if node == end_idx:
            shortest_path = path
            break

        # Explore neighbors
        for direction, neighbor in graph[node].items():
            if neighbor not in visited:
                visited.add(neighbor)
                queue.append(path + [neighbor])

    if not shortest_path:
        return "No path found."

    if len(shortest_path) < 2:
        return "" # Already at destination

    # 3. Convert the absolute node path into relative directions
    # Assign integer values to absolute directions to easily calculate turns
    # N: 0, E: 1, S: 2, W: 3
    dir_map = {'N': 0, 'E': 1, 'S': 2, 'W': 3}

    # Differences modulo 4 correspond to relative turns:
    # 0: straight, 1: right, 2: back, 3: left
    rel_map = {0: 'f', 1: 'r', 2: 'b', 3: 'l'}

    def get_abs_dir(node_u, node_v):
        """Helper to find the absolute direction going from node u to node v"""
        for d, n in graph[node_u].items():
            if n == node_v:
                return dir_map[d]
        raise ValueError(f"No edge found from {node_u} to {node_v}")

    # The problem guarantees start is an endpoint, and initial orientation is
    # toward the adjacent cell. Therefore, the first movement is ALWAYS 'f'.
    current_abs_dir = get_abs_dir(shortest_path[0], shortest_path[1])
    directions_str = "f"

    # Iterate through the rest of the path to calculate relative turns
    for i in range(1, len(shortest_path) - 1):
        u = shortest_path[i]
        v = shortest_path[i + 1]

        next_abs_dir = get_abs_dir(u, v)

        # Calculate the turn needed from current orientation to next orientation
        turn = (next_abs_dir - current_abs_dir) % 4
        directions_str += rel_map[turn]

        # Update current orientation
        current_abs_dir = next_abs_dir

    return directions_str

# ==========================================
# Example usage and testing block
# ==========================================
if __name__ == "__main__":
    print("\nWelcome to the Pathfinding Algorithm Simulator!")
    start = int(input("Enter the starting node: "))
    end = int(input("Enter the ending node: "))
    result = find_shortest_path_directions('sample_maze.csv', start, end)
    print(f"Path directions from {start} to {end}: {result}")