# -*- coding: utf-8 -*-
import sys
import threading
import time

from hm10_esp32 import HM10ESP32Bridge

###
#Note that PORT can be different depending on your environment!!
###
PORT = 'COM5'
EXPECTED_NAME = 'HM10_G6'

def background_listener(bridge):
    while True:
        msg = bridge.listen()
        if msg:
            print(f"\r[CarCar]: {msg}")
            print("[You]: ", end="", flush=True)
        time.sleep(0.1)

def print_menu():
    print("\n" + "="*40)
    print("==== Car Control Command Menu ====")
    print("="*40)
    print("Mode Switching")
    print("  BT   : Switch to BlueTooth mode and stop")
    print("  AUTO : Switch to Auto mode")
    print("\nManual Control (Type 'BT' first)")
    print("  F    : Forward")
    print("  B    : Backward")
    print("  L    : Turn Left")
    print("  R    : Turn Right")
    print("  S    : Stop")
    print("\nSystem Commands")
    print("  help : Show this command menu again")
    print("  exit : Exit and close the chat")
    print("="*40 + "\n")
    
    
def main():
    bridge = HM10ESP32Bridge(port=PORT)

    # 1. Configuration Check
    current_name = bridge.get_hm10_name()
    if current_name != EXPECTED_NAME:
        print(f"Target mismatch. Current: {current_name}, Expected: {EXPECTED_NAME}")
        print(f"Updating target name to {EXPECTED_NAME}...")

        if bridge.set_hm10_name(EXPECTED_NAME):
            print("Name updated successfully. Resetting ESP32...")
            bridge.reset()
            # Re-init after reset
            bridge = HM10ESP32Bridge(port=PORT)
        else:
            print("Failed to set name. Exiting.")
            sys.exit(1)

    # 2. Connection Check
    status = bridge.get_status()
    if status != "CONNECTED":
        print(f"ESP32 is {status}. Please ensure HM-10 is advertising. Exiting.")
        sys.exit(0)

    print(f"Ready! Connected to {EXPECTED_NAME}")
    
 
    threading.Thread(target=background_listener, args=(bridge,), daemon=True).start()

    time.sleep(0.5) # 
    print_menu()

    try:
        while True:
            user_msg = input("[You]: ")
            
            if user_msg.lower() in ['exit', 'quit']: 
                break
            elif user_msg.lower() == 'help':
                print_menu()
                continue 
            
            
            if user_msg: 
                bridge.send(user_msg)
                
    except (KeyboardInterrupt, EOFError):
        pass

    print("\nChat closed.")

if __name__ == "__main__":
    main()