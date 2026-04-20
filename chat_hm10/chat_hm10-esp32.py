import sys
import time

from hm10_esp32 import HM10ESP32Bridge

# Note: PORT can be different depending on your environment
PORT = 'COM5'
EXPECTED_NAME = 'HM10_G6'

# Replace this with your actual BFS result
BFS_RESULT = "rbrbrbrb" 
WINDOW_SIZE = 3

def execute_auto_path(bridge, path_string):
    """
    Executes the path using a sliding window protocol.
    """
    commands = list(path_string.upper())
    total_steps = len(commands)
    sent_idx = 0
    ack_count = 0

    print("\n" + "="*40)
    print(f"Starting automatic path execution: {path_string.upper()}")
    print(f"Total steps: {total_steps}")
    print("="*40)

    # 1. Initial fill: Send the first 3 commands to the car
    for _ in range(WINDOW_SIZE):
        if sent_idx < total_steps:
            bridge.send(commands[sent_idx])
            print(f"[SEND] Initial command {sent_idx+1}/{total_steps}: {commands[sent_idx]}")
            sent_idx += 1
            time.sleep(0.1)

    # 2. Wait and refill loop
    while ack_count < total_steps:
        msg = bridge.listen()
        
        if msg:
            msg = msg.strip()
            
            # Check for the acknowledgment signal from ESP32
            if "STEP_DONE" in msg:
                ack_count += 1
                print(f"\n[ACK] Node reached. Completed step {ack_count}")
                
                # Send the next command if available
                if sent_idx < total_steps:
                    bridge.send(commands[sent_idx])
                    print(f"[SEND] Next command {sent_idx+1}/{total_steps}: {commands[sent_idx]}")
                    sent_idx += 1
            else:
                print(f"[STATUS] {msg}")
                
        time.sleep(0.01)

    print("\n" + "="*40)
    print("All commands executed. Destination reached.")
    print("="*40 + "\n")


def main():
    bridge = HM10ESP32Bridge(port=PORT)

    # Configuration Check
    current_name = bridge.get_hm10_name()
    if current_name != EXPECTED_NAME:
        print(f"Target mismatch. Current: {current_name}, Expected: {EXPECTED_NAME}")
        print(f"Updating target name to {EXPECTED_NAME}...")

        if bridge.set_hm10_name(EXPECTED_NAME):
            print("Name updated successfully. Resetting ESP32...")
            bridge.reset()
            bridge = HM10ESP32Bridge(port=PORT)
        else:
            print("Failed to set name. Exiting.")
            sys.exit(1)

    # Connection Check
    status = bridge.get_status()
    if status != "CONNECTED":
        print(f"ESP32 is {status}. Please ensure HM-10 is advertising. Exiting.")
        sys.exit(0)

    print(f"Ready! Connected to {EXPECTED_NAME}")
    time.sleep(1) 
    
    # Execute the calculated path
    try:
        execute_auto_path(bridge, BFS_RESULT)
    except KeyboardInterrupt:
        print("\nUser interrupted. Stopping the car.")
        bridge.send("S") 

if __name__ == "__main__":
    main()
# # -*- coding: utf-8 -*-
# import sys
# import threading
# import time

# from hm10_esp32 import HM10ESP32Bridge

# ###
# #Note that PORT can be different depending on your environment!!
# ###
# PORT = 'COM5'
# EXPECTED_NAME = 'HM10_G6'
# # 假設 result 是 BFS 算出來的結果 "ffrlb"
# commands = list(result.upper()) # 轉成大寫並拆成 ['F', 'F', 'R', 'L', 'B']
# sent_idx = 0     # 目前發送到哪一條
# ack_count = 0    # 車子已經回報完成幾條
# WINDOW_SIZE = 3  # 車子緩存大小

# def background_listener(bridge):
#     while True:
#         msg = bridge.listen()
#         if msg:
#             print(f"\r[CarCar]: {msg}")
#             print("[You]: ", end="", flush=True)
#         time.sleep(0.1)

# def print_menu():
#     print("\n" + "="*40)
#     print("==== Car Control Command Menu ====")
#     print("="*40)
#     print("Mode Switching")
#     print("  BT   : Switch to BlueTooth mode and stop")
#     print("  AUTO : Switch to Auto mode")
#     print("\nManual Control (Type 'BT' first)")
#     print("  F    : Forward")
#     print("  B    : Backward")
#     print("  L    : Turn Left")
#     print("  R    : Turn Right")
#     print("  S    : Stop")
#     print("\nSystem Commands")
#     print("  help : Show this command menu again")
#     print("  exit : Exit and close the chat")
#     print("="*40 + "\n")


# def main():
#     bridge = HM10ESP32Bridge(port=PORT)

#     # 1. Configuration Check
#     current_name = bridge.get_hm10_name()
#     if current_name != EXPECTED_NAME:
#         print(f"Target mismatch. Current: {current_name}, Expected: {EXPECTED_NAME}")
#         print(f"Updating target name to {EXPECTED_NAME}...")

#         if bridge.set_hm10_name(EXPECTED_NAME):
#             print("Name updated successfully. Resetting ESP32...")
#             bridge.reset()
#             # Re-init after reset
#             bridge = HM10ESP32Bridge(port=PORT)
#         else:
#             print("Failed to set name. Exiting.")
#             sys.exit(1)

#     # 2. Connection Check
#     status = bridge.get_status()
#     if status != "CONNECTED":
#         print(f"ESP32 is {status}. Please ensure HM-10 is advertising. Exiting.")
#         sys.exit(0)

#     print(f"Ready! Connected to {EXPECTED_NAME}")
    
 
#     threading.Thread(target=background_listener, args=(bridge,), daemon=True).start()

#     time.sleep(0.5) # 
#     print_menu()

#     try:
#         while True:
#             user_msg = input("[You]: ")
            
#             if user_msg.lower() in ['exit', 'quit']: 
#                 break
#             elif user_msg.lower() == 'help':
#                 print_menu()
#                 continue 
            
#             # send the messenge to CarCar
#             if user_msg: 
#                 bridge.send(user_msg) 
                
#     except (KeyboardInterrupt, EOFError):
#         pass

#     print("\nChat closed.")

# if __name__ == "__main__":
#     main()
