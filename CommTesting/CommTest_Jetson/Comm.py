# libraries
import socket

# objects
class client:
    def __init__(self, CLIENT_IP, CLIENT_PORT):
        self.ip = CLIENT_IP
        self.port = CLIENT_PORT

    def connect(self, debug = False):
        if debug == True: print(f"Connecting to client at {self.ip} {self.port}...")
        CLIENT_SOCKET = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        err = socket.connect_ex((self.ip, self.port))
        if (err != 0):
            if debug == True: print(f"Failed to connect to client at {self.ip} {self.port}")
            return -1
        if debug == True: print(f"Successfully connected to client at {self.ip} {self.port}")
        self.socket = CLIENT_SOCKET
        return 0

    def write(self, message, debug = False):
        message = f"{message}"
        try:
            self.socket.sendall(message.encode())
            if debug == True: print(f"Successfully sent message '{message.strip()}'")
        except socket.error as e:
            if debug == True: print(f"Failed to send message '{message.strip()}'")
            return -1
        return 0

    def read(self, debug = False):
        reply = self.socket.recv(1024).decode().strip()
        if reply:
            if debug == True: print(f"Successfully received message '{reply}'")
            return reply
        if debug == True: print(f"Failed to receive message")
        return None

    def disconnect(self, debug = False):
        self.socket.close()
        if debug == True: print(f"Disconnected from client at {self.ip} {self.port}")
