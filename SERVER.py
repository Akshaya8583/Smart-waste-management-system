#for sending data
# import socket

# host = '192.168.135.56'  
# port = 80

# while True:
#     try:
#         with socket.create_connection((host, port)) as client:
#             message = input("Enter a message: ") + '\n'  
#             client.sendall(message.encode())
#     except Exception as e:
#         print(f"Error: {e}")



# for receiving data
import socket

host = '0.0.0.0' 
port = 8080

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
    server_socket.bind((host, port))
    server_socket.listen()

    print(f"Server listening on {host}:{port}")

    while True:
        conn, addr = server_socket.accept()
        with conn:
            print(f"Connected by {addr}")
            data = conn.recv(1024)
            if data:
                print(f"Received data: {data.decode()}")
