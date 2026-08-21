# subfiles
from WifiComm import wifiClient

# libraries
import sys
import os
import time

CONNECTION_TIMEOUT = 15000 # ms

CLIENT_IP = "192.168.0.5"
CLIENT_PORT = 5000

wifiClient(CLIENT_IP, CLIENT_PORT)

if __name__ == "__main__":
    if (wifiClient.connect(timeout=CONNECTION_TIMEOUT, debug=True) != 0):
        print(f"Exiting program, failure to connect to client at {wifiClient.ip} {wifiClient.port}")
        exit(1)

    messages = [
        "A", 
        "Hello world!", 
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Suspendisse facilisis nulla id eros varius dignissim. Aenean non lectus sit amet sapien viverra dignissim. Donec vel nibh placerat, venenatis justo ut, pulvinar risus. Integer eu tortor varius, semper turpis eu, ornare erat. Praesent iaculis dolor a placerat posuere. Morbi tincidunt erat in dolor tempus, sed sodales justo mollis. Fusce mi neque, feugiat in imperdiet eu, sodales vitae sem. Duis posuere dolor sem, in varius purus semper eget. Quisque nec mi arcu. Morbi quis mi ac purus vestibulum auctor eu eget diam. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Fusce nec sagittis dolor, a eleifend quam."
    ]

    for i in range(3):
        print(f"===== Trial {i} =====")
        start = time.perf_counter() * 1000

        wifiClient.write(message=messages[i], debug=True)

        success = True

        while (reply := wifiClient.read()) is not None:
            current = time.perf_counter() * 1000
            duration = (current - start)
            if (duration.count() % 1000):
                print(f"Waiting for message, Time elapsed: {duration} ms...")
            if (duration > CONNECTION_TIMEOUT):
                success = False
                break

        if(success == False):
            print(f"Failed to receive message due to timeout (exceeded {CONNECTION_TIMEOUT} ms)")
            continue
        end = time.perf_counter() * 1000
        duration = (current - start)
        print(f"Successfully received message {reply}, total time elapsed: {duration} ms\n")
