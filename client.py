import socket
from time import sleep


def send_message():
    # Create a TCP socket
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Server address and port
    port = 8000
    addr = "pico-server.local"
    # addr = "192.168.1.110"
    # addr = "arnorid.com"
    # port = 13000
    server_address = (addr, port)  # Default port 8080, change if needed

    try:
        # Connect to the server
        client_socket.connect(server_address)
        print(f"Connected to {server_address[0]}:{server_address[1]}")

        # Send message
        message = "Hello World"
        client_socket.sendall(message.encode("utf-8"))
        print(f"Sent: {message}")

        # Wait for the response
        response = client_socket.recv(1024)
        print(f"Received: {response.decode('utf-8')}")

    except ConnectionRefusedError:
        print("Connection failed. Make sure the server is running.")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        # Close the connection
        client_socket.close()
        print("Connection closed")


if __name__ == "__main__":
    send_message()
